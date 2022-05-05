import sys
import os
import signal
from dash import dcc, html
from dash_extensions.enrich import Input, Output, State, DashProxy, MultiplexerTransform
import dash_bootstrap_components as dbc

from utils import *
import utils

if len(sys.argv) == 3:
    set_ip(sys.argv[1])
    set_port(int(sys.argv[2]))

app = DashProxy(
    name="visuals",
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
                ),
                html.Div(
                    children=[
                        dbc.Button(
                            "Play",
                            id="play-button",
                            n_clicks=0,
                            outline=True,
                            color="success",
                            size="sm",
                            style={
                                "display": "flex",
                                "padding-right": "18px",
                            }
                        ),
                        dbc.Button(
                            "Pause",
                            id="pause-button",
                            n_clicks=0,
                            outline=True,
                            color="danger",
                            size="sm",
                            style={
                                "display": "flex",
                            }
                        )
                    ],
                    style={
                        "display": "inline-block",
                        "width": "10%",
                        "margin-left": "8vh"
                    }
                ),
                html.Div(
                    children=[
                        dcc.Slider(
                            id='animation-slider',
                            min=0,
                            max=0,
                            step=1,
                            value=0,
                            marks={i: f'{i}' for i in range(0, 1)},
                            tooltip={
                                "placement": "bottom",
                                "always_visible": True
                            }
                        )
                    ],
                    style={
                        "display": "inline-block",
                        "width": "72%",
                        "margin-right": "12vh"
                    }
                ),
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
                        dbc.Button(  # Button that resets the view to default
                            "Default view",
                            id="reset-button",
                            n_clicks=0,
                            outline=True,
                            color="success",
                            size="lg",
                            style={
                                "margin-right": "50px"
                            }
                        ),
                        dbc.Button(
                            "Kill Visualization",
                            id="kill-vis-button",
                            n_clicks=0,
                            outline=True,
                            color="danger",
                            size="lg",
                            style={
                                "margin-right": "10px"
                            }
                        ),
                        dbc.Button(
                            "Kill Simulation",
                            id="kill-sim-button",
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
                        "display": "flex"
                    }
                ),
            ],
            style={
                'width': '29%',
                'display': 'inline-block',
                "margin-top": "70px",
                'vertical-align': 'top'
            }
        ),
        html.Div(
            id="timers",
            children=[
                dcc.Interval(  # Timer component for updating data
                    id='update-timer',
                    interval=1 * 10000,  # in milliseconds
                    n_intervals=0
                ),
                dcc.Interval(
                    id="animation-timer",
                    interval=500,
                    n_intervals=0,
                    disabled=True
                )
            ]
        )
    ]
)


@app.callback(
    Output("animation-slider", "max"),
    Output("animation-slider", "marks"),
    Input("update-timer", "n_intervals"))
def download_data(update_input):
    """
    Update button callback, basically ONLY updates the dataframe, retains everything else as it was
    :param update_input: unused input, how many times was the update-timer incremented
    :return: Figure with updated DataFrame
    """
    csv_str = socket_send_and_read((f"send_data {utils.frame}").encode()).decode()
    update_data_csv(csv_str)

    marks = {i: f'{i}' for i in range(0, utils.frame + 1)}
    return utils.frame, marks


@app.callback(
    Output("map", "figure"),
    Input("animation-slider", "value"),
    State("map", "figure"))
def update_figure(chosen_frame, curr_fig):
    return update_img(curr_fig, chosen_frame)


@app.callback(
    Output("map", "figure"),
    Output("z-slider", "value"),
    Output("radius-slider", "value"),
    Output("animation-slider", "value"),
    Input("reset-button", "n_clicks"))
def reset_button(reset_input):
    """
    Reset button callback, resets the figure to default state, with default zoom, default location
    default slider values...
    :param reset_input: unused input, how many times was the reset-button pressed
    :return: Figure in default state
    """
    return create_default_figure(), DEFAULT_Z_COEF, DEFAULT_RADIUS_COEF, 0


@app.callback(
    Input("play-button", "n_clicks"),
    State("timers", "children"),
    Output("timers", "children"))
def start_animation(play_input, timers):
    """
    Start animation
    :param play_input: Unused input, how many times was the button pressed
    :param timers: list of timers
    :return: new timer component
    """
    timers[1]["props"]["disabled"] = False
    return timers


@app.callback(
    Input("pause-button", "n_clicks"),
    State("timers", "children"),
    Output("timers", "children"))
def pause_animation(pause_input, timers):
    """
    Pause animation
    :param pause_input: Unused input, how many times was the button pressed
    :param timers: list of timers
    """
    timers[1]["props"]["disabled"] = True
    return timers


@app.callback(
    Input("animation-timer", "n_intervals"),
    State("animation-timer", "disabled"),
    State("animation-slider", "value"),
    State("animation-slider", "max"),
    Output("animation-slider", "value"))
def animate(play_input, disabled, curr_frame, max_frame):
    """
    Animation of the map
    :param play_input: Unused input, seconds passed
    :param disabled: is the animation disabled?
    :param curr_frame: current frame
    :param max_frame: max frame
    :return: next frame (or current frame if curr frame == max frame)
    """
    if disabled:
        return curr_frame
    elif curr_frame < max_frame:
        return curr_frame + 1
    else:
        return curr_frame


@app.callback(
    Input("kill-vis-button", "n_clicks"),
    Output("kill-vis-button", "n_clicks"))
def kill_visuals(kill_input):
    """
    Kills the visualization
    """
    os.kill(os.getpid(), signal.SIGTERM)


@app.callback(
    Input("kill-sim-button", "n_clicks"),
    Output("kill-sim-button", "n_clicks"))
def kill_simulation(kill_input):
    """
    Kills the simulation
    """
    socket_send(b"start")


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
    if not socket_send(b"start"):
        print("Could not send 'start' to server")
    create_first_frame()
    app.run_server(debug=True, use_reloader=False)
