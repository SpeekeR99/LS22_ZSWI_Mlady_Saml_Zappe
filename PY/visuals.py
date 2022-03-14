import plotly.express as px
import pandas as pd
from dash import dcc, html
from dash_extensions.enrich import Input, Output, State, DashProxy, MultiplexerTransform
import dash_bootstrap_components as dbc

import socket
import sys

## ------------- CLIENT PART --------------------

port = 4242 if len(sys.argv) != 3 else int(sys.argv[2])
host_ip = "127.0.0.1" if len(sys.argv) != 3 else sys.argv[1]

try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print ("Socket successfully created")
except socket.error as err:
        print ("socket creation failed with error %s" %(err))
        exit(0)

try: 
        s.connect((host_ip, port)) 
except socket.gaierror as e: 
        print ("Address-related error connecting to server: %s" % e) 
        s.close()
except socket.error as e: 
        print ("Connection error: %s" % e) 
        s.close()
print("Client part connected")
# client ready to send commands to the server
# usage: a callback in the visualisation app (through a button perhaps)
# the callback calls:
# try:
#    s.send(command_in_string.encode())
# except socket.timeout as err:
#        print("Error: Server timed out.")
#        s.close()
# except socket.error:
#    print("Error: could not write to server.")
#    s.close()



## ------------- VISUALS PART --------------------

DEFAULT_Z_COEF = 5  # 8
DEFAULT_RADIUS_COEF = 18.2  # 18.5


def create_default_figure(filepath="data/merged.csv", z_coef=DEFAULT_Z_COEF, radius_coef=DEFAULT_RADIUS_COEF):
    """
    Creates default figure containing the map from csv data file
    :param filepath: Path to data csv file
    :param z_coef: Z magnitude coefficient, basically how contrast the colors are
    :param radius_coef: Radius coefficient, how big the dots on the map are
    :return: Default figure with density mapbox
    """
    df = pd.read_csv(filepath)  # Reading the main data here
    fig = px.density_mapbox(  # Creating new figure
        df,
        lat="latitude",
        lon="longitude",
        z=df["pocet_obyvatel"] ** (1.0 / z_coef),  # Roots seem to work better than logarithms
        radius=df["pocet_obyvatel"] ** (1.0 / (21 - radius_coef)),  # Roots seem to work better than logarithms
        hover_name="nazev_obce",
        hover_data=["nazev_obce", "kod_obce", "pocet_obyvatel"],
        animation_frame="datum",
        mapbox_style="open-street-map",
        center=dict(lat=49.88537, lon=15.3684),
        zoom=6,  # 6.6
        color_continuous_scale="plasma"
    )
    fig.update_layout(hoverlabel=dict(bgcolor="white", font_size=16, font_family="Inter"))
    return fig


app = DashProxy(
    prevent_initial_callbacks=True,
    transforms=[MultiplexerTransform()],  # More callbacks for one output
    external_stylesheets=[dbc.themes.BOOTSTRAP]  # Bootstrap
)

app.layout = html.Div(  # Main div
    [
        html.Div(  # Div with figure (map)
            children=[
                dcc.Graph(
                    id="map",
                    figure=create_default_figure(),
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
                    min=1, max=20, step=.1, value=DEFAULT_Z_COEF,
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
                    min=1, max=20, step=.1, value=DEFAULT_RADIUS_COEF,
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
                        "margin-top": "350px",
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


def remain_figure_state(new_fig, old_fig, z_slider=False, radius_slider=False):
    """
    Restores user important data, such as where the user is located now and how much has he zoomed
    Because with every figure update, every information is lost and figure would be reset to default
    :param new_fig: New future figure
    :param old_fig: Current state of the figure before updating
    :param z_slider: Z magnitude slider, if user used this, don't update from the old state
    :param radius_slider: Radius slider, if user used this, don't update from the old state
    :return: Figure that has new dataframe, but old characteristics (such as zoom and animation frame...)
    """
    if old_fig is not None:
        new_fig["layout"]["sliders"][0]["active"] = old_fig["layout"]["sliders"][0]["active"]
        new_fig["data"][0].coloraxis = old_fig["data"][0]["coloraxis"]
        new_fig["data"][0].customdata = old_fig["data"][0]["customdata"]
        new_fig["data"][0].hovertemplate = old_fig["data"][0]["hovertemplate"]
        new_fig["data"][0].hovertext = old_fig["data"][0]["hovertext"]
        new_fig["data"][0].lat = old_fig["data"][0]["lat"]
        new_fig["data"][0].lon = old_fig["data"][0]["lon"]
        new_fig["data"][0].name = old_fig["data"][0]["name"]
        if not radius_slider:  # If the user used the radius slider, don't use the old value
            new_fig["data"][0].radius = old_fig["data"][0]["radius"]
        if not z_slider:  # If the user used the z slider, don't use the old value
            new_fig["data"][0].z = old_fig["data"][0]["z"]
        new_fig["data"][0].subplot = old_fig["data"][0]["subplot"]
        new_fig["layout"]["mapbox"]["center"] = old_fig["layout"]["mapbox"]["center"]
        new_fig["layout"]["mapbox"]["zoom"] = old_fig["layout"]["mapbox"]["zoom"]
    return new_fig


@app.callback(
    Output("map", "figure"),
    Input("update-button", "n_clicks"),
    State("map", "figure"))
def update_button(update_input, curr_fig):
    """
    Update button callback, basically ONLY updates the dataframe, retains everything else as it was
    :param update_input: unused input, how many times was the update-button pressed
    :param curr_fig: Current state of figure, right before updating
    :return: Figure with updated DataFrame
    """
    fig = create_default_figure()
    fig = remain_figure_state(fig, curr_fig)
    return fig


@app.callback(
    Output("map", "figure"),
    Output("z-slider", "value"),
    Output("radius-slider", "value"),
    Input("reset-button", "n_clicks"))
def reset_button(reset_input):
    """
    Reset button callback, resets the figure to default state, with default zoom, default location
    default slider values...
    :param reset_input: unused input, how many times was the reset-button pressed
    :return: Figure in default state
    """
    global DEFAULT_Z_COEF
    global DEFAULT_RADIUS_COEF
    return create_default_figure(), DEFAULT_Z_COEF, DEFAULT_RADIUS_COEF


@app.callback(
    Output("map", "figure"),
    Input("z-slider", "value"),
    State("map", "figure"))
def z_slider(z_coef, curr_fig):
    """
    Z-slider callback, updates the z magnitude coefficient based on the user updated slider
    :param z_coef: Coefficient of how contrast the colours are
    :param curr_fig: Current state of figure, right before updating
    :return: Figure with updated z magnitude value
    """
    fig = create_default_figure(z_coef=z_coef)
    fig = remain_figure_state(fig, curr_fig, z_slider=True)
    return fig


@app.callback(
    Output("map", "figure"),
    Input("radius-slider", "value"),
    State("map", "figure"))
def radius_slider(radius_coef, curr_fig):
    """
    Radius-slider callback, updates the radius coefficient based on the user updated slider
    :param radius_coef: Coefficient of how big the dots on the map are
    :param curr_fig: Current state of figure, right before updating
    :return: Figure with updated radius value
    """
    fig = create_default_figure(radius_coef=radius_coef)
    fig = remain_figure_state(fig, curr_fig, radius_slider=True)
    return fig


if __name__ == '__main__':
    app.run_server(debug=True)
