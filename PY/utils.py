import socket
import plotly.express as px
import pandas as pd
import platform
import ctypes

# SOCKET STUFF

SOCKET_EXIT_CODE = b"I'LL BE BACK"
__port = 4242
__host_ip = "127.0.0.1"
SOCKET_BUFFER_SIZE = 4194304


def create_and_connect_socket():
    """
    Creates and connects socket to server
    :return: Socket to server or None if error
    """
    sockfd = object()
    try:
        sockfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print("Socket successfully created")
    except socket.error as err:
        print("socket creation failed with error %s" % err)
        return None

    try:
        sockfd.connect((__host_ip, __port))
        print("Client part connected")
    except socket.gaierror as e:
        print("Address-related error connecting to server: %s" % e)
        close_socket(sockfd)
        return None
    except socket.error as e:
        print("Connection error: %s" % e)
        close_socket(sockfd)
        return None
    return sockfd


def __sock_send(sockfd, bmsg, single_send=False):
    """
    Sends message to server
    :param sockfd: client socket
    :param bmsg: bytes message to send
    :return: True if message was sent, False otherwise (error)
    """
    success = False
    try:
        sockfd.send(bmsg)
        success = True
    except socket.timeout:
        print("Error: Server timed out.")
    except socket.error:
        print("Error: could not write to server.")
    finally:
        if single_send:
            close_socket(sockfd)

    return success


def close_socket(sockfd):
    """
    Closes socket
    :param sockfd: Socket to close
    :return: no return value
    """
    sockfd.send(SOCKET_EXIT_CODE)
    # sockfd.shutdown(socket.SHUT_WR)
    sockfd.close()
    print("Client disconnected\n")


def socket_send(bytes_message, sock=None):
    """
    Sends message to server
    :param bytes_message: bytes message to send
    :param sock: socket of client, if None, creates new socket and discards afterwards
    :return: True if message was sent, False otherwise (error)
    """
    single_send = False
    if sock is None:
        single_send = True
        sock = create_and_connect_socket()
        if sock is None:
            close_socket(sock)
            return False

    if __sock_send(sock, bytes_message, single_send):
        return True
    return False


# SETTERS

def set_ip(ip):
    """
    Sets the ip of the server
    :param ip: IP to set
    :return: no return value
    """
    global __host_ip
    __host_ip = ip


def set_port(port: int):
    """
    Sets the port of the server
    :param port: Port to set
    :return: no return value
    """
    global __port
    __port = port


# VISULAISATION

MERGEPATH = "../DATA/merged.csv"
INIPATH = "../DATA/initial.csv"
SCALE_FACTOR = 1.25
if platform.uname()[0] == "Windows":
    SCALE_FACTOR = ctypes.windll.shcore.GetScaleFactorForDevice(0) / 100
DEFAULT_Z_COEF = 5  # 8
DEFAULT_RADIUS_COEF = 19.7 - 1.2 * SCALE_FACTOR  # 18.5 (100%) 18.2 (125%)
FRAME = 0


def __create_data_hash_table(filepath=INIPATH):
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


def __initialize_merged_csv(source=INIPATH, filepath=MERGEPATH):
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


CITY_ID_HASH_TABLE = __create_data_hash_table()
__initialize_merged_csv()


def create_default_figure(filepath=MERGEPATH, z_coef=DEFAULT_Z_COEF, radius_coef=DEFAULT_RADIUS_COEF):
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
        z=df["pocet_nakazenych"] ** (1.0 / z_coef),  # Roots seem to work better than logarithms
        #  TODO - the +1 MAY cause some problems when there are no cases in the city
        radius=(df["pocet_nakazenych"] + 1) ** (1.0 / (21 - radius_coef)),  # Roots seem to work better than logarithms
        hover_name="nazev_obce",
        hover_data=["nazev_obce", "kod_obce", "pocet_obyvatel", "pocet_nakazenych"],
        animation_frame="datum",
        mapbox_style="open-street-map",
        center=dict(lat=49.88537, lon=15.3684),
        zoom=9 - 2.4 * SCALE_FACTOR,  # 6.6 (100%) 6 (125%)
        color_continuous_scale="plasma"
    )
    fig.update_layout(hoverlabel=dict(bgcolor="white", font_size=16, font_family="Inter"))
    return fig


def update_data_csv(csv_data):
    """
    Appends new CSV files (frames) to the merged.csv main data file
    Expected format is the same, as the output format from csvManager.c
    So all that is needed from server is to copy the created framexxxx.csv file
    and send all the lines as Strings to client, it will handle it well
    :param csv_data: New csv data frame, expected format is the same as the output format from csvManager.c
    :return: no return value
    """

    if csv_data.startswith("no data"):
        return
    print("data recieved\n")
    global FRAME
    FRAME = FRAME + 1
    with open(MERGEPATH, "a", encoding="utf8") as fp:
        lines = csv_data.split("\n")
        lines.pop(0)
        for line in lines:
            # with open("dbg.log","a") as dbg:
            #    dbg.write(line)
            #    dbg.write("\n")
            if line == "\x04":
                break
            line = line.split(",")
            kod_obce = line[0]
            pocet_obyvatel = line[1]
            pocet_nakazenych = line[2]
            datum = str(int(line[3]) + 1)
            info = CITY_ID_HASH_TABLE[kod_obce]
            nazev_obce = info["nazev_obce"]
            latitude = info["latitude"]
            longitude = info["longitude"]
            fp.write("\n" + nazev_obce + "," + kod_obce + "," + latitude + "," + longitude + "," + pocet_obyvatel + ","
                     + pocet_nakazenych + "," + datum)


def update_img(cur_fig):
    """
    Reads data from server, updates the data
    and loads the data into the image
    :param cur_fig: Figure to be updated
    :return: figure with new data, same state as cur_fig
    """
    sockfd = create_and_connect_socket()

    socket_send(("send_data " + str(FRAME)).encode(), sockfd)

    csv_data = bytes()

    while not csv_data.endswith(b'\x04'):
        csv_data = csv_data + sockfd.recv(SOCKET_BUFFER_SIZE)

    csv_str = csv_data.decode("ascii")
    # with open("dbg.log","w") as dbg:
    #    dbg.write(csv_str)
    #    dbg.write("recv end\n")

    close_socket(sockfd)
    update_data_csv(csv_str)

    fig = create_default_figure()
    fig = remain_figure_state(fig, cur_fig)
    return fig


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
