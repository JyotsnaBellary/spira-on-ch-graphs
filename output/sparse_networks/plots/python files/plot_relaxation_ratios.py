#!/usr/bin/env python3
"""Relaxation ratios for sparse graphs: Spira pops / New Variant pops vs m."""
from pathlib import Path
from typing import Dict

import matplotlib.pyplot as plt
import pandas as pd
from matplotlib.lines import Line2D


def load_and_prepare(csv_path: Path) -> pd.DataFrame:
    """Read average.csv and add ratio columns; uses pops as relax proxy."""
    df = pd.read_csv(csv_path)
    df = df.sort_values("m")  # order by edge count

    s_col = "avg_s_pops"
    nv_col = "avg_nv_pops"
    if s_col not in df.columns or nv_col not in df.columns:
        missing = [c for c in (s_col, nv_col) if c not in df.columns]
        raise ValueError(f"Missing columns {missing} in {csv_path}")

    df["relax_ratio"] = df[s_col] / df[nv_col]
    return df


def plot_for_benchmark(
    root: Path,
    benchmark: str,
    weight_labels: Dict[str, str],
    weight_colors: Dict[str, str],
) -> None:
    """Plot relaxation ratios vs filename for a given benchmark."""
    fig, ax = plt.subplots(figsize=(8, 5))
    plotted = False
    sample_df = None

    for weight_dir, weight_label in weight_labels.items():
        csv_path = root / weight_dir / benchmark / "average.csv"
        if not csv_path.exists():
            print(f"Skip missing file: {csv_path}")
            continue
        df = load_and_prepare(csv_path)
        if sample_df is None:
            sample_df = df

        xs = list(range(len(df)))
        ax.plot(
            xs,
            df["relax_ratio"],
            color=weight_colors[weight_dir],
            marker="o",
            linestyle="-",
            label=weight_labels[weight_dir],
        )
        plotted = True

    if not plotted:
        print(f"No data to plot for {benchmark}")
    else:
        ax.axhline(1.0, color="gray", linewidth=1, linestyle="--", alpha=0.5)
        ax.set_xticks(range(len(sample_df)))
        ax.set_xticklabels(sample_df["filename"], rotation=45, ha="right")
        ax.set_xlabel("Graph Instances (sorted by |V|)")
        ax.set_ylabel("Spira pops / New variant pops")
        ax.set_title(f"Sparse graphs ({benchmark}) - pop ratio by file")
        ax.grid(True, which="both", linestyle=":", alpha=0.4)
        ax.legend(title="Edge weights")
        output_path = root / f"relaxation_ratios_{benchmark}_filenames.png"
        fig.tight_layout()
        fig.savefig(output_path, dpi=200)
        print(f"Saved {output_path}")


def plot_abs_for_benchmark(
    root: Path,
    benchmark: str,
    weight_labels: Dict[str, str],
    weight_colors: Dict[str, str],
) -> None:
    """Plot absolute pops for Spira vs new variant for a given benchmark."""
    fig, ax = plt.subplots(figsize=(8, 5))
    sample_df = None

    series_styles = {
        "avg_s_pops": ("o", "-"),
        "avg_nv_pops": ("s", "--"),
    }

    # Remember which weight types we actually plotted (in case files are missing).
    plotted_weights = set()

    for weight_dir, weight_label in weight_labels.items():
        csv_path = root / weight_dir / benchmark / "average.csv"
        if not csv_path.exists():
            print(f"Skip missing file: {csv_path}")
            continue
        df = load_and_prepare(csv_path)
        if sample_df is None:
            sample_df = df
        plotted_weights.add(weight_dir)

        xs = list(range(len(df)))
        for col, (marker, linestyle) in series_styles.items():
            ax.plot(
                xs,
                df[col],
                color=weight_colors[weight_dir],
                marker=marker,
                linestyle=linestyle,
                label=None,
            )

    if sample_df is not None and not sample_df.empty:
        ax.set_xticks(range(len(sample_df)))
        ax.set_xticklabels(sample_df["filename"], rotation=45, ha="right")

    ax.set_xlabel("Graph Instances (sorted by |V|)")
    ax.set_ylabel("PQ pops (absolute)")
    ax.set_title(f"sparse graphs ({benchmark}) - absolute pops")
    ax.grid(True, which="both", linestyle=":", alpha=0.4)

    # Two-level legend: edge-weight colors + algorithm markers/linestyles.
    weight_handles = [
        Line2D([0], [0], color=weight_colors[w], lw=2, label=weight_labels[w])
        for w in plotted_weights
    ]
    algo_handles = [
        Line2D(
            [0],
            [0],
            color="black",
            marker=marker,
            linestyle=linestyle,
            label="Spira" if col == "avg_s_pops" else "New variant",
            markersize=6,
        )
        for col, (marker, linestyle) in series_styles.items()
    ]
    legend1 = ax.legend(handles=weight_handles, title="Edge weights", loc="upper left")
    legend2 = ax.legend(handles=algo_handles, title="Algorithm", loc="upper right")
    ax.add_artist(legend1)
    ax.add_artist(legend2)

    output_path = root / f"relaxation_abs_{benchmark}.png"
    fig.tight_layout()
    fig.savefig(output_path, dpi=200)
    print(f"Saved {output_path}")


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
        plot_for_benchmark(root, benchmark, weight_labels, weight_colors)
        # plot_abs_for_benchmark(root, benchmark, weight_labels, weight_colors)


if __name__ == "__main__":
    main()
