import platform
import ctypes
import plotly.express as px
import pandas as pd
from dash import dcc, html
from dash_extensions.enrich import Input, Output, State, DashProxy, MultiplexerTransform
import dash_bootstrap_components as dbc

import socket
import sys

# ------------- CLIENT PART --------------------

port = 4242 if len(sys.argv) != 3 else int(sys.argv[2])
host_ip = "127.0.0.1" if len(sys.argv) != 3 else sys.argv[1]
SOCKET_BUFFER_SIZE = 4194304


def create_and_connect_socket():
    try:
        sockfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print("Socket successfully created")
    except socket.error as err:
        print("socket creation failed with error %s" % err)
        return None

    try:
        sockfd.connect((host_ip, port))
        print("Client part connected")
    except socket.gaierror as e:
        print("Address-related error connecting to server: %s" % e)
        sockfd.close()
        return None
    except socket.error as e:
        print("Connection error: %s" % e)
        sockfd.close()
        return None
    return sockfd


# client ready to send commands to the server
# usage: a callback in the visualisation app (through a button perhaps)
# the callback calls:
# try:
#    sockfd.send(command_in_string.encode())
# except socket.timeout as err:
#        print("Error: Server timed out.")
#        sockfd.close()
# except socket.error:
#    print("Error: could not write to server.")
#    sockfd.close()


# ------------- DATA PART -----------------------


def create_data_hash_table(filepath="../DATA/initial.csv"):
    """
    Creates hash table where key is city id
    Under city id 3 more keys can be found - nazev_obce, latitude and longitude
    These contain the static values of cities that won't ever change
    :param filepath: Filepath to initial.csv
    :return: Hash table of static values of cities
    """
    hash_table = {}
    with open(filepath, "r", encoding="utf8") as fp:
        keys = fp.readline().split(",")
        line = fp.readline().split(",")
        while line and len(line) > 3:
            hash_table[line[1]] = {keys[0]: line[0], keys[2]: line[2], keys[3]: line[3]}
            line = fp.readline().split(",")
    return hash_table


def initialize_merged_csv(source="../DATA/initial.csv", filepath="../DATA/merged.csv"):
    """
    Creates and initializes merged.csv with frame value 0
    Uses initial.csv to initialize the default first state of the simulation
    :param source: Filepath to initial.csv
    :param filepath: Filepath to merged.csv
    :return: no return value
    """
    with open(filepath, "w", encoding="utf8") as fp:
        with open(source, "r", encoding="utf8") as ini:
            line = ini.readline().split(",")
            while line and len(line) > 3:
                fp.write(line[0] + "," + line[1] + "," + line[2] + "," + line[3] + "," + line[5] + "," + line[6] + "," +
                         line[7])
                line = ini.readline().split(",")


def update_data_csv(csv_data):
    """
    Appends new CSV files (frames) to the merged.csv main data file
    Expected format is the same, as the output format from csvManager.c
    So all that is needed from server is to copy the created framexxxx.csv file
    and send all the lines as Strings to client, it will handle it well
    :param csv_data: New csv data frame, expected format is the same as the output format from csvManager.c
    :return: no return value
    """
    with open(FILEPATH, "a", encoding="utf8") as fp:
        lines = csv_data.split("\n")
        for line in lines:
            if line == "\x04":
                break
            line = line.split(",")
            kod_obce = line[0]
            pocet_obyvatel = line[1]
            pocet_nakazenych = line[2]
            datum = line[3]
            info = CITY_ID_HASH_TABLE[kod_obce]
            nazev_obce = info["nazev_obce"]
            latitude = info["latitude"]
            longitude = info["longitude"]
            fp.write("\n" + nazev_obce + "," + kod_obce + "," + latitude + "," + longitude + "," + pocet_obyvatel + ","
                     + pocet_nakazenych + "," + datum)


# ------------- VISUALS PART --------------------

SCALE_FACTOR = 1.25
if platform.uname()[0] == "Windows":
    SCALE_FACTOR = ctypes.windll.shcore.GetScaleFactorForDevice(0) / 100
DEFAULT_Z_COEF = 5  # 8
DEFAULT_RADIUS_COEF = 19.7 - 1.2 * SCALE_FACTOR  # 18.5 (100%) 18.2 (125%)
FILEPATH = "../DATA/merged.csv"
CITY_ID_HASH_TABLE = create_data_hash_table()


def create_default_figure(filepath=FILEPATH, z_coef=DEFAULT_Z_COEF, radius_coef=DEFAULT_RADIUS_COEF):
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
        zoom=9 - 2.4 * SCALE_FACTOR,  # 6.6 (100%) 6 (125%)
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
                        "margin-top": str(1350 - 800 * SCALE_FACTOR) + "px",
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


def remain_figure_state(new_fig, old_fig, z_slider_trigger=False, radius_slider_trigger=False):
    """
    Restores user important data, such as where the user is located now and how much has he zoomed
    Because with every figure update, every information is lost and figure would be reset to default
    :param new_fig: New future figure
    :param old_fig: Current state of the figure before updating
    :param z_slider_trigger: Z magnitude slider, if user used this, don't update from the old state
    :param radius_slider_trigger: Radius slider, if user used this, don't update from the old state
    :return: Figure that has new dataframe, but old characteristics (such as zoom and animation frame...)
    """
    if old_fig is not None:
        if "sliders" in old_fig["layout"].keys():
            new_fig["layout"]["sliders"][0]["active"] = old_fig["layout"]["sliders"][0]["active"]
        new_fig["data"][0].coloraxis = old_fig["data"][0]["coloraxis"]
        new_fig["data"][0].customdata = old_fig["data"][0]["customdata"]
        new_fig["data"][0].hovertemplate = old_fig["data"][0]["hovertemplate"]
        new_fig["data"][0].hovertext = old_fig["data"][0]["hovertext"]
        new_fig["data"][0].lat = old_fig["data"][0]["lat"]
        new_fig["data"][0].lon = old_fig["data"][0]["lon"]
        new_fig["data"][0].name = old_fig["data"][0]["name"]
        if not radius_slider_trigger:  # If the user used the radius slider, don't use the old value
            new_fig["data"][0].radius = old_fig["data"][0]["radius"]
        if not z_slider_trigger:  # If the user used the z slider, don't use the old value
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
    sockfd = create_and_connect_socket()

    try:
        sockfd.send("send_data".encode())
    except socket.timeout:
        print("Error: Server timed out.")
        sockfd.close()
    except socket.error:
        print("Error: could not write to server.")
        sockfd.close()
    csv_data = sockfd.recv(SOCKET_BUFFER_SIZE).decode('ascii').rstrip('\x00\x0a\x0D')
    sockfd.close()

    update_data_csv(csv_data)
    # update_data_csv("568449,100,100,1\n554782,500,2,2")

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
    fig = remain_figure_state(fig, curr_fig, z_slider_trigger=True)
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
    fig = remain_figure_state(fig, curr_fig, radius_slider_trigger=True)
    return fig


if __name__ == '__main__':
    initialize_merged_csv()
    app.run_server(debug=True)
