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
        print("Socket creation failed with error %s" % err)
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
    :param bmsg: bytes message to send (raw data, with ending \\x04)
    :return: True if message was sent, False otherwise (error)
    """
    success = False
    try:
        if sockfd.send(bmsg) == len(bmsg):
            print(f"\tMessage '{bmsg.decode()}' sent")
            success = True
        else:
            print(f"\tMessage '{bmsg.decode()}' not sent")
    except socket.timeout:
        print("Error: server timed out.")
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
    socket_send(SOCKET_EXIT_CODE, sockfd)
    # sockfd.shutdown(socket.SHUT_WR)
    sockfd.close()
    print("Client disconnected\n")


def socket_send(bytes_message, sock=None):
    """
    Sends message to server
    :param bytes_message: bytes message to send, without ending \\x04 (will be added)
    :param sock: socket of client, if None, creates new socket and discards afterwards
    :return: True if message was sent, False otherwise (error)
    """
    bytes_message += b'\x04'
    single_send = False
    if sock is None:
        single_send = True
        sock = create_and_connect_socket()
        if sock is None:
            return False

    if __sock_send(sock, bytes_message, single_send):
        return True
    return False


def socket_read(sock):
    """
    Reads message from server
    :param sock: socket of client
    :return: Bytes message from server
    """
    msg = b""
    try:
        while True:
            msg += sock.recv(SOCKET_BUFFER_SIZE)
            if msg.endswith(b'\x04'):
                break
    except socket.timeout:
        print("Error: server timed out.")
    except socket.error:
        print("Error: could not read from server.")
    return msg


def socket_send_and_read(bytes_message):
    """
    Sends message to server and reads response
    :param bytes_message: Bytes message to send
    :return: Response from server (bytes)
    """
    sock = create_and_connect_socket()
    if sock is None:
        return None
    if socket_send(bytes_message, sock):
        msg = socket_read(sock)
        close_socket(sock)
        return msg
    return None


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


# VISUALIZATION


MERGEPATH = "../DATA/merged.csv"
INIPATH = "../DATA/initial.csv"
FRAMESPATH = "../DATA/vis_frames/"
SCALE_FACTOR = 1.25
if platform.uname()[0] == "Windows":
    SCALE_FACTOR = ctypes.windll.shcore.GetScaleFactorForDevice(0) / 100
DEFAULT_Z_COEF = 5  # 8
DEFAULT_RADIUS_COEF = 5.5 - 0.5 * SCALE_FACTOR  # 18.5 (100%) 18.2 (125%)
old_frame = 0
frame = 0
old_total_infected = 0
total_infected = 0
new_infected = 0


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


def create_default_figure(filepath=FRAMESPATH + "frame0000.csv", z_coef=DEFAULT_Z_COEF,
                          radius_coef=DEFAULT_RADIUS_COEF):
    """
    Creates default figure containing the map from csv data file
    :param filepath: Path to data csv file
    :param z_coef: Z magnitude coefficient, basically how contrast the colors are
    :param radius_coef: Radius coefficient, how big the dots on the map are
    :return: Default figure with density mapbox
    """
    df = pd.read_csv(filepath)  # Reading the main data here
    curr_frame = 1000 * int(filepath[-8]) + 100 * int(filepath[-7]) + 10 * int(filepath[-6]) + int(filepath[-5])

    global old_total_infected
    global total_infected
    global new_infected
    old_total_infected = total_infected
    total_infected = df["pocet_nakazenych"].sum()
    if curr_frame != old_frame:
        new_infected = total_infected - old_total_infected

    fig = px.density_mapbox(  # Creating new figure
        df,
        lat="latitude",
        lon="longitude",
        z=df["pocet_nakazenych"] ** (1.0 / z_coef),  # Roots seem to work better than logarithms
        radius=(df["pocet_nakazenych"] + 1) ** (2.5 / (11 - radius_coef)),
        hover_name="nazev_obce",
        hover_data=["nazev_obce", "kod_obce", "pocet_obyvatel", "pocet_nakazenych"],
        mapbox_style="open-street-map",
        center=dict(lat=49.88537, lon=15.3684),
        zoom=9 - 2.4 * SCALE_FACTOR,  # 6.6 (100%) 6 (125%)
        color_continuous_scale="plasma"
    )
    fig.update_layout(hoverlabel=dict(bgcolor="white", font_size=16, font_family="Inter"))
    return fig


def create_first_frame():
    """
    Creates first frame of the simulation for visualization
    """
    with open(INIPATH, "r", encoding="utf8") as fp_ini:
        with open(FRAMESPATH + "frame0000.csv", "w", encoding="utf8") as fp_frame:
            header = fp_ini.readline().replace(",vymera", "")
            fp_frame.write(header)
            line = fp_ini.readline().split(",")
            while line and len(line) > 1:
                line.pop(4)
                fp_frame.write(
                    line[0] + "," + line[1] + "," + line[2] + "," + line[3] + "," + line[4] + "," + line[5] + "," +
                    line[6])
                line = fp_ini.readline().split(",")


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
    print("Data received\n")
    global frame
    frame += 1
    filepath = FRAMESPATH + "frame" + (str(frame).rjust(4, '0')) + ".csv"
    with open(MERGEPATH, "a", encoding="utf8") as fp:
        lines = csv_data.split("\n")
        lines.pop(0)
        __write_received2csv(fp, lines)
    with open(filepath, 'w', encoding="utf8") as fp:
        lines = csv_data.split("\n")
        lines.pop(0)
        with open(INIPATH, "r", encoding="utf8") as fp_ini:
            header = fp_ini.readline().replace(",vymera", "").replace("\n", "")
            fp.write(header)
        __write_received2csv(fp, lines)


def __write_received2csv(fp, lines):
    """
    Writes received lines to the csv file
    :param fp: File pointer to the csv file
    :param lines: Received lines from server
    """
    for line in lines:
        if "\x04" in line:
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


def update_img(chosen_frame, curr_fig, z_coef=DEFAULT_Z_COEF, radius_coef=DEFAULT_RADIUS_COEF):
    """
    Updates the map image with the new data
    :param chosen_frame: Chosen frame of the animation
    :param curr_fig: Current figure state
    :param z_coef: Z maagnitude coefficient
    :param radius_coef: Radius coefficient
    :return: Figure with the new data and old zoom and translation
    """
    filepath = FRAMESPATH + "frame" + str(chosen_frame).rjust(4, '0') + ".csv"
    fig = create_default_figure(filepath=filepath, z_coef=z_coef, radius_coef=radius_coef)
    fig = remain_figure_state(fig, curr_fig)
    global old_frame
    old_frame = chosen_frame
    return fig


def remain_figure_state(new_fig, old_fig):
    """
    Restores translation and zoom of the old figure to the new one
    :param new_fig: New future figure
    :param old_fig: Current state of the figure before updating
    :return: Figure that has new dataframe, but old translation and zoom
    """
    if old_fig is not None:
        new_fig["layout"]["mapbox"]["center"] = old_fig["layout"]["mapbox"]["center"]
        new_fig["layout"]["mapbox"]["zoom"] = old_fig["layout"]["mapbox"]["zoom"]
    return new_fig
