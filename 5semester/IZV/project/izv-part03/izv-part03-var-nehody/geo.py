#!/usr/bin/python3.10
# coding=utf-8
import pandas as pd
import geopandas
import matplotlib.pyplot as plt
import contextily as ctx
from sklearn.cluster import KMeans
from matplotlib.cm import ScalarMappable
from matplotlib.colors import Normalize


def make_geo(df: pd.DataFrame) -> geopandas.GeoDataFrame:
    """
        Konvertovani dataframe do geopandas.GeoDataFrame se spravnym kodovani
    :param df: raw frame
    :return: geopandas frame in EPSG3857
    """

    df = df[df['d'].notna() & df['e'].notna() & df["p2a"].notna()]  # drop rows with invalid lat/lng/dates

    return geopandas.GeoDataFrame(
        df,
        geometry=geopandas.points_from_xy(df.d, df.e),
        crs="EPSG:5514"
    ).to_crs("EPSG:3857")


def export(fig, location, show):
    """
        Function exports plot

    :param fig: figure to export
    :param location: export location in case of saving to the disk
    :param show: True/False
    """
    if location:
        fig.savefig(location, bbox_inches="tight")

    if show:
        plt.show()

    plt.tight_layout()
    plt.close(fig)


def plot_geo(gdf: geopandas.GeoDataFrame, fig_location: str = None,
             show_figure: bool = False):
    """
        Vykresleni grafu s nehodami s alkoholem pro roky 2018-2021 pre Jihomoravsky kraj

    :param gdf: processed geopandas frame
    :param fig_location: path for figure file
    :param show_figure: True/False
    """

    fig, (y1, y2) = plt.subplots(2, 2, figsize=(15, 12))
    gdf = gdf[(gdf["region"] == "JHM") & (gdf["p11"] >= 3)]  # vodorovne crs

    border_vals_adj = 1000  # unzoom it little bit
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

    plt.tight_layout()

    fig.suptitle("Nehody v kraji JHM", fontsize=20)
    fig.legend(("Nehoda", ), loc="lower center")

    export(fig, fig_location, show_figure)


def plot_cluster(gdf: geopandas.GeoDataFrame, fig_location: str = None,
                 show_figure: bool = False):
    """
        Vykresleni grafu s lokalitou vsech nehod v kraji shlukovanych do clusteru

    :param gdf: processed geopandas frame
    :param fig_location: path for figure file
    :param show_figure: True/False
    """
    gdf = gdf[((gdf["region"] == "JHM") & (gdf["p36"] > 0) & (gdf["p36"] < 4))].copy()
    # ^^^ copy() to not change original data

    kmeans = KMeans(n_clusters=20, n_init=10)  # i've chosen KMeans alg, the one I understand the most
    kmeans.fit(gdf.loc[:, ["d", "e"]])

    gdf["cluster"] = kmeans.predict(gdf.loc[:, ["d", "e"]])
    gdf4 = gdf.dissolve(by="cluster", aggfunc={"p1": "count"})

    fig, ax = plt.subplots(1, 1, figsize=(10, 11))
    gdf4.plot(ax=ax, column="p1", cmap="viridis", markersize=4)
    cmappable = ScalarMappable(norm=Normalize(gdf4["p1"].min(), gdf4["p1"].max()), cmap="viridis")

    ctx.add_basemap(ax, source=ctx.providers.Stamen.TonerLite, crs=gdf.crs.to_string())
    ax.axis("off")
    ax.set_title("Nehody v jednotlivych oblastiach")

    bar = fig.colorbar(cmappable, location="bottom", shrink=1, pad=0.02, ax=ax)
    bar.set_label("Pocet nehod")

    export(fig, fig_location, show_figure)


if __name__ == "__main__":
    # zde muzete delat libovolne modifikace
    gdf = make_geo(pd.read_pickle("/Users/sdobron/skola/izv/project/izv-part03/izv-part03-var-nehody/accidents.pkl.gz"))
    plot_geo(gdf, "geo1.png", True)
    plot_cluster(gdf, "geo2.png", True)
