#!/usr/bin/env python3
"""
Compare total pertinent edges vs NV pops for each input (dense graphs).
Generates one plot per weight family and benchmark.
"""

from pathlib import Path
from typing import Dict

import matplotlib.pyplot as plt
import pandas as pd


def load_avg(csv_path: Path) -> pd.DataFrame:
    df = pd.read_csv(csv_path)
    required = ["filename", "m", "avg_nv_pops", "avg_pert"]
    missing = [c for c in required if c not in df.columns]
    if missing:
        raise ValueError(f"Missing columns {missing} in {csv_path}")
    return df.sort_values("m")


def plot_for_weight(root: Path, weight_dir: str, weight_label: str, weight_color: str, benchmark: str) -> None:
    csv_path = root / weight_dir / benchmark / "average.csv"
    if not csv_path.exists():
        print(f"Skip missing file: {csv_path}")
        return
    df = load_avg(csv_path)

    x = range(len(df))
    width = 0.4

    plt.figure(figsize=(10, 5))
    plt.bar([i - width / 2 for i in x], df["avg_nv_pops"], width=width, color=weight_color, alpha=0.8, label="NV pops")
    plt.bar([i + width / 2 for i in x], df["avg_pert"], width=width, color="gray", alpha=0.6, label="Pertinent edges")

    plt.xticks(list(x), df["filename"], rotation=45, ha="right")
    plt.yscale("log")
    plt.ylabel("Count (log scale)")
    plt.xlabel("Input file (sorted by |E|)")
    plt.title(f"Dense — NV pops vs pertinent — {weight_label} — {benchmark}")
    plt.grid(True, axis="y", linestyle=":", alpha=0.4)
    plt.legend()
    plt.tight_layout()
    out = root / f"pops_vs_pertinent_{weight_dir}_{benchmark}.png"
    plt.savefig(out, dpi=200)
    print(f"Saved {out}")
    plt.close()


def main() -> None:
    root = Path(__file__).parent
    weight_labels: Dict[str, str] = {
        "exponential_weights": "Exponential",
        "original_weights": "Geometric",
        "random_weights": "Random",
    }
    weight_colors: Dict[str, str] = {
        "exponential_weights": "#1f77b4",
        "original_weights": "#2ca02c",
        "random_weights": "#d62728",
    }
    for benchmark in ("spt_benchmark", "src_dst_benchmark"):
        for wdir, wlabel in weight_labels.items():
            plot_for_weight(root, wdir, wlabel, weight_colors[wdir], benchmark)


if __name__ == "__main__":
    main()
