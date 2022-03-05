import plotly.express as px
import pandas as pd
from dash import Dash, dcc, html, Input, Output

app = Dash(__name__)

app.layout = html.Div([
    html.Div(
        children=[dcc.Graph(id="map", style={"width": "130vh", "height": "90vh"})],
        style={'width': '70%', 'display': 'inline-block'}),
    html.Div(
        children=[
            html.Div("'Z' scale coefficient", style={"text-align": "center", "margin-bottom": "10px"}),
            dcc.Slider(1, 20, .1, value=8, id='z-slider', marks=None,
                       tooltip={"placement": "bottom", "always_visible": True}),
            html.Div("Radius scale coefficient", style={"text-align": "center", "margin-bottom": "10px",
                                                        "margin-top": "50px"}),
            dcc.Slider(1, 20, .1, value=2.5, id='radius-slider', marks=None,
                       tooltip={"placement": "bottom", "always_visible": True}),
            html.Div(html.Button("Update data", id="update-button", n_clicks=0,
                                 style={'font-size': '12px', 'width': '140px', 'display': 'inline-block',
                                        'height': '40px'}),
                     style={"margin-top": "550px", "justify-content": "center", "align-items": "center",
                            "display": "flex"})
        ],
        style={'width': '29%', 'display': 'inline-block', "margin-top": "70px", 'vertical-align': 'top'})
])


@app.callback(
    Output('map', 'figure'),
    Input('update-button', 'n_clicks'),
    Input("z-slider", "value"),
    Input("radius-slider", "value"),)
def update_figure(unused_input, z_coef, radius_coef):
    df = pd.read_csv("data/merged.csv")

    fig = px.density_mapbox(df, lat="latitude", lon="longitude", z=df["pocet_obyvatel"] ** (1.0 / z_coef),
                            radius=df["pocet_obyvatel"] ** (1.0 / radius_coef),
                            hover_name="nazev_obce", hover_data=["nazev_obce", "kod_obce", "pocet_obyvatel"],
                            animation_frame="datum", mapbox_style="open-street-map",
                            center=dict(lat=49.88537, lon=15.3684), zoom=6.6, color_continuous_scale="plasma")
    fig.update_layout(hoverlabel=dict(bgcolor="white", font_size=16, font_family="Inter"))

    return fig


if __name__ == '__main__':
    app.run_server(debug=True)
