# Modelling the spreading of socio-geographic phenomen using techniques similar to the method of finite elements

---

## Installation

### Server and simulatiuon

**Makefile** is used for *building* the server and simulation. Therefore **Make** is required to be installed.
Another requirement is **C compiler** that supports **POSIX** standard (*Unix* has this by default | *Windows* need special compiler such as **Cygwin**).

Basic build can be done by using one of the following commands in the **Terminal** from the *ROOT* folder of the app:
```sh
make
```
```sh
make all
```

### Visualization

*Visualization* requires interpreter of **Python 3.6** or higher and *package manager* **Pip** (it comes with the interpreter).

Next all dependencies need to be installed by using the following command in the **Terminal** from the *ROOT* folder of the app:
```sh
pip install -r requirements.txt
```

---

## Before launching

Before launching the app, *parameters* of the simulation can be modified by editing the **parameters.cfg** in the *ROOT* folder of the app.
***Recommendation***: modify the values only! Don't *delete* or *add* any lines!

Initial state is defined in the **ROOT/DATA/initial.csv** file. Keep the format of the *csv* file, but feel free to add **well-formated** rows!

Requirement before the launch is existence of the following *files* / *folders*:
- **initial.csv** file in the *DATA* folder
- **DATA/sim_frames** folder
- **DATA/vis_frames** folder

---

## Launch and usage

Always launch the **server** first, then launch the **visualization**.

Server can be launched from the **Terminal** from the *ROOT* folder of the app.
(Assuming the app was built by using *make* and **server** or **server.exe** exists in the *ROOT* folder)
Two arguments can be used while launching the *server*: **-ip4** and **-port**. As the names suggest, **-ip4** sets the IPv4 of the server and **-port** sets the port on which the server is listening.
Default value of **IPv4** is *INADDR_ANY* (0.0.0.0), default value of **port** is *4242*.
Server can be launched using one of the following commands in the **Terminal** from the *ROOT* folder of the app:
- **Unix**:
    ```sh
    root $ ./server
    ```
    ```sh
    root $ ./server -ip4 127.0.0.1 -port 4242
    ```
- **Windows**:
    ```sh
    C:/root> server.exe
    ```
    ```sh
    C:/root> server.exe -ip4 127.0.0.1 -port 4242
    ```
    (root is the *ROOT* folder of the app)
    ***Recommendation***: use the default values.

Visualization can be launched from the **Terminal** from the *ROOT/PY* folder of the app.
Two arguments can be used while launching the *visualization*, **they are possitional unlike the server ones!** First argument is the *IPv4* adress of the server, second argument is the *port* that the server is listening on.
Visualization can be launched using one of the following commands in the **Terminal** from the *ROOT* folder of the app:
- **Unix**:
    ```sh
    root/PY $ python visuals.py
    ```
    ```sh
    root/PY $ python visuals.py 127.0.0.1 4242
    ```
- **Windows**:
    ```sh
    C:/root/PY> python visuals.py
    ```
    ```sh
    C:/root/PY> python visuals.py 127.0.0.1 4242
    ```
    (root/PY is the *ROOT/PY* folder of the app)
    ***Recommendation***: use the default values.

If the visualization successfully connects to the server it sends a *command* to **start** the simulation. There should be an **adress** in the console on which the visualization can be found.

The map is fully interactive and there is plenty of buttons and sliders:
- Animation can be controlled by the *slider* and two *buttons* located under the map.
- Contrast of colors (yellow means more infected) can be modified with the '*Z-slider*'.
- Radius of the circles can be modified with the '*Radius-slider*'.
- *Default view* button restarts the visualization to the default state (map position and zoom, animation frame is set to 0, sliders are set to the value of 5).
- *Kill* buttons are used to terminate both *visualization* and the *simulation*.
    ***Recommendation***: terminate simulation first, if you want to terminate **both**!
