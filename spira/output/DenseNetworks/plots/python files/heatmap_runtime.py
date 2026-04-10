#!/usr/bin/env python3
"""
Heatmaps: runtime comparison for dense graphs across edge-weight distributions.
Reads average.csv from DenseNetworks/*/spt_benchmark (and src_dst_benchmark) and
plots mean runtimes for Dijkstra, Spira, and New Variant.
"""

from pathlib import Path
from typing import Dict

import matplotlib.pyplot as plt
from matplotlib import colors
from matplotlib.colors import LinearSegmentedColormap
from matplotlib.ticker import MaxNLocator, ScalarFormatter
import numpy as np
import pandas as pd
import seaborn as sns


ROOT = Path(__file__).parent


def load_avg(root: Path, weight_dir: str, benchmark: str) -> pd.Series | None:
    csv_path = root / weight_dir / benchmark / "average.csv"
    if not csv_path.exists():
        print(f"Missing: {csv_path}")
        return None
    df = pd.read_csv(csv_path)
    # Average across files for this weight type
    agg = df[["avg_d_time", "avg_s_time", "avg_nv_time"]].mean()
    agg["weight"] = weight_dir
    return agg


def build_matrix(root: Path, benchmark: str, weight_labels: Dict[str, str]) -> pd.DataFrame:
    rows = []
    for wdir, label in weight_labels.items():
        series = load_avg(root, wdir, benchmark)
        if series is None:
            continue
        row = pd.Series(
            {
                "Dijkstra": series["avg_d_time"],
                "Spira": series["avg_s_time"],
                "New Variant": series["avg_nv_time"],
            },
            name=label,
        )
        rows.append(row)
    if not rows:
        return pd.DataFrame()
    return pd.DataFrame(rows)


def plot_heatmap(matrix: pd.DataFrame, benchmark: str, out_path: Path) -> None:
    plt.figure(figsize=(6, 4))
    vals = matrix.to_numpy().astype(float)
    vmin = vals.min()
    vmax = vals.max()
    # Lavender/blue to coral soft gradient, continuous.
    cmap = LinearSegmentedColormap.from_list(
        "blue_to_red_soft",
        [
            (0.0, "#cddff7"),  # light lavender/blue
            (0.35, "#7ba3ff"),  # slightly darker mid blue
            (0.6, "#f2c8c8"),   # light coral
            (0.8, "#f98c8c"),   # mid red
            (1.0, "#ff927d"),   # soft red
        ],
    )
    norm = colors.LogNorm(vmin=max(vmin, 1e-9), vmax=vmax)
    ax = sns.heatmap(
        matrix,
        annot=True,
        fmt=".1f",
        cmap=cmap,
        norm=norm,
        cbar_kws={"label": "Runtime (avg ms)"},
        linewidths=0.5,
    )
    cbar = ax.collections[0].colorbar
    cbar.locator = MaxNLocator(nbins=7)
    cbar.formatter = ScalarFormatter(useMathText=True)
    cbar.update_ticks()
    plt.title(f"Dense runtime (avg) — {benchmark}")
    plt.ylabel("Edge-weight distribution")
    plt.xlabel("SSSP Algorithms")
    plt.tight_layout()
    plt.savefig(out_path, dpi=200)
    print(f"Saved {out_path}")


def main() -> None:
    weight_labels = {
        "exponential_weights": "Exponential",
        "original_weights": "Geometric",
        "random_weights": "Random",
    }
    for benchmark in ("spt_benchmark", "src_dst_benchmark"):
        matrix = build_matrix(ROOT, benchmark, weight_labels)
        if matrix.empty:
            print(f"No data for {benchmark}")
            continue
        out = ROOT / f"heatmap_runtime_dense_{benchmark}.png"
        plot_heatmap(matrix, benchmark, out)


if __name__ == "__main__":
    main()
