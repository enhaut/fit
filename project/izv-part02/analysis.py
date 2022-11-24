#!/usr/bin/env python3.9
# coding=utf-8

from matplotlib import pyplot as plt
from matplotlib.ticker import LinearLocator
import pandas as pd
import seaborn as sns
import numpy as np
import zipfile
import io

# muzete pridat libovolnou zakladni knihovnu ci knihovnu predstavenou na prednaskach
# dalsi knihovny pak na dotaz


# Ukol 1: nacteni dat ze ZIP souboru
def load_data(filename: str) -> pd.DataFrame:
    # tyto konstanty nemente, pomuzou vam pri nacitani
    headers = ["p1", "p36", "p37", "p2a", "weekday(p2a)", "p2b", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13a",
                "p13b", "p13c", "p14", "p15", "p16", "p17", "p18", "p19", "p20", "p21", "p22", "p23", "p24", "p27", "p28",
                "p34", "p35", "p39", "p44", "p45a", "p47", "p48a", "p49", "p50a", "p50b", "p51", "p52", "p53", "p55a",
                "p57", "p58", "a", "b", "d", "e", "f", "g", "h", "i", "j", "k", "l", "n", "o", "p", "q", "r", "s", "t", "p5a",
               "region"]

    regions = {
        "00": "PHA",
        "01": "STC",
        "02": "PLK",
        "03": "JHC",
        "04": "ULK",
        "05": "HKK",
        "06": "JHM",
        "07": "MSK",
        "14": "OLK",
        "15": "ZLK",
        "16": "VYS",
        "17": "PAK",
        "18": "LBK",
        "19": "KVK"
    }
    # ^^ ids are used as a key, much faster than having region ID as a key

    raw_data = io.BytesIO()
    nl_encoded = "\r\n".encode("cp1250")

    with zipfile.ZipFile(filename) as zips:  # unzip year zip
        for zip_name in zips.namelist():
            zip_file = zipfile.ZipFile(
                zips.open(
                    zip_name
                )
            )
            for csv_file in zip_file.namelist():
                csv = zip_file.open(csv_file)  # unzip region csv
                raw_csv = csv.read()
                if not len(raw_csv):
                    continue  # skip empty region csvs

                region = ""
                if csv_file.endswith(".csv"):
                    region = csv_file[:-4]

                raw_data.write(
                    raw_csv.replace(  # append `;{REGION_ID}` before the line end
                        nl_encoded,
                        f";{regions.get(region, '')}\r\n".encode("cp1250")
                    )
                )

    raw_data.seek(0)  # move cursor in front of first line

    df = pd.read_csv(
        raw_data,
        low_memory=False,
        on_bad_lines=None,
        names=headers,
        header=None,
        encoding="cp1250",
        encoding_errors="ignore",
        sep=";",
        index_col=0,
        decimal=","
    )
    return df


def _get_size(df: pd.DataFrame):
    size = df.memory_usage(deep=True).sum()
    return round(size / (10**6), 1)


# Ukol 2: zpracovani dat
def parse_data(df: pd.DataFrame, verbose: bool = False) -> pd.DataFrame:
    if verbose:
        print(f"orig_size: {_get_size(df)} MB")
    df = df[~df.index.duplicated(keep="first")].copy()

    df = df.rename(columns={"p2a": "date"})
    df["date"] = pd.to_datetime(df["date"], errors="coerce")

    to_int_cols = [
        "l", "n", "weekday(p2a)", "r", "s", "p19", "p5a", "p6", "p8", "p9", "p10", "p11", "p12", "p13c",
        "p15", "p16", "p17", "p18", "p20", "p21", "p22", "p23", "p24", "p27", "p28", "p7", "p13a", "p13b",
        "p34", "p35", "p36", "p37", "p39", "p44", "p45a", "p47", "p48a", "p49", "p50a", "p51",
        "p52", "p55a", "p57", "p58"
    ]
    for col in to_int_cols:
        df[col] = pd.to_numeric(df[col], errors="coerce", downcast="integer")

    categorized_cols = ["q", "i", "h", "k", "p", "t"]
    for col in categorized_cols:
        df[col] = df[col].astype("category")

    to_float_cols = ["s", "r", "g", "f", "e", "d", "b", "a", "p53", "p14", "p9", "p37"]
    for col in to_float_cols:
        df[col] = pd.to_numeric(df[col], errors="coerce", downcast="float")

    if verbose:
        print(f"new_size: {_get_size(df)} MB")

    return df


def p19_enum_to_str(enum: int):
    """
        Function converts enum to weather condition type.

    :param enum: ID of weather condition type
    :return: string or None if ID is not defined
    """
    match enum:
        case 1:
            return "Deň - nezhoršená"
        case 2 | 3:
            return "Deň - zhoršená"
        case 4 | 6:
            return "Noc - nezhoršená"
        case 5 | 7:
            return "Noc - zhoršená"
        case _:
            return None


# Ukol 3: počty nehod v jednotlivých regionech podle viditelnosti
def plot_visibility(df: pd.DataFrame, fig_location: str = None,
                    show_figure: bool = False):
    df["p19t"] = [
        p19_enum_to_str(p19) for p19 in df["p19"]
    ]  # enum to weather condition type

    regions = df["region"].unique()[:4]
    region_names = {
        "PHA": "hl. m. Praha",
        "STC": "Středočeský kraj",
        "JHC": "Jihočeský kraj",
        "PLK": "Plzeňský kraj"
    }

    fig, axes = plt.subplots(2, 2, figsize=(12, 6))
    fig.subplots_adjust(hspace=.5)  # bigger space between plot rows
    fig.suptitle("Počet nehod dle regionu a viditelnosti")
    axx = [*axes[0], *axes[1]]  # 2d array to 1d

    for i, region in enumerate(regions):
        region_data = df[df["region"] == region]
        visibilities = region_data.groupby(["p19t"])

        visibility_names = list(visibilities.size().keys())
        visibility_values = visibilities.size().values

        bar = sns.barplot(ax=axx[i], x=visibility_names, y=visibility_values)
        for label in bar.containers:
            bar.bar_label(label, )  # add # of accidents above weather condition type

        axx[i].set_title(region_names.get(region, region))
        axx[i].set_xticklabels(visibility_names, rotation=10)
        axx[i].set_ylim(0, visibility_values.max() * 1.2)

    if fig_location:
        fig.savefig(fig_location, bbox_inches="tight")

    if show_figure:
        plt.show()

    plt.close(fig)


def p7_enum_to_str(enum: int):
    match enum:
        case 1:
            return "Čelná"
        case 2 | 3:
            return "Bočná"
        case 4:
            return "Zozadu"
        case _:
            return None


# Ukol4: druh srážky jedoucích vozidel
def plot_direction(df: pd.DataFrame, fig_location: str = None,
                   show_figure: bool = False):
    region_names = {
        "PHA": "hl. m. Praha",
        "STC": "Středočeský kraj",
        "JHC": "Jihočeský kraj",
        "PLK": "Plzeňský kraj"
    }

    df["p7"] = pd.Series(
        [
            p7_enum_to_str(p7)
            for p7 in df["p7"]
        ],
        dtype="category",
        index=df.index
    )  # convert collision type ID into word

    df["month"] = [
        date.month for date in df["date"]
    ]  # create column month with month of accident

    regions_data = df[df["region"].isin(region_names.keys())].groupby(["region"])  # filter for region's rows + group it

    with sns.axes_style("darkgrid"):  # set style
        fig, axes = plt.subplots(2, 2, figsize=(12, 6))
    axx = [*axes[0], *axes[1]]  # 2d array to 1d for easier iteration

    fig.subplots_adjust(hspace=.5)  # bigger space between plot rows

    for i, (region, region_data) in enumerate(regions_data):
        with sns.axes_style("darkgrid"):  # set style
            sns.countplot(data=region_data, x="month", hue="p7", ax=axx[i])
            # catplot automatically groups it by ["month", "p7"]

        axx[i].set(title=region_names.get(region, region))
        axx[i].set_ylabel("Počet nehôd")
        axx[i].set_xlabel("Mesiac")
        axx[i].get_legend().remove()  # there is figure-wide legend

    fig.legend(["Bočná",  "Čelná", "Zozadu"], title="Druh zrážky", loc="center right")

    if fig_location:
        fig.savefig(fig_location, bbox_inches="tight")

    if show_figure:
        plt.show()

    plt.close(fig)


def set_worst_conseq(row):
    """
        Function returns text representation of "type" of accident.
        Count of injuries in accident is meant as a "type" of accident.
    :param row: dataframe row
    :return: text representation of accident type
    """
    if row["p13a"] > max([row["p13b"], row["p13c"]]):
        conseq = "Usmrtenie"
    elif row["p13b"] > max([row["p13a"], row["p13c"]]):
        conseq = "Tazke zranenia"
    else:
        conseq = "Lahke zranenia"

    return conseq


# Ukol 5: Následky v čase
def plot_consequences(df: pd.DataFrame, fig_location: str = None,
                    show_figure: bool = False):
    """
        Function plots consequences of accidents in chosen range.
    :param df: parsed data frame
    :param fig_location: file name of figure
    :param show_figure: whether function shows plot
    """
    region_names = {
        "PHA": "hl. m. Praha",
        "STC": "Středočeský kraj",
        "JHC": "Jihočeský kraj",
        "PLK": "Plzeňský kraj"
    }
    d_from = np.datetime64("2016-01-01")
    d_to = np.datetime64("2022-01-01")

    regions_data = df[
        (df["region"].isin(region_names.keys())) &  # filter for selected regions only
        ((d_from < df["date"]) & (df["date"] < d_to)) &  # filter accidents within dates
        ((df["p13a"] > 0) | (df["p13b"] > 0) | (df["p13c"] > 0))  # filter accidents without injuries
    ]

    regions_data["conseq"] = regions_data.apply(set_worst_conseq, axis=1)  # transform accident type into text form

    con = pd.pivot_table(
        regions_data,
        aggfunc="count",
        index=["region", "date"],
        columns=["conseq"],  # aggregate consequences
        fill_value=0,
        values="p14"  # every row contains it (at my dataset, hopefully on yours also)
    )

    no_index = con.reset_index()
    resampled = no_index.groupby("region").resample("M", on="date").sum()

    with sns.axes_style("darkgrid"):  # set style of subplots
        fig, axes = plt.subplots(2, 2, figsize=(12, 6))

    fig.subplots_adjust(hspace=.5)  # bigger space between plot rows
    fig.suptitle("Počet nehôd podla regionu a viditelnosti")
    axx = [*axes[0], *axes[1]]  # 2d array to 1d

    labels = ["01/16", "01/17", "01/18", "01/19", "01/20", "01/21", "01/22"]
    locator = LinearLocator(numticks=len(labels))  # i know, not the best solution

    for i, (region, data) in enumerate(resampled.groupby("region")):
        axx[i].margins(x=0)
        with sns.axes_style("darkgrid"):  # set style of liness
            p = data.plot(kind="line", ax=axx[i])

        p.get_xaxis().set_major_locator(locator)
        p.set_xticklabels(labels)
        p.set_xlabel("Datum")
        if i % 2 == 0:  # add it to the first plot in row (2 plots in row => %2)
            p.set_ylabel("Počet nehôd")

        p.get_legend().remove()
        p.set_title(f"Kraj: {region}")

    fig.legend(["Lahke zranenia", "Tazke zranenia", "Usmrtenie"], title="Nasledky", loc="center right",
               bbox_to_anchor=(1.07, .5))

    if fig_location:
        fig.savefig(fig_location, bbox_inches="tight")

    if show_figure:
        plt.show()

    plt.close(fig)


if __name__ == "__main__":
    # zde je ukazka pouziti, tuto cast muzete modifikovat podle libosti
    # skript nebude pri testovani pousten primo, ale budou volany konkreni 
    # funkce.
    df = load_data("data/data.zip")
    df2 = parse_data(df, True)

    plot_visibility(df2, "01_visibility.png")
    plot_direction(df2, "02_direction.png", True)
    plot_consequences(df2, "03_consequences.png")


# Poznamka:
# pro to, abyste se vyhnuli castemu nacitani muzete vyuzit napr
# VS Code a oznaceni jako bunky (radek #%%% )
# Pak muzete data jednou nacist a dale ladit jednotlive funkce
# Pripadne si muzete vysledny dataframe ulozit nekam na disk (pro ladici
# ucely) a nacitat jej naparsovany z disku
