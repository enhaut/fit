#!/usr/bin/env python3
"""
IZV cast1 projektu
Autor: 

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


def generate_sinus(show_figure: bool=False, save_path: str | None=None):
    pass


def download_data(url="https://ehw.fit.vutbr.cz/izv/temp.html"):
    pass


def get_avg_temp(data, year=None, month=None) -> float:
    pass
