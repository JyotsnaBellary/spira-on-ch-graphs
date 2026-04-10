#!/usr/bin/env python3
"""
Bar plots: pertinent edge counts (out vs in) per graph instance for dense graphs.
Reads average.csv under each weight directory and plots avg_out vs avg_in.
"""

from pathlib import Path
from typing import Dict

import matplotlib.pyplot as plt
import pandas as pd


def load_avg(csv_path: Path) -> pd.DataFrame:
    df = pd.read_csv(csv_path)
    required = ["filename", "m", "avg_out", "avg_in"]
    missing = [c for c in required if c not in df.columns]
    if missing:
        raise ValueError(f"Missing columns {missing} in {csv_path}")
    df = df.sort_values("m")
    # Normalize by edge count so small graphs remain visible.
    df["out_per_m"] = df["avg_out"] / df["m"] * 100.0
    df["in_per_m"] = df["avg_in"] / df["m"] * 100.0
    return df


def plot_counts(
    root: Path,
    benchmark: str,
    weight_labels: Dict[str, str],
    weight_colors: Dict[str, str],
) -> None:
    for scale, suffix in (("log", "log"), ("linear", "linear")):
        plt.figure(figsize=(10, 5))
        plotted = False
        base_order = None
        all_vals = []
        n_weights = len(weight_labels)
        inner_width = 0.18
        start_offset = -inner_width * (n_weights - 1)

        for idx, (wdir, wlabel) in enumerate(weight_labels.items()):
            csv_path = root / wdir / benchmark / "average.csv"
            if not csv_path.exists():
                print(f"Skip missing file: {csv_path}")
                continue
            df = load_avg(csv_path)
            if base_order is None:
                base_order = list(df["filename"])
            df = df.set_index("filename").reindex(base_order)
            x = range(len(df))
            offset = start_offset + idx * inner_width * 2
            if scale == "log":
                plt.bar([i + offset - inner_width / 2 for i in x], df["out_per_m"], width=inner_width, color=weight_colors[wdir], alpha=0.8, label=f"{wlabel} out")
                plt.bar([i + offset + inner_width / 2 for i in x], df["in_per_m"], width=inner_width, color=weight_colors[wdir], alpha=0.5, label=f"{wlabel} in")
            else:
                plt.plot(
                    x,
                    df["out_per_m"],
                    marker="o",
                    linestyle="-",
                    color=weight_colors[wdir],
                    label=f"{wlabel} out",
                )
                plt.plot(
                    x,
                    df["in_per_m"],
                    marker="s",
                    linestyle="--",
                    color=weight_colors[wdir],
                    label=f"{wlabel} in",
                )
            all_vals.extend(df["out_per_m"].tolist())
            all_vals.extend(df["in_per_m"].tolist())
            plotted = True

        if not plotted or base_order is None:
            print(f"No data for {benchmark}")
            plt.close()
            continue

        plt.xticks(range(len(base_order)), base_order, rotation=45, ha="right")
        plt.ylabel("Pertinent edges (% of total edges)")
        plt.xlabel("Graph Instances (sorted by |E|)")
        plt.title(f"Dense — Pertinent edges: out vs in — {benchmark} ({scale})")
        plt.legend(ncol=2, title="Edge weights × direction")
        plt.grid(axis="y", linestyle=":", alpha=0.4)

        if scale == "log":
            positives = [v for v in all_vals if v > 0]
            if positives:
                ymin = max(min(positives) * 0.5, 1e-6)
                plt.yscale("log")
                plt.ylim(bottom=ymin)

        plt.tight_layout()
        out = root / f"pertinent_counts_{benchmark}_{suffix}.png"
        plt.savefig(out, dpi=200)
        print(f"Saved {out}")
        plt.close()


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
        plot_counts(root, benchmark, weight_labels, weight_colors)


if __name__ == "__main__":
    main()
