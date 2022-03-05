import numpy as np
import plotly.express as px
import pandas as pd

df = pd.read_csv("data/merged.csv")

fig = px.scatter_mapbox(df, lat="latitude", lon="longitude", hover_name="nazev_obce",
                        hover_data=["nazev_obce", "kod_obce", "pocet_obyvatel"], animation_frame="datum",
                        mapbox_style="open-street-map", center=dict(lat=49.88537, lon=15.3684), zoom=6.8,
                        color=np.log10(df["pocet_obyvatel"]) / np.log10(1000000), color_continuous_scale="plasma",
                        size=np.log10(df["pocet_obyvatel"]), title="FEMlike huehue")
fig.update_layout(hoverlabel=dict(bgcolor="white", font_size=16, font_family="Inter"))
fig.show()
