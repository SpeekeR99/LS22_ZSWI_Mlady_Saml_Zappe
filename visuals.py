import plotly.express as px
import pandas as pd
import dash
from dash import dcc, html, Input, Output, State
import dash_bootstrap_components as dbc

app = dash.Dash(external_stylesheets=[dbc.themes.BOOTSTRAP])  # Bootstrap connection

app.layout = html.Div(  # Main div
    [
        html.Div(  # Div with figure (map)
            children=[
                dcc.Graph(
                    id="map",
                    style={
                        "width": "130vh",
                        "height": "90vh"
                    }
                )
            ],
            style={
                'width': '70%',
                'display': 'inline-block'
            }
        ),
        html.Div(  # Div with sliders and buttons
            children=[
                html.Div(  # Just a text above a slider
                    "'Z' scale coefficient",
                    style={
                        "text-align": "center",
                        "margin-bottom": "10px"
                    }
                ),
                dcc.Slider(  # Slider for z coefficient
                    id='z-slider',
                    min=1, max=20, step=.1, value=8,
                    marks={i: f'{i}' for i in range(1, 21)},
                    tooltip={
                        "placement": "bottom",
                        "always_visible": True
                    }
                ),
                html.Div(  # Just a text above a slider
                    "Radius scale coefficient",
                    style={
                        "text-align": "center",
                        "margin-bottom": "10px",
                        "margin-top": "50px"
                    }
                ),
                dcc.Slider(  # Slider for radius coefficient
                    id='radius-slider',
                    min=1, max=20, step=.1, value=2.5,
                    marks={i: f'{i}' for i in range(1, 21)},
                    tooltip={
                        "placement": "bottom",
                        "always_visible": True
                    }
                ),
                html.Div(  # Div with buttons
                    children=[
                        dbc.Button(  # Button for triggering callback
                            "Update data",
                            id="update-button",
                            n_clicks=0,
                            outline=True,
                            color="dark",
                            size="lg",
                            style={"margin-right": "100px"}
                        ),
                        dbc.Button(  # Button that resets the view to default
                            "Reset",
                            id="reset-button",
                            n_clicks=0,
                            outline=True,
                            color="danger",
                            size="lg"
                        )
                    ],
                    style={
                        "margin-top": "550px",
                        "justify-content": "center",
                        "align-items": "center",
                        "display": "flex"}),
            ],
            style={
                'width': '29%',
                'display': 'inline-block',
                "margin-top": "70px",
                'vertical-align': 'top'
            }
        )
    ]
)


@app.callback(
    [Output('map', 'figure'),
     Output("z-slider", "value"),
     Output("radius-slider", "value")],
    Input('update-button', 'n_clicks'),
    Input("reset-button", "n_clicks"),
    Input("z-slider", "value"),
    Input("radius-slider", "value"),
    State("map", "figure"))
def update_figure(update_input, reset_input, z_coef, radius_coef, curr_fig):
    """
    Callback function, callback can be called by pressing any button or by fiddling with the sliders.
    If user used sliders to trigger the callback, z and radius coefficient won't be taken from the previous state
    of the figure (curr_fig).
    If user used update button to trigger the callback, everything will stay the same, but data will be updated.
    If user used reset button to trigger the callback, everything will reset to default state (default state is
    the state the app is started at).
    :param update_input: number of times the update button was clicked, it is not important
    :param reset_input: number of times the reset button was clicked, it is not important
    :param z_coef: the value on the z-slider
    :param radius_coef: the value on the radius-slider
    :param curr_fig: state of the figure right before updating it, used to store important user data such as zoom
    :return: new state of the figure and for reset purposes z-slider value and radius-slider value too
    """
    ctx = dash.callback_context  # To distinguish which input was used

    trigger = None
    if ctx.triggered:
        trigger = ctx.triggered[0]["prop_id"]  # String of the used input

    if trigger is not None and "reset" in trigger:  # If resetting, coefficients are set to default values
        z_coef = 8
        radius_coef = 2.5

    df = pd.read_csv("data/merged.csv")  # Reading the main data here

    fig = px.density_mapbox(  # Creating new figure
        df,
        lat="latitude",
        lon="longitude",
        z=df["pocet_obyvatel"] ** (1.0 / z_coef),  # Roots seem to work better than logarithms
        radius=df["pocet_obyvatel"] ** (1.0 / radius_coef),  # Roots seem to work better than logarithms
        hover_name="nazev_obce",
        hover_data=["nazev_obce", "kod_obce", "pocet_obyvatel"],
        animation_frame="datum",
        mapbox_style="open-street-map",
        center=dict(lat=49.88537, lon=15.3684),
        zoom=6.6,
        color_continuous_scale="plasma"
    )
    fig.update_layout(hoverlabel=dict(bgcolor="white", font_size=16, font_family="Inter"))

    # Big block of code to restore user important data, such as where the user is located now and how much has he zoomed
    # Because with every figure update, every information is lost and figure would be reset to default
    if curr_fig is not None and trigger is not None and not ("reset" in trigger):
        fig["layout"]["sliders"][0]["active"] = curr_fig["layout"]["sliders"][0]["active"]
        fig["data"][0].coloraxis = curr_fig["data"][0]["coloraxis"]
        fig["data"][0].customdata = curr_fig["data"][0]["customdata"]
        fig["data"][0].hovertemplate = curr_fig["data"][0]["hovertemplate"]
        fig["data"][0].hovertext = curr_fig["data"][0]["hovertext"]
        fig["data"][0].lat = curr_fig["data"][0]["lat"]
        fig["data"][0].lon = curr_fig["data"][0]["lon"]
        fig["data"][0].name = curr_fig["data"][0]["name"]
        if "update" in trigger:  # Important to not use if the trigger was from sliders or reset button!
            fig["data"][0].radius = curr_fig["data"][0]["radius"]
            fig["data"][0].z = curr_fig["data"][0]["z"]
        fig["data"][0].subplot = curr_fig["data"][0]["subplot"]
        fig["layout"]["mapbox"]["center"] = curr_fig["layout"]["mapbox"]["center"]
        fig["layout"]["mapbox"]["zoom"] = curr_fig["layout"]["mapbox"]["zoom"]

    return [fig, z_coef, radius_coef]


if __name__ == '__main__':
    app.run_server(debug=True)
