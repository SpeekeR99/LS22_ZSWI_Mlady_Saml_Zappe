import plotly.express as px
import pandas as pd
import dash
from dash import dcc, html, Input, Output, State
import dash_bootstrap_components as dbc

app = dash.Dash(external_stylesheets=[dbc.themes.BOOTSTRAP])

app.layout = html.Div([
    html.Div(
        children=[dcc.Graph(id="map", style={"width": "130vh", "height": "90vh"})],
        style={'width': '70%', 'display': 'inline-block'}),
    html.Div(
        children=[
            html.Div("'Z' scale coefficient", style={"text-align": "center", "margin-bottom": "10px"}),
            dcc.Slider(1, 20, .1, value=8, id='z-slider', marks={i: f'{i}' for i in range(1, 21)},
                       tooltip={"placement": "bottom", "always_visible": True}),
            html.Div("Radius scale coefficient", style={"text-align": "center", "margin-bottom": "10px",
                                                        "margin-top": "50px"}),
            dcc.Slider(1, 20, .1, value=2.5, id='radius-slider', marks={i: f'{i}' for i in range(1, 21)},
                       tooltip={"placement": "bottom", "always_visible": True}),
            html.Div(children=[dbc.Button("Update data", id="update-button", n_clicks=0, outline=True, color="dark",
                                          size="lg", style={"margin-right": "100px"}),
                               dbc.Button("Reset", id="reset-button", n_clicks=0, outline=True, color="danger",
                                          size="lg")],
                     style={"margin-top": "550px", "justify-content": "center", "align-items": "center",
                            "display": "flex"})
        ],
        style={'width': '29%', 'display': 'inline-block', "margin-top": "70px", 'vertical-align': 'top'})
])


@app.callback(
    [Output('map', 'figure'),
     Output("z-slider", "value"),
     Output("radius-slider", "value")],
    Input('update-button', 'n_clicks'),
    Input("reset-button", "n_clicks"),
    Input("z-slider", "value"),
    Input("radius-slider", "value"),
    State("map", "figure"))
def update_figure(unused_input, reset_input, z_coef, radius_coef, curr_fig):
    ctx = dash.callback_context

    trigger = None
    if ctx.triggered:
        trigger = ctx.triggered[0]["prop_id"]

    if trigger is not None and "reset" in trigger:
        z_coef = 8
        radius_coef = 2.5

    df = pd.read_csv("data/merged.csv")

    fig = px.density_mapbox(df, lat="latitude", lon="longitude", z=df["pocet_obyvatel"] ** (1.0 / z_coef),
                            radius=df["pocet_obyvatel"] ** (1.0 / radius_coef),
                            hover_name="nazev_obce", hover_data=["nazev_obce", "kod_obce", "pocet_obyvatel"],
                            animation_frame="datum", mapbox_style="open-street-map",
                            center=dict(lat=49.88537, lon=15.3684), zoom=6.6, color_continuous_scale="plasma")
    fig.update_layout(hoverlabel=dict(bgcolor="white", font_size=16, font_family="Inter"))

    if curr_fig is not None and trigger is not None and not ("reset" in trigger):
        fig["layout"]["sliders"][0]["active"] = curr_fig["layout"]["sliders"][0]["active"]
        fig["data"][0].coloraxis = curr_fig["data"][0]["coloraxis"]
        fig["data"][0].customdata = curr_fig["data"][0]["customdata"]
        fig["data"][0].hovertemplate = curr_fig["data"][0]["hovertemplate"]
        fig["data"][0].hovertext = curr_fig["data"][0]["hovertext"]
        fig["data"][0].lat = curr_fig["data"][0]["lat"]
        fig["data"][0].lon = curr_fig["data"][0]["lon"]
        fig["data"][0].name = curr_fig["data"][0]["name"]
        if "update" in trigger:
            fig["data"][0].radius = curr_fig["data"][0]["radius"]
            fig["data"][0].z = curr_fig["data"][0]["z"]
        fig["data"][0].subplot = curr_fig["data"][0]["subplot"]
        fig["layout"]["mapbox"]["center"] = curr_fig["layout"]["mapbox"]["center"]
        fig["layout"]["mapbox"]["zoom"] = curr_fig["layout"]["mapbox"]["zoom"]

    return [fig, z_coef, radius_coef]


if __name__ == '__main__':
    app.run_server(debug=True)
