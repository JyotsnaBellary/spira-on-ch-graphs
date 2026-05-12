#!/usr/bin/env python3
"""Generate sparse SPT benchmark plots for threshold and weight experiments."""

import csv
import os
import re
from pathlib import Path

os.environ.setdefault("MPLCONFIGDIR", "/tmp/matplotlib")

import matplotlib.pyplot as plt


ROOT = Path(__file__).resolve().parents[1]
SPARSE_OUTPUT_DIR = ROOT / "output" / "sparse_networks"
PLOTS_DIR = ROOT / "plots"

THRESHOLDS = [
    ("output_1_4", "threshold 1/4", "original_1_4"),
    ("output_half", "threshold 1/2", "original_halb"),
    ("output_3_4", "threshold 3/4", "original_3_4"),
]

WEIGHTS = [
    ("Exponential", "exponential_weights", "#1f77b4"),
    ("Uniform", "uniform_weights", "#2ca02c"),
    ("Original", "original_weights", "#d62728"),
]


def osm_key(name: str) -> int:
    match = re.search(r"(\d+)$", name)
    return int(match.group(1)) if match else 10**9


def read_average_rows(threshold_dir: Path, weight_dir: str) -> list[dict[str, str]]:
    average_csv = threshold_dir / weight_dir / "spt_benchmark" / "average.csv"
    rows_by_filename: dict[str, dict[str, str]] = {}

    with average_csv.open(newline="") as file:
        for row in csv.DictReader(file):
            # Keep the last row if average.csv accidentally contains duplicates.
            rows_by_filename[row["filename"]] = row

    return [rows_by_filename[name] for name in sorted(rows_by_filename, key=osm_key)]


def plot_runtime_ratios(threshold_dir: Path, threshold_label: str, output_dir: Path) -> None:
    fig, ax = plt.subplots(figsize=(10.5, 5.2))

    for label, weight_dir, color in WEIGHTS:
        rows = read_average_rows(threshold_dir, weight_dir)
        x = [row["filename"] for row in rows]
        dijkstra_times = [float(row["avg_d_time"]) for row in rows]
        spira_ratio = [float(row["avg_s_time"]) / d for row, d in zip(rows, dijkstra_times)]
        new_variant_ratio = [float(row["avg_nv_time"]) / d for row, d in zip(rows, dijkstra_times)]

        ax.plot(x, spira_ratio, marker="o", linestyle="-", color=color, label=f"{label}: spira/dijkstra")
        ax.plot(x, new_variant_ratio, marker="s", linestyle="--", color=color, label=f"{label}: new_variant/dijkstra")

    ax.axhline(1.0, color="0.65", linestyle="--", linewidth=1)
    ax.set_title(f"Sparse graphs, {threshold_label} (spt_benchmark)")
    ax.set_xlabel("Graph instances (sorted by |V|)")
    ax.set_ylabel("Relative runtime compared to Dijkstra")
    ax.grid(True, linestyle=":", alpha=0.5)
    ax.tick_params(axis="x", rotation=45)
    ax.legend(ncol=2, fontsize=9, framealpha=0.9)
    fig.tight_layout()
    fig.savefig(output_dir / "runtime_ratios_by_weight.png", dpi=300)
    plt.close(fig)


def plot_pertinent_out_vs_in(threshold_dir: Path, threshold_label: str, output_dir: Path) -> None:
    fig, ax = plt.subplots(figsize=(10.5, 4.8))

    for label, weight_dir, color in WEIGHTS:
        rows = read_average_rows(threshold_dir, weight_dir)
        x = [row["filename"] for row in rows]
        out_pct = []
        in_pct = []

        for row in rows:
            out_edges = float(row["avg_out"])
            in_edges = float(row["avg_in"])
            pertinent_edges = float(row["avg_pert"])
            out_pct.append(100.0 * out_edges / pertinent_edges if pertinent_edges > 0 else 0.0)
            in_pct.append(100.0 * in_edges / pertinent_edges if pertinent_edges > 0 else 0.0)

        ax.plot(x, out_pct, marker="o", linestyle="-", color=color, label=f"{label} out")
        ax.plot(x, in_pct, marker="s", linestyle="--", color=color, label=f"{label} in")

    ax.set_title(f"Sparse graphs, {threshold_label}: pertinent edges out vs in")
    ax.set_xlabel("Graph instances (sorted by |V|)")
    ax.set_ylabel("Pertinent edges (% of pertinent edges)")
    ax.grid(True, linestyle=":", alpha=0.5)
    ax.tick_params(axis="x", rotation=45)
    ax.legend(title="Edge weights x direction", ncol=2, fontsize=9, framealpha=0.9)
    fig.tight_layout()
    fig.savefig(output_dir / "pertinent_edges_out_vs_in_by_weight.png", dpi=300)
    plt.close(fig)


def plot_pertinent_to_query_edges(threshold_dir: Path, threshold_label: str, output_dir: Path) -> None:
    fig, ax = plt.subplots(figsize=(10.5, 4.8))

    for label, weight_dir, color in WEIGHTS:
        rows = read_average_rows(threshold_dir, weight_dir)
        x = [row["filename"] for row in rows]
        y = [100.0 * float(row["ratio_pert_m"]) for row in rows]
        ax.plot(x, y, marker="o", linestyle="-", color=color, label=label)

    ax.set_title(f"Sparse graphs, {threshold_label}: pertinent edges / query edges")
    ax.set_xlabel("Graph instances (sorted by |V|)")
    ax.set_ylabel("Pertinent edges (% of reachable query graph edges)")
    ax.grid(True, linestyle=":", alpha=0.5)
    ax.tick_params(axis="x", rotation=45)
    ax.legend(title="Edge weights", fontsize=9, framealpha=0.9)
    fig.tight_layout()
    fig.savefig(output_dir / "pertinent_edges_to_query_edges_by_weight.png", dpi=300)
    plt.close(fig)


def plot_priority_queue_pops(threshold_dir: Path, threshold_label: str, output_dir: Path) -> None:
    fig, ax = plt.subplots(figsize=(10.5, 5.0))

    for label, weight_dir, color in WEIGHTS:
        rows = read_average_rows(threshold_dir, weight_dir)
        x = [row["filename"] for row in rows]
        spira_pops = [float(row["avg_s_pops"]) for row in rows]
        new_variant_pops = [float(row["avg_nv_pops"]) for row in rows]

        ax.plot(x, spira_pops, marker="o", linestyle="-", color=color, label=f"{label}: Spira P pops")
        ax.plot(x, new_variant_pops, marker="s", linestyle="--", color=color, label=f"{label}: New Variant P pops")

    ax.set_title(f"Sparse graphs, {threshold_label}: priority queue pops")
    ax.set_xlabel("Graph instances (sorted by |V|)")
    ax.set_ylabel("Average priority queue pops")
    ax.grid(True, linestyle=":", alpha=0.5)
    ax.tick_params(axis="x", rotation=45)
    ax.legend(ncol=2, fontsize=9, framealpha=0.9)
    fig.tight_layout()
    fig.savefig(output_dir / "priority_queue_pops_by_weight.png", dpi=300)
    plt.close(fig)


def main() -> None:
    for threshold_folder, threshold_label, plot_folder in THRESHOLDS:
        threshold_dir = SPARSE_OUTPUT_DIR / threshold_folder
        output_dir = PLOTS_DIR / plot_folder
        output_dir.mkdir(parents=True, exist_ok=True)

        plot_runtime_ratios(threshold_dir, threshold_label, output_dir)
        plot_pertinent_out_vs_in(threshold_dir, threshold_label, output_dir)
        plot_pertinent_to_query_edges(threshold_dir, threshold_label, output_dir)
        plot_priority_queue_pops(threshold_dir, threshold_label, output_dir)

        print(f"Wrote plots for {threshold_label} to {output_dir.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
