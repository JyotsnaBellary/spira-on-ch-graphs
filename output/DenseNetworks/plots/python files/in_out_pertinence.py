#!/usr/bin/env python3
"""
Dense graphs: in/out SPT pertinence comparisons (grouped bars).
"""

from pathlib import Path
from typing import Dict, Iterable

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


def load_avg(csv_path: Path) -> pd.DataFrame:
    df = pd.read_csv(csv_path)
    df = df.sort_values("m")
    required = ["filename", "ratio_spt_out", "ratio_spt_in"]
    missing = [c for c in required if c not in df.columns]
    if missing:
        raise ValueError(f"Missing columns {missing} in {csv_path}")
    return df


def plot_in_out_share(
    root: Path,
    benchmark: str,
    weight_labels: Dict[str, str],
    weight_colors: Dict[str, str],
) -> None:
    plt.figure(figsize=(11, 5))
    datasets = []
    base_order: Iterable[str] | None = None

    # Collect datasets and keep a consistent filename ordering.
    for wdir, wlabel in weight_labels.items():
        csv_path = root / wdir / benchmark / "average.csv"
        if not csv_path.exists():
            print(f"Skip missing file: {csv_path}")
            continue
        df = load_avg(csv_path)
        if base_order is None:
            base_order = list(df["filename"])
        datasets.append((wlabel, weight_colors[wdir], df.set_index("filename")))

    if not datasets:
        print(f"No data for in/out plot: {benchmark}")
        return

    filenames = list(base_order or [])
    x = np.arange(len(filenames))
    total_series = len(datasets) * 2
    bar_width = 0.8 / max(total_series, 1)
    start = x - (total_series - 1) * bar_width / 2

    for idx, (label, color, df) in enumerate(datasets):
        ordered = df.reindex(filenames)
        out_vals = ordered["ratio_spt_out"]
        in_vals = ordered["ratio_spt_in"]
        pos_out = start + idx * 2 * bar_width
        pos_in = pos_out + bar_width
        plt.bar(pos_out, out_vals, width=bar_width, color=color, alpha=0.85, label=f"{label} out")
        plt.bar(pos_in, in_vals, width=bar_width, color=color, alpha=0.45, label=f"{label} in")

    plt.xticks(x, filenames, rotation=45, ha="right")
    plt.ylim(0, 1)
    plt.xlabel("Graph Instances (sorted by |E|)")
    plt.ylabel("Share of SPT edges (out vs in)")
    plt.title(f"Dense — SPT pertinence (out vs in) — {benchmark}")
    plt.grid(True, axis="y", linestyle=":", alpha=0.4)
    plt.legend(title="Edge weights × direction", ncol=2)
    plt.tight_layout()
    out = root / f"in_out_pertinent_{benchmark}.png"
    plt.savefig(out, dpi=200)
    print(f"Saved {out}")


def main() -> None:
    root = Path(__file__).parent
    weight_labels = {
        "exponential_weights": "Exponential",
        "original_weights": "Geometric",
        "random_weights": "Random",
    }
    weight_colors = {
        "exponential_weights": "#1f77b4",
        "original_weights": "#2ca02c",
        "random_weights": "#d62728",
    }
    for benchmark in ("spt_benchmark", "src_dst_benchmark"):
        plot_in_out_share(root, benchmark, weight_labels, weight_colors)


if __name__ == "__main__":
    main()
