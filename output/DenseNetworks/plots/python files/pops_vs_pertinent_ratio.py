#!/usr/bin/env python3
"""
Dense graphs: ratio of NV pops to pertinent edges per input, all weight families.
Outputs a single image with subplots for SPT and SRC-DST benchmarks.
"""

from pathlib import Path
from typing import Dict, List, Tuple

import matplotlib.pyplot as plt
import pandas as pd


ROOT = Path(__file__).parent
BENCHMARKS: Tuple[str, ...] = ("spt_benchmark", "src_dst_benchmark")
WEIGHT_LABELS: Dict[str, str] = {
    "exponential_weights": "Exponential",
    "original_weights": "Geometric",
    "random_weights": "Random",
}
WEIGHT_COLORS: Dict[str, str] = {
    "exponential_weights": "#1f77b4",
    "original_weights": "#2ca02c",
    "random_weights": "#d62728",
}


def load_avg(csv_path: Path) -> pd.DataFrame:
    df = pd.read_csv(csv_path)
    required = ["filename", "m", "avg_nv_pops", "avg_pert"]
    missing = [c for c in required if c not in df.columns]
    if missing:
        raise ValueError(f"Missing columns {missing} in {csv_path}")
    df = df.sort_values("m")
    # Ratio of NV pops to pertinent edge count
    df["ratio_nv_to_pert"] = df["avg_nv_pops"] / df["avg_pert"].replace(0, pd.NA)
    df = df.dropna(subset=["ratio_nv_to_pert"])
    return df


def plot_sub(ax, benchmark: str) -> bool:
    base_order: List[str] | None = None
    plotted = False

    for wdir, wlabel in WEIGHT_LABELS.items():
        csv_path = ROOT / wdir / benchmark / "average.csv"
        if not csv_path.exists():
            print(f"Skip missing file: {csv_path}")
            continue
        df = load_avg(csv_path)
        if base_order is None:
            base_order = list(df["filename"])
        df = df.set_index("filename").reindex(base_order)
        x = list(range(len(df)))
        ax.plot(
            x,
            df["ratio_nv_to_pert"],
            marker="o",
            linestyle="-",
            color=WEIGHT_COLORS[wdir],
            alpha=0.9,
            label=wlabel,
        )
        plotted = True

    if not plotted or base_order is None:
        return False

    ax.set_xticks(range(len(base_order)))
    ax.set_xticklabels(base_order, rotation=45, ha="right")
    ax.set_ylabel("NV pops / pertinent edges")
    ax.set_title(benchmark.replace("_", " "))
    ax.grid(axis="y", linestyle=":", alpha=0.4)
    # Use plain ticks (no sci/10^) and set a sensible lower bound.
    ax.ticklabel_format(style="plain", axis="y")
    ax.set_ylim(bottom=0.8)
    return True


def main() -> None:
    for bench in BENCHMARKS:
        fig, ax = plt.subplots(figsize=(12, 5))
        if not plot_sub(ax, bench):
            plt.close(fig)
            continue
        handles, labels = ax.get_legend_handles_labels()
        fig.legend(handles, labels, title="Edge weights", ncol=len(WEIGHT_LABELS), loc="upper center", bbox_to_anchor=(0.5, 1.05))
        fig.suptitle(f"Dense — NV pops to pertinent edge ratio — {bench}")
        fig.tight_layout(rect=[0, 0, 1, 0.95])
        out = ROOT / f"pops_vs_pertinent_ratio_dense_{bench}.png"
        fig.savefig(out, dpi=200)
        print(f"Saved {out}")


if __name__ == "__main__":
    main()
