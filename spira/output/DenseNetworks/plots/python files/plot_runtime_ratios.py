#!/usr/bin/env python3
"""Plot runtime ratios for dense (complete) graphs.

Reads average.csv files under each weight type (exponential, original,
random) for both spt_benchmark and src_dst_benchmark and plots:
  - spira/dijkstra (orange)
  - new_variant/dijkstra (purple)
X-axis uses n (nodes) sorted ascending so the curve goes small→large.
Creates PNGs in the same folder.
"""
from pathlib import Path
from typing import Dict

import matplotlib.pyplot as plt
import pandas as pd
from matplotlib.lines import Line2D


def load_and_prepare(csv_path: Path) -> pd.DataFrame:
    """Read average.csv and add ratio columns."""
    df = pd.read_csv(csv_path)
    df = df.sort_values("n")  # enforce small→large by size
    df["spira_ratio"] = df["avg_s_time"] / df["avg_d_time"]
    df["new_variant_ratio"] = df["avg_nv_time"] / df["avg_d_time"]
    return df


def plot_for_benchmark(
    root: Path,
    benchmark: str,
    weight_labels: Dict[str, str],
    weight_colors: Dict[str, str],
) -> None:
    """Plot ratios for a given benchmark (spt_benchmark or src_dst_benchmark)."""
    fig, ax = plt.subplots(figsize=(8, 5))

    ratio_styles = {
        "spira_ratio": ("o", "-"),
        "new_variant_ratio": ("s", "--"),
    }
    sample_df = None

    for weight_dir, weight_label in weight_labels.items():
        csv_path = root / weight_dir / benchmark / "average.csv"
        if not csv_path.exists():
            print(f"Skip missing file: {csv_path}")
            continue
        df = load_and_prepare(csv_path)
        if sample_df is None:
            sample_df = df

        for ratio_key, (marker, linestyle) in ratio_styles.items():
            xs = list(range(len(df)))
            ax.plot(
                xs,
                df[ratio_key],
                color=weight_colors[weight_dir],
                marker=marker,
                linestyle=linestyle,
                label=None,
            )

    # x-axis labels are filenames, ordered by increasing n
    if sample_df is not None and not sample_df.empty:
        ax.set_xticks(range(len(sample_df)))
        ax.set_xticklabels(sample_df["filename"], rotation=45, ha="right")

    ax.axhline(1.0, color="gray", linewidth=1, linestyle="--", alpha=0.5)
    ax.set_xlabel("Graph Instances (sorted by |V|)")
    ax.set_ylabel("Relative runtime compared to Dijkstra")
    ax.set_title(f"Dense graphs ({benchmark})")
    ax.grid(True, which="both", linestyle=":", alpha=0.4)

    # Two-level legend: edge-weight colors + ratio markers/linestyles.
    weight_handles = [
        Line2D([0], [0], color=color, lw=2, label=weight_labels[weight_dir])
        for weight_dir, color in weight_colors.items()
    ]
    ratio_handles = [
        Line2D(
            [0],
            [0],
            color="black",
            marker=marker,
            linestyle=linestyle,
            label=ratio_key.replace("_ratio", "/dijkstra"),
            markersize=6,
        )
        for ratio_key, (marker, linestyle) in ratio_styles.items()
    ]
    legend1 = ax.legend(handles=weight_handles, title="Edge weights", loc="upper left")
    legend2 = ax.legend(handles=ratio_handles, title="Ratios", loc="upper right")
    ax.add_artist(legend1)
    ax.add_artist(legend2)

    output_path = root / f"runtime_ratios_{benchmark}.png"
    fig.tight_layout()
    fig.savefig(output_path, dpi=200)
    print(f"Saved {output_path}")


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
        plot_for_benchmark(root, benchmark, weight_labels, weight_colors)


if __name__ == "__main__":
    main()
