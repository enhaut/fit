#!/usr/bin/python3.10
# coding=utf-8
import pandas as pd
import geopandas
import matplotlib.pyplot as plt
import contextily as ctx
import sklearn.cluster
import numpy as np
# muzete pridat vlastni knihovny


def make_geo(df: pd.DataFrame) -> geopandas.GeoDataFrame:
    """ Konvertovani dataframe do geopandas.GeoDataFrame se spravnym kodovani"""
    df = df[df['d'].notna() & df['e'].notna() & df["p2a"].notna()]  # drop rows with invalid lat/lng/dates

    return geopandas.GeoDataFrame(
        df,
        geometry=geopandas.points_from_xy(df.d, df.e),
        crs="EPSG:5514"
    )


def export(fig, location, show):
    if location:
        fig.savefig(location, bbox_inches="tight")

    if show:
        plt.show()

    plt.close(fig)


def plot_geo(gdf: geopandas.GeoDataFrame, fig_location: str = None,
             show_figure: bool = False):
    """ Vykresleni grafu s nehodami s alkoholem pro roky 2018-2021 pre Jihomoravsky kraj """

    fig, (y1, y2) = plt.subplots(2, 2, figsize=(15, 12))
    gdf = gdf[(gdf["region"] == "JHM") & (gdf["p11"] >= 3)].to_crs("EPSG:3857")  # vodorovne crs

    border_vals_adj = 1000  # asd
    xlim = gdf.total_bounds[0] + border_vals_adj, gdf.total_bounds[2] + border_vals_adj
    ylim = gdf.total_bounds[1] + border_vals_adj, gdf.total_bounds[3] + border_vals_adj

    for year, ax in zip(("2018", "2019", "2020", "2021"), (*y1, *y2)):
        filtered = gdf[(gdf['p2a'] > f"{year}-01-01") & (gdf['p2a'] < f"{year}-12-31")]
        filtered.plot(
            ax=ax,
            markersize=2,
        )

        ctx.add_basemap(
            ax,
            source=ctx.providers.Stamen.TonerLite,
            crs=gdf.crs.to_string()
        )
        ax.set_xlim(*xlim)
        ax.set_ylim(*ylim)
        ax.set_title(year)

        ax.axis("off")

    plt.legend()
    plt.tight_layout()

    fig.suptitle("Nehody v kraji JHM", fontsize=20)
    fig.legend(("Nehoda", ), loc="lower center")

    export(fig, fig_location, show_figure)


def plot_cluster(gdf: geopandas.GeoDataFrame, fig_location: str = None,
                 show_figure: bool = False):
    """ Vykresleni grafu s lokalitou vsech nehod v kraji shlukovanych do clusteru """
    pass

if __name__ == "__main__":
    # zde muzete delat libovolne modifikace
    gdf = make_geo(pd.read_pickle("accidents.pkl.gz"))
    plot_geo(gdf, "geo1.png", True)
    plot_cluster(gdf, "geo2.png", True)
