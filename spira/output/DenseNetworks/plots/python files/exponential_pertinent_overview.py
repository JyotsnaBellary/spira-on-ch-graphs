#!/usr/bin/env python3
"""
Single-view plot for exponential-weight dense graphs:
Pertinent edges (out vs in) per graph instance for both benchmarks.
"""

from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd


ROOT = Path(__file__).parent


def load_avg(csv_path: Path) -> pd.DataFrame:
    df = pd.read_csv(csv_path)
    required = ["filename", "m", "avg_out", "avg_in"]
    missing = [c for c in required if c not in df.columns]
    if missing:
        raise ValueError(f"Missing columns {missing} in {csv_path}")
    df = df.sort_values("m")
    df["out_per_m"] = df["avg_out"] / df["m"] * 100.0
    df["in_per_m"] = df["avg_in"] / df["m"] * 100.0
    return df


def plot_for_benchmark(ax, df: pd.DataFrame, benchmark: str) -> None:
    x = range(len(df))
    width = 0.4
    ax.bar([i - width / 2 for i in x], df["out_per_m"], width=width, color="#1f77b4", alpha=0.9, label="Exponential out")
    ax.bar([i + width / 2 for i in x], df["in_per_m"], width=width, color="#84b6ff", alpha=0.8, label="Exponential in")
    ax.set_xticks(list(x))
    ax.set_xticklabels(df["filename"], rotation=45, ha="right")
    ax.set_ylabel("Pertinent edges (% of total edges)")
    ax.set_title(benchmark.replace("_", " "))
    ax.grid(axis="y", linestyle=":", alpha=0.4)
    positives = [v for v in pd.concat([df["out_per_m"], df["in_per_m"]]) if v > 0]
    if positives:
        ymin = max(min(positives) * 0.5, 1e-6)
        ax.set_yscale("log")
        ax.set_ylim(bottom=ymin)


def main() -> None:
    fig, axes = plt.subplots(2, 1, figsize=(12, 8), sharex=False)
    plotted = False
    for ax, benchmark in zip(axes, ("spt_benchmark", "src_dst_benchmark")):
        csv_path = ROOT / "exponential_weights" / benchmark / "average.csv"
        if not csv_path.exists():
            ax.set_visible(False)
            continue
        df = load_avg(csv_path)
        plot_for_benchmark(ax, df, benchmark)
        plotted = True

    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, ncol=2, title="Edge weights × direction", loc="upper center", bbox_to_anchor=(0.5, 1.02))
    fig.suptitle("Dense (exponential) — Pertinent edges: out vs in")
    fig.tight_layout(rect=[0, 0, 1, 0.96])

    out = ROOT / "pertinent_counts_exponential_overview.png"
    fig.savefig(out, dpi=200)
    print(f"Saved {out}")


if __name__ == "__main__":
    main()
