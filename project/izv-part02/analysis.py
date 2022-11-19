#!/usr/bin/env python3.9
# coding=utf-8

from matplotlib import pyplot as plt
import pandas as pd
import seaborn as sns
import numpy as np
import zipfile
import io

# muzete pridat libovolnou zakladni knihovnu ci knihovnu predstavenou na prednaskach
# dalsi knihovny pak na dotaz


# Ukol 1: nacteni dat ze ZIP souboru
def load_data(filename : str) -> pd.DataFrame:
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
        index_col=0
    )
    return df

# Ukol 2: zpracovani dat
def parse_data(df : pd.DataFrame, verbose : bool = False) -> pd.DataFrame:
    pass

# Ukol 3: počty nehod v jednotlivých regionech podle viditelnosti
def plot_visibility(df: pd.DataFrame, fig_location: str = None,
                    show_figure: bool = False):
    pass

# Ukol4: druh srážky jedoucích vozidel
def plot_direction(df: pd.DataFrame, fig_location: str = None,
                   show_figure: bool = False):
    pass

# Ukol 5: Následky v čase
def plot_consequences(df: pd.DataFrame, fig_location: str = None,
                    show_figure: bool = False):
    pass

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
