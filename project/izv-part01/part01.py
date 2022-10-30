#!/usr/bin/env python3
"""
IZV cast1 projektu
Autor: Samuel Dobroň

Detailni zadani projektu je v samostatnem projektu e-learningu.
Nezapomente na to, ze python soubory maji dane formatovani.

Muzete pouzit libovolnou vestavenou knihovnu a knihovny predstavene na prednasce
"""


from bs4 import BeautifulSoup
import requests
import numpy as np
import matplotlib.pyplot as plt
from typing import List


def integrate(x: np.array, y: np.array) -> float:
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
    colors = ["blue", "orange", "green"]
    x = np.linspace(-3, 3, 100)
    xsq = x**2
    ys = xsq * np.array([a]).T

    fig = plt.figure(figsize=(6, 4))
    ax = fig.add_subplot()
    ax.margins(0, 0)
    ax.spines["top"].set_bounds(-3, 4.2)
    ax.spines["bottom"].set_bounds(-3, 4.2)
    ax.spines["right"].set_position(("data", 4.2))

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
    fig.set_ylim(-0.8, 0.9)
    fig.yaxis.set_ticks(np.arange(-0.8, 0.9, 0.4))
    fig.set_xlabel("t")
    fig.margins(0, 0)
    if not isinstance(thresholds, np.ndarray):
        fig.plot(t, func, color="blue")
    else:
        positive = func > thresholds
        tcopy = t.copy()
        funccopy = func.copy()
        tcopy[~positive] = np.nan
        funccopy[~positive] = np.nan

        fig.plot(tcopy, funccopy, color="green")
        t[positive] = np.nan
        func[positive] = np.nan
        fig.plot(t, func, color="red")


def generate_sinus(show_figure: bool = False, save_path: str | None = None):

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


def to_int(val):
    try:
        return int(val.text)
    except ValueError:
        return 0


def to_float(val):
    try:
        return np.float64(val.text.replace(",", "."))
    except (ValueError, AttributeError):
        return np.nan


def download_data(url="https://ehw.fit.vutbr.cz/izv/temp.html"):
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


def filter_data(table, key, value):
    return list(
        filter(
            lambda x: x[key] == value,
            table
        )
    )


def get_avg_temp(data, year=None, month=None) -> float:
    if year is not None:
        data = filter_data(data, "year", year)

    if month is not None:
        data = filter_data(data, "month", month)

    sum_temp = np.float64(0)
    len_temp = np.uint32(0)

    for row in data:
        sum_temp += np.sum(row["temp"])
        len_temp += row["temp"].size  # 1d array

    return sum_temp / len_temp
