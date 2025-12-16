#!/usr/bin/env python3
"""
Sparse graphs: pertinence-focused plots.

Outputs per benchmark:
  1) Pertinent share vs m (ratio_pert_m) across weight families.
"""

from pathlib import Path
from typing import Dict

import matplotlib.pyplot as plt
import pandas as pd


def load_avg(csv_path: Path) -> pd.DataFrame:
    df = pd.read_csv(csv_path)
    df = df.sort_values("m")
    required = ["filename", "m", "ratio_pert_m"]
    missing = [c for c in required if c not in df.columns]
    if missing:
        raise ValueError(f"Missing columns {missing} in {csv_path}")
    return df


def plot_pertinent_share(
    root: Path,
    benchmark: str,
    weight_labels: Dict[str, str],
    weight_colors: Dict[str, str],
) -> None:
    plt.figure(figsize=(9, 5))
    plotted = False
    for wdir, wlabel in weight_labels.items():
        csv_path = root / wdir / benchmark / "average.csv"
        if not csv_path.exists():
            print(f"Skip missing file: {csv_path}")
            continue
        df = load_avg(csv_path)
        plt.plot(
            df["filename"],
            df["ratio_pert_m"],
            marker="o",
            linestyle="-",
            color=weight_colors[wdir],
            label=wlabel,
        )
        plotted = True
    if not plotted:
        print(f"No data for {benchmark}")
        return

    plt.xlabel("Graph file")
    plt.ylabel("Pertinent share (pertinent / m)")
    plt.title(f"Sparse — Pertinent share by graph — {benchmark}")
    plt.grid(True, linestyle=":", alpha=0.4)
    plt.legend(title="Edge weights")
    plt.xticks(rotation=45, ha="right")
    plt.tight_layout()
    out = root / f"pertinent_share_{benchmark}.png"
    plt.savefig(out, dpi=200)
    print(f"Saved {out}")


def main() -> None:
    root = Path(__file__).parent
    weight_labels = {
        "exponential_weights": "Exponential",
        "original_weights": "Uniform",
        "random_weights": "Random",
    }
    weight_colors = {
        "exponential_weights": "#1f77b4",
        "original_weights": "#2ca02c",
        "random_weights": "#d62728",
    }

    for benchmark in ("spt_benchmark", "src_dst_benchmark"):
        plot_pertinent_share(root, benchmark, weight_labels, weight_colors)


if __name__ == "__main__":
    main()
