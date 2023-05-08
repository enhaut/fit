#!/usr/bin/env python3
"""
IZV cast1 projektu
Autor: Samuel Dobroň
Verzia: Python 3.10.7

Detailni zadani projektu je v samostatnem projektu e-learningu.
Nezapomente na to, ze python soubory maji dane formatovani.

Muzete pouzit libovolnou vestavenou knihovnu a knihovny predstavene na prednasce
"""
import bs4
from bs4 import BeautifulSoup
import requests
import numpy as np
import matplotlib.pyplot as plt
from typing import List, Any


def integrate(x: np.array, y: np.array) -> float:
    """
        Function returns area under the function defined by x and y arrays, where:

        :param x: points on axis X
        :param y: values of function on axis Y
        :return: area as a float
    """
    return np.sum(
        (
                x[1:] - x[:-1]
        ) * (
                (
                        y[:-1] + y[1:]
                )/2
        )
    )


def generate_graph(
    a: List[float], show_figure: bool = False, save_path: str | None = None
):
    """
        Function generates graph for function f(x) = a[i] * x^2, where i comes from list a

    :param a: list of coefficients
    :param show_figure: boolean value whether show plot or not
    :param save_path: path for saving generated plot
    """
    colors = ["blue", "orange", "green"]
    x = np.linspace(-3, 3, 100)
    xsq = x**2
    ys = xsq * np.array([a]).T  # transposing it to get 3 rows of 1 coefficient that multiplies xsq

    fig = plt.figure(figsize=(6, 4))
    ax = fig.add_subplot()
    ax.margins(0, 0)  # make curve start at the axis
    ax.spines["top"].set_bounds(-3, 4.2)
    ax.spines["bottom"].set_bounds(-3, 4.2)
    ax.spines["right"].set_position(("data", 4.2))
    # ^^^ move right axis to make space for annotations

    ax.set_ylim(-20, 20)
    ax.set_ylabel(r"$f_{a}(x)$")

    for i in range(a_len := len(a)):
        ax.plot(
            x, ys[i], color=colors[i % 3], label=rf"$\gamma_{ {a[i]} }(x)$"
        )
        ax.fill_between(
            x, ys[i], color=colors[i % 3], alpha=0.1, label="_nolegend_"
        )
        ax.annotate(
            rf"$\int f_{ {a[i]} }(x)dx$",
            xy=(x[-1], ys[i, -1]),
            xytext=(0, -4),
            textcoords="offset points",
        )

    fig.legend(loc="upper center", ncol=a_len)

    if show_figure:
        plt.show()
    elif save_path:
        fig.savefig(save_path, bbox_inches="tight")
    plt.close(fig)


def plot_func(fig, t, func, thresholds=None):
    """
        Function draws function on figure.

    :param fig: figure to draw
    :param t: array time (a.k.a. X axis)
    :param func: array of values (Y axis)
    :param thresholds: list of values whose are used as condition for line color
    """
    fig.set_ylim(-0.8, 0.9)
    fig.yaxis.set_ticks(np.arange(-0.8, 0.9, 0.4))
    fig.set_xlabel("t")
    fig.margins(0, 0)
    if not isinstance(thresholds, np.ndarray):  # should be faster than comparing by "is not None"
        fig.plot(t, func, color="blue")
    else:
        positive = func > thresholds
        tcopy = t.copy()
        funccopy = func.copy()

        # hide negative (red) part of values
        tcopy[~positive] = np.nan
        funccopy[~positive] = np.nan

        fig.plot(tcopy, funccopy, color="green")

        # hide positive (green) part of values
        t[positive] = np.nan
        func[positive] = np.nan

        fig.plot(t, func, color="red")


def generate_sinus(show_figure: bool = False, save_path: str | None = None):
    """
        Function generates 3 sinus waves.

    :param show_figure: boolean value whether show plot or not
    :param save_path: path for saving generated plot
    """
    t = np.linspace(0, 100, 10000)
    f1 = np.double(0.5) * np.sin(np.pi * t * np.double(1 / 50))
    f2 = np.double(0.25) * np.sin(np.pi * t)

    fig = plt.figure(figsize=(8, 12))
    a1, a2, a3 = fig.subplots(nrows=3)

    plot_func(a1, t, f1)
    plot_func(a2, t, f2)
    plot_func(a3, t, f1 + f2, f1)

    if show_figure:
        plt.show()
    elif save_path:
        fig.savefig(save_path, bbox_inches="tight")
    plt.close(fig)


def to_int(val: bs4.element.Tag):
    """
        Function converts tag text to int.

    :param val: tag that contains string representation of int
    :return: int(val); in case of error 0
    """
    try:
        return int(val.text)
    except ValueError:
        return 0


def to_float(val: bs4.element.Tag):
    """
        Function converts tag text to float64.

    :param val: tag that contains string representation of float value
    :return: float value of val; on error np.nan is returned
    """
    try:
        return np.float64(val.text.replace(",", "."))
    except (ValueError, AttributeError):
        return np.nan


def download_data(url="https://ehw.fit.vutbr.cz/izv/temp.html") -> list[dict[str, Any]]:
    """
        Function downloads and parses data from table at url.

    :param url: url to gather data from
    :return: list of dictionaries that contains parsed data; on error empty list
    """
    try:
        request = requests.get(url)
    except requests.exceptions.RequestException as e:
        print(f"Could not get page: {e}")
        return []

    soup = BeautifulSoup(request.content, "html.parser")
    rows = soup.select("table > tr")
    parsed = []

    for row in rows:
        cols = row.select("td > p")
        parsed.append(
            {
                "year": to_int(cols[0]),
                "month": to_int(cols[1]),
                "temp": np.fromiter(map(to_float, cols[2:]), dtype=np.float64)
            }
        )
    return parsed


def filter_data(table: list[dict[str, Any]], key: str, value: Any) -> list[dict[str, Any]]:
    """
        Function filters list of dictionaries based on element[key] == value condition.

    :param table: list of dictionaries from download_data() method
    :param key: key used to filter
    :param value: value used to filter
    :return: list of filtered dictionaries
    """
    return list(
        filter(
            lambda x: x[key] == value,
            table
        )
    )


def get_avg_temp(data: list[dict[str, Any]], year: int = None, month: int = None) -> float:
    """
        Method returns average temperature based on year/month, year+month.

    :param data: data to get average from
    :param year: year
    :param month: month
    :return: average temperature; 0 on error
    """
    if year is not None:
        data = filter_data(data, "year", year)

    if month is not None:
        data = filter_data(data, "month", month)

    sum_temp = np.float64(0)
    len_temp = np.uint32(0)

    for row in data:
        sum_temp += np.sum(row["temp"])
        len_temp += row["temp"].size  # 1d array

    if len_temp > 0:
        return sum_temp / len_temp

    return 0
