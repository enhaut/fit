import pandas as pd
import matplotlib.pyplot as plt


def process_df(path="accidents.pkl.gz"):
    """
        Function loads and parses data frame from file.
    :param path: path to the file
    :return: parsed dataframe
    """
    df = pd.read_pickle(path)

    df["p16"] = pd.to_numeric(df["p16"], "coerce", "integer")  # "mark" invalid entries
    df["p2a"] = pd.to_datetime(df['p2a'])
    df["year"] = df["p2a"].dt.year

    return df[
        (
                (df["p16"].notnull()) &   # accident surface conditions
                (0 <= df["p16"]) &
                (df["p16"] <= 9) &
                (df["p13a"].notnull()) &  # deaths
                (df["p13b"].notnull()) &  # serious injuries
                (df["p13b"].notnull())    # light injuries
        )
    ]  # filter invalid entries


def surface_condition_table(df: pd.DataFrame):
    """
        Function prints summary table of accidents per surface type
    :param df: parsed dataframe
    """

    road_conditions = ["Other", "Dry", "Dry (dirty)", "Wet", "Dirt on road", "Ice - gritted", "Ice - ungritted",
                       "Spilled (gas, oil, ...)", "Ice coating", "Ice coating on the bridges"]
    df["p16a"] = df.apply(lambda r: road_conditions[r["p16"]], axis=1)  # set string surface condition for eery row
    df_ygroups = df.groupby("p16a")

    print(df_ygroups["p1"].count().reset_index().sort_values("p1", ascending=False).style.to_latex())

    print("Most expensive categories\n",
          df_ygroups["p14"].mean(numeric_only=True).sort_values(ascending=False).iloc[:2])


def plot_accidents_within_years(df: pd.DataFrame):
    """
        Function plots accidents on surface conditions as year to year trend stats

    :param df: processed data frame
    """
    fig = plt.figure(figsize=(10, 5))
    ax = fig.add_subplot()

    plot = pd.crosstab(df['year'], df['p16a']).plot(ax=ax, logy=True, legend=False)

    handles, labels = plot.get_legend_handles_labels()
    order = [7, 0, 5, 3, 2, 4, 8, 6, 9, 1]  # array value defines order in legend and element position defines type
    order.reverse()  # its reversed, but find it more descriptive as it is

    plot.legend(
        [handles[idx] for idx in order],
        [labels[idx] for idx in order],
        title='Road surface condition',
        bbox_to_anchor=(1, 0.9)
    )

    plot.set_ylabel("Number of accidents")
    plot.set_xlabel("Year")
    ax.margins(0, 0.05)
    plt.tight_layout()
    ax.set_title("Year to year trends of accidents on road surface condition")
    fig.savefig("fig.png", bbox_inches="tight")
    plt.close(fig)


def injuries_in_category(accidents: pd.DataFrame, injury_col: str):
    """
        Functions calculates ratio of injury_col accidents / accidents

    :param accidents: table with desired entries only
    :param injury_col: metric column
    :return: ratio of injury_col accidents / accidents
    """
    accidents_count = len(accidents)

    injured = accidents[accidents[injury_col] > 0]
    return round(len(injured) / accidents_count * 100, 2)


def icy_sufrace_injuries(df: pd.DataFrame):
    """
        Function returns ratios of deadly and light injuries accidents on icy surfaces.
    :param df: processed dataset
    :return: ratio of deadly, light injuries accidents
    """
    icy_surface_accidents = df[((df["p16a"] == "Ice - ungritted") | (df["p16a"] == "Ice - gritted") | (
            df["p16a"] == "Ice coating") | (df["p16a"] == "Ice coating on the bridges"))]

    deaths = injuries_in_category(icy_surface_accidents, "p13a")
    light_injuries = injuries_in_category(icy_surface_accidents, "p13c")
    return deaths, light_injuries


def drywet_sufrace_injuries(df: pd.DataFrame):
    """
            Function returns ratios of deadly and light injuries accidents on dry+wet surfaces.
        :param df: processed dataset
        :return: ratio of deadly, light injuries accidents
        """
    surface_accidents = df[((df["p16a"] == "Dry") | (df["p16a"] == "Dry (dirty)") | (df["p16a"] == "Wet"))]

    deaths = injuries_in_category(surface_accidents, "p13a")
    light_injuries = injuries_in_category(surface_accidents, "p13c")
    return deaths, light_injuries


def surface_injuries_ratios(df: pd.DataFrame):
    """
        Function prints ratios of fatality of surface condition
    :param df: processed dataframe
    """

    icy_deaths, icy_light_injuries = icy_sufrace_injuries(df)
    deaths, light_injuries = drywet_sufrace_injuries(df)

    print(f"% of deadly accidents on icy roads: {icy_deaths}")
    print(f"% of deadly accidents on dry/dry (dirty) and wet roads: {deaths}")

    print(f"% of light injuries accidents on icy roads: {icy_light_injuries}")
    print(f"% of light injuries accidents on dry/dry (dirty) and wet roads: {light_injuries}")


if __name__ == "__main__":
    df = process_df()
    surface_condition_table(df)
    plot_accidents_within_years(df)
    surface_injuries_ratios(df)
