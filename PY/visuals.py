from utils import *
from dash import dcc, html
from dash_extensions.enrich import Input, Output, State, DashProxy, MultiplexerTransform
import dash_bootstrap_components as dbc

import sys

if len(sys.argv) == 3:
    set_ip(sys.argv[1])
    set_port(int(sys.argv[2]))

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
        ),
        dcc.Interval(  # Timer component for updating data
            id='update-timer',
            interval=1 * 10000,  # in milliseconds
            n_intervals=0
        )
    ]
)


@app.callback(
    Output("map", "figure"),
    Input("update-timer", "n_intervals"),
    State("map", "figure"))
def update_button(update_input, curr_fig):
    """
    Update button callback, basically ONLY updates the dataframe, retains everything else as it was
    :param update_input: unused input, how many times was the update-timer incremented
    :param curr_fig: Current state of figure, right before updating
    :return: Figure with updated DataFrame
    """
    return update_img(curr_fig)


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
    socket_send(b"start")
    app.run_server(debug=True)
