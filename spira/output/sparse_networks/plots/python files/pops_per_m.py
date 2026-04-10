#!/usr/bin/env python3
"""
Sparse graphs only:
  - NV pops-per-m vs m for all weight families (one plot per benchmark).
"""

from pathlib import Path
from typing import Dict

import matplotlib.pyplot as plt
import pandas as pd
from matplotlib import ticker


def load_and_prepare(csv_path: Path) -> pd.DataFrame:
    """Read average.csv and compute NV pops per m."""
    df = pd.read_csv(csv_path)
    df = df.sort_values("m")
    if "avg_nv_pops" not in df.columns or "m" not in df.columns:
        raise ValueError(f"Missing avg_nv_pops or m in {csv_path}")
    df["nv_per_m"] = df["avg_nv_pops"] / df["m"]
    return df


def plot_pops_per_m(
    root: Path,
    benchmark: str,
    weight_labels: Dict[str, str],
    weight_colors: Dict[str, str],
) -> None:
    """Pops-per-m for NV per file; grouped bars for each weight family."""
    plt.figure(figsize=(11, 5))
    plotted = False
    base_order = None
    n_weights = len(weight_labels)
    width = 0.8 / max(n_weights, 1)

    for idx, (weight_dir, weight_label) in enumerate(weight_labels.items()):
        csv_path = root / weight_dir / benchmark / "average.csv"
        if not csv_path.exists():
            print(f"Skip missing file: {csv_path}")
            continue
        df = load_and_prepare(csv_path)
        if base_order is None:
            base_order = list(df["filename"])
        df = df.set_index("filename").reindex(base_order)
        x = range(len(df))
        offset = (idx - (n_weights - 1) / 2) * width
        plt.bar(
            [i + offset for i in x],
            df["nv_per_m"],
            width=width,
            color=weight_colors[weight_dir],
            alpha=0.9,
            label=weight_label,
        )
        plotted = True

    if not plotted or base_order is None:
        print(f"No data to plot for {benchmark}")
        return

    plt.xticks(range(len(base_order)), base_order, rotation=45, ha="right")
    plt.xlabel("Input file (sorted by |E|)")
    plt.ylabel("NV pops per edge (pops / m)")
    plt.title(f"Sparse graphs — NV pops per m — {benchmark}")
    plt.grid(axis="y", linestyle=":", alpha=0.4)
    plt.legend(title="Edge weights")

    ax = plt.gca()
    ax.yaxis.set_major_formatter(ticker.ScalarFormatter())
    ax.ticklabel_format(style="plain", axis="y")

    plt.tight_layout()
    out = root / f"sparse_nv_pops_per_m_{benchmark}.png"
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
        plot_pops_per_m(root, benchmark, weight_labels, weight_colors)


if __name__ == "__main__":
    main()
