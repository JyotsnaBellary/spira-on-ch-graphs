#!/usr/bin/env python3
"""Compare CH and CCH SPT runtimes on matching benchmark rows."""

from __future__ import annotations

import argparse
import csv
import os
import re
from pathlib import Path
from statistics import mean, median

os.environ.setdefault("MPLCONFIGDIR", "/tmp/matplotlib")

import matplotlib.pyplot as plt


ROOT = Path(__file__).resolve().parents[1]

THRESHOLDS = [
    ("output_1_4", "25%"),
    ("output_half", "50%"),
    ("output_3_4", "75%"),
]

WEIGHTS = [
    ("original_weights", "Original", "#d00000"),
    ("uniform_weights", "Uniform", "#2a9d8f"),
    ("exponential_weights", "Exponential", "#4361ee"),
]

ALGORITHMS = [
    ("avg_d_time", "Dijkstra", "o"),
    ("avg_s_time", "Spira", "s"),
    ("avg_nv_time", "New Variant", "^"),
]


def osm_sort_key(name: str) -> int:
    match = re.search(r"(\d+)$", name)
    return int(match.group(1)) if match else 10**9


def read_average_map(root: Path, threshold: str, weight: str) -> dict[str, dict[str, str]]:
    path = root / threshold / weight / "spt_benchmark" / "average.csv"
    if not path.exists():
        raise FileNotFoundError(f"Missing average CSV: {path}")

    with path.open(newline="") as file:
        return {row["filename"]: row for row in csv.DictReader(file)}


def collect_ratios(ch_root: Path, cch_root: Path) -> list[dict[str, object]]:
    rows = []
    for threshold, threshold_label in THRESHOLDS:
        for weight, weight_label, _ in WEIGHTS:
            ch_rows = read_average_map(ch_root, threshold, weight)
            cch_rows = read_average_map(cch_root, threshold, weight)
            common_instances = sorted(ch_rows.keys() & cch_rows.keys(), key=osm_sort_key)

            for instance in common_instances:
                for column, algorithm_label, _ in ALGORITHMS:
                    ch_time = float(ch_rows[instance][column])
                    cch_time = float(cch_rows[instance][column])
                    rows.append(
                        {
                            "threshold": threshold,
                            "threshold_label": threshold_label,
                            "weight": weight,
                            "weight_label": weight_label,
                            "instance": instance,
                            "algorithm": algorithm_label,
                            "ch_time_us": ch_time,
                            "cch_time_us": cch_time,
                            "cch_to_ch_ratio": cch_time / ch_time if ch_time > 0 else 0.0,
                        }
                    )
    return rows


def write_summary(rows: list[dict[str, object]], output_dir: Path) -> None:
    detail_path = output_dir / "ch_vs_cch_runtime_ratios.csv"
    with detail_path.open("w", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)

    summary_path = output_dir / "ch_vs_cch_runtime_summary.csv"
    groups: dict[tuple[str, str, str], list[float]] = {}
    for row in rows:
        key = (str(row["threshold_label"]), str(row["weight_label"]), str(row["algorithm"]))
        groups.setdefault(key, []).append(float(row["cch_to_ch_ratio"]))

    with summary_path.open("w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(["threshold", "weight", "algorithm", "matched_instances", "mean_cch_to_ch", "median_cch_to_ch"])
        for key in sorted(groups):
            values = groups[key]
            writer.writerow([*key, len(values), f"{mean(values):.8f}", f"{median(values):.8f}"])


def plot_ratio_lines(rows: list[dict[str, object]], output_dir: Path) -> None:
    for threshold, threshold_label in THRESHOLDS:
        fig, axes = plt.subplots(1, len(WEIGHTS), figsize=(16, 4.8), sharey=True)

        for ax, (_, weight_label, color) in zip(axes, WEIGHTS):
            subset = [
                row
                for row in rows
                if row["threshold_label"] == threshold_label and row["weight_label"] == weight_label
            ]
            instances = sorted({str(row["instance"]) for row in subset}, key=osm_sort_key)

            for _, algorithm_label, marker in ALGORITHMS:
                values = [
                    float(row["cch_to_ch_ratio"])
                    for instance in instances
                    for row in subset
                    if row["instance"] == instance and row["algorithm"] == algorithm_label
                ]
                ax.plot(instances, values, marker=marker, linewidth=1.4, markersize=4, label=algorithm_label)

            ax.axhline(1.0, color="0.45", linestyle=":", linewidth=1)
            ax.set_title(weight_label)
            ax.set_xlabel("Graph instance")
            ax.tick_params(axis="x", rotation=45)
            ax.grid(True, linestyle=":", alpha=0.45)

        axes[0].set_ylabel("CCH runtime / CH runtime")
        axes[-1].legend(framealpha=0.9)
        fig.suptitle(f"CCH vs CH runtime ratio, threshold {threshold_label}")
        fig.tight_layout()
        fig.savefig(output_dir / f"ch_vs_cch_runtime_ratio_{threshold}.png", dpi=300)
        plt.close(fig)


def plot_summary_bars(rows: list[dict[str, object]], output_dir: Path) -> None:
    for _, algorithm_label, _ in ALGORITHMS:
        labels = [label for _, label in THRESHOLDS]
        x = range(len(labels))
        width = 0.24

        fig, ax = plt.subplots(figsize=(9.5, 4.8))
        for offset, (_, weight_label, color) in zip([-width, 0, width], WEIGHTS):
            medians = []
            for _, threshold_label in THRESHOLDS:
                values = [
                    float(row["cch_to_ch_ratio"])
                    for row in rows
                    if row["algorithm"] == algorithm_label
                    and row["threshold_label"] == threshold_label
                    and row["weight_label"] == weight_label
                ]
                medians.append(median(values) if values else 0.0)

            ax.bar([value + offset for value in x], medians, width=width, color=color, label=weight_label)

        ax.axhline(1.0, color="0.45", linestyle=":", linewidth=1)
        ax.set_title(f"Median CCH/CH runtime ratio: {algorithm_label}")
        ax.set_xlabel("Threshold")
        ax.set_ylabel("Median CCH runtime / CH runtime")
        ax.set_xticks(list(x), labels)
        ax.grid(axis="y", linestyle=":", alpha=0.45)
        ax.legend(framealpha=0.9)
        fig.tight_layout()
        filename = algorithm_label.lower().replace(" ", "_")
        fig.savefig(output_dir / f"ch_vs_cch_median_ratio_{filename}.png", dpi=300)
        plt.close(fig)


def plot_combined_runtime_grid(rows: list[dict[str, object]], output_dir: Path) -> None:
    fig, axes = plt.subplots(
        len(THRESHOLDS),
        len(WEIGHTS),
        figsize=(15.5, 10.5),
        sharey=True,
        constrained_layout=True,
    )

    algorithm_labels = [label for _, label, _ in ALGORITHMS]
    x_positions = list(range(len(ALGORITHMS)))
    bar_width = 0.36

    for row_index, (_, threshold_label) in enumerate(THRESHOLDS):
        for col_index, (_, weight_label, _) in enumerate(WEIGHTS):
            ax = axes[row_index][col_index]
            ch_medians = []
            cch_medians = []

            for _, algorithm_label, _ in ALGORITHMS:
                subset = [
                    row
                    for row in rows
                    if row["threshold_label"] == threshold_label
                    and row["weight_label"] == weight_label
                    and row["algorithm"] == algorithm_label
                ]
                ch_medians.append(median(float(row["ch_time_us"]) for row in subset))
                cch_medians.append(median(float(row["cch_time_us"]) for row in subset))

            ax.bar(
                [position - bar_width / 2 for position in x_positions],
                ch_medians,
                width=bar_width,
                color="#8ecae6",
                edgecolor="#2f4858",
                label="CH",
            )
            ax.bar(
                [position + bar_width / 2 for position in x_positions],
                cch_medians,
                width=bar_width,
                color="#ffb703",
                edgecolor="#6c4700",
                label="CCH",
            )

            for position, ch_value, cch_value in zip(x_positions, ch_medians, cch_medians):
                ratio = cch_value / ch_value if ch_value > 0 else 0.0
                ax.text(
                    position,
                    max(ch_value, cch_value) * 1.08,
                    f"{ratio:.2f}x",
                    ha="center",
                    va="bottom",
                    fontsize=8,
                )

            ax.set_title(f"{threshold_label}, {weight_label}")
            ax.set_xticks(x_positions, algorithm_labels, rotation=20, ha="right")
            ax.set_yscale("log")
            ax.grid(axis="y", which="both", linestyle=":", alpha=0.4)

            if col_index == 0:
                ax.set_ylabel("Median runtime per query (microseconds)")

            if row_index == 0 and col_index == len(WEIGHTS) - 1:
                ax.legend(framealpha=0.9)

    fig.suptitle("CH vs CCH median runtime by threshold, weight, and algorithm")
    fig.savefig(output_dir / "ch_vs_cch_runtime_grid.png", dpi=300)
    plt.close(fig)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ch-root", type=Path, default=ROOT / "output_ch" / "sparse_networks")
    parser.add_argument("--cch-root", type=Path, default=ROOT / "output_cch" / "sparse_networks")
    parser.add_argument("--output-dir", type=Path, default=ROOT / "plots" / "CH_vs_CCH" / "runtime")
    args = parser.parse_args()

    args.output_dir.mkdir(parents=True, exist_ok=True)
    rows = collect_ratios(args.ch_root, args.cch_root)
    write_summary(rows, args.output_dir)
    plot_ratio_lines(rows, args.output_dir)
    plot_summary_bars(rows, args.output_dir)
    plot_combined_runtime_grid(rows, args.output_dir)
    print(f"Wrote CH vs CCH runtime comparison to {args.output_dir}")


if __name__ == "__main__":
    main()
