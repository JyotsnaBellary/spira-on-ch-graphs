#!/usr/bin/env python3
"""Plot SPT runtime comparisons across thresholds and weight distributions."""

from __future__ import annotations

import argparse
import csv
import math
import os
import re
from pathlib import Path

os.environ.setdefault("MPLCONFIGDIR", "/tmp/matplotlib")

import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm


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
    ("avg_d_time", "Dijkstra", "o", "-"),
    ("avg_s_time", "Spira", "s", "--"),
    ("avg_nv_time", "New Variant", "^", "-."),
]


def osm_sort_key(name: str) -> int:
    match = re.search(r"(\d+)$", name)
    return int(match.group(1)) if match else 10**9


def geometric_mean(values: list[float]) -> float:
    positive = [value for value in values if value > 0]
    if not positive:
        return 0.0
    return math.exp(sum(math.log(value) for value in positive) / len(positive))


def read_average_rows(input_root: Path, threshold: str, weight: str) -> list[dict[str, str]]:
    average_csv = input_root / threshold / weight / "spt_benchmark" / "average.csv"
    if not average_csv.exists():
        raise FileNotFoundError(f"Missing average CSV: {average_csv}")

    rows_by_filename: dict[str, dict[str, str]] = {}
    with average_csv.open(newline="") as file:
        for row in csv.DictReader(file):
            rows_by_filename[row["filename"]] = row

    return [rows_by_filename[name] for name in sorted(rows_by_filename, key=osm_sort_key)]


def write_summary(input_root: Path, output_dir: Path) -> None:
    output_path = output_dir / "runtime_summary.csv"
    with output_path.open("w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(
            [
                "threshold",
                "weight",
                "algorithm",
                "instance_count",
                "geomean_runtime_us",
                "mean_runtime_us",
                "median_runtime_us",
            ]
        )

        for threshold, threshold_label in THRESHOLDS:
            for weight, weight_label, _ in WEIGHTS:
                rows = read_average_rows(input_root, threshold, weight)
                for column, algorithm_label, _, _ in ALGORITHMS:
                    values = sorted(float(row[column]) for row in rows)
                    median_value = values[len(values) // 2] if len(values) % 2 else (values[len(values) // 2 - 1] + values[len(values) // 2]) / 2.0
                    writer.writerow(
                        [
                            threshold_label,
                            weight_label,
                            algorithm_label,
                            len(values),
                            f"{geometric_mean(values):.8f}",
                            f"{sum(values) / len(values):.8f}",
                            f"{median_value:.8f}",
                        ]
                    )


def plot_runtime_ratios_by_threshold(input_root: Path, output_dir: Path, title_prefix: str) -> None:
    for threshold, threshold_label in THRESHOLDS:
        fig, ax = plt.subplots(figsize=(11.2, 5.4))

        for weight, weight_label, color in WEIGHTS:
            rows = read_average_rows(input_root, threshold, weight)
            x = [row["filename"] for row in rows]
            dijkstra = [float(row["avg_d_time"]) for row in rows]
            spira_ratio = [float(row["avg_s_time"]) / base if base > 0 else 0.0 for row, base in zip(rows, dijkstra)]
            new_variant_ratio = [float(row["avg_nv_time"]) / base if base > 0 else 0.0 for row, base in zip(rows, dijkstra)]

            ax.plot(x, spira_ratio, marker="o", linestyle="-", color=color, label=f"{weight_label}: Spira/Dijkstra")
            ax.plot(x, new_variant_ratio, marker="s", linestyle="--", color=color, label=f"{weight_label}: New Variant/Dijkstra")

        ax.axhline(1.0, color="0.55", linestyle=":", linewidth=1)
        ax.set_title(f"{title_prefix}: runtime ratios, threshold {threshold_label}")
        ax.set_xlabel("Graph instance, sorted by full graph size")
        ax.set_ylabel("Runtime ratio")
        ax.grid(True, linestyle=":", alpha=0.45)
        ax.tick_params(axis="x", rotation=45)
        ax.legend(ncol=2, fontsize=8.5, framealpha=0.9)
        fig.tight_layout()
        fig.savefig(output_dir / f"runtime_ratios_{threshold}.png", dpi=300)
        plt.close(fig)


def plot_absolute_runtime_by_threshold_weight(input_root: Path, output_dir: Path, title_prefix: str) -> None:
    for threshold, threshold_label in THRESHOLDS:
        for weight, weight_label, color in WEIGHTS:
            rows = read_average_rows(input_root, threshold, weight)
            x = [row["filename"] for row in rows]

            fig, ax = plt.subplots(figsize=(10.8, 5.2))
            for column, algorithm_label, marker, linestyle in ALGORITHMS:
                ax.plot(
                    x,
                    [float(row[column]) for row in rows],
                    marker=marker,
                    linestyle=linestyle,
                    linewidth=1.4,
                    markersize=4,
                    label=algorithm_label,
                )

            ax.set_title(f"{title_prefix}: runtime, threshold {threshold_label}, {weight_label} weights")
            ax.set_xlabel("Graph instance, sorted by full graph size")
            ax.set_ylabel("Average runtime per query (microseconds)")
            ax.set_yscale("log")
            ax.grid(True, which="both", linestyle=":", alpha=0.45)
            ax.tick_params(axis="x", rotation=45)
            ax.legend(framealpha=0.9)
            fig.tight_layout()
            fig.savefig(output_dir / f"runtime_absolute_{threshold}_{weight}.png", dpi=300)
            plt.close(fig)


def runtime_matrix(input_root: Path, weight: str) -> list[list[float]]:
    matrix = []
    for column, _, _, _ in ALGORITHMS:
        algorithm_row = []
        for threshold, _ in THRESHOLDS:
            rows = read_average_rows(input_root, threshold, weight)
            algorithm_row.append(geometric_mean([float(row[column]) for row in rows]))
        matrix.append(algorithm_row)
    return matrix


def plot_runtime_heatmaps(input_root: Path, output_dir: Path, title_prefix: str) -> None:
    matrices = {weight_label: runtime_matrix(input_root, weight) for weight, weight_label, _ in WEIGHTS}
    fastest = min(value for matrix in matrices.values() for row in matrix for value in row if value > 0)
    relative = {
        weight_label: [[value / fastest if fastest > 0 else 0.0 for value in row] for row in matrix]
        for weight_label, matrix in matrices.items()
    }
    max_relative = max(value for matrix in relative.values() for row in matrix for value in row)

    fig, axes = plt.subplots(1, len(WEIGHTS), figsize=(15, 4.8), constrained_layout=True)
    norm = LogNorm(vmin=1.0, vmax=max_relative)
    last_image = None

    for ax, (_, weight_label, _) in zip(axes, WEIGHTS):
        matrix = matrices[weight_label]
        rel_matrix = relative[weight_label]
        last_image = ax.imshow(rel_matrix, cmap="coolwarm", norm=norm)
        ax.set_title(weight_label)
        ax.set_xticks(range(len(THRESHOLDS)), [label for _, label in THRESHOLDS])
        ax.set_yticks(range(len(ALGORITHMS)), [label for _, label, _, _ in ALGORITHMS])
        ax.set_xlabel("Threshold")

        for algorithm_index in range(len(ALGORITHMS)):
            for threshold_index in range(len(THRESHOLDS)):
                runtime = matrix[algorithm_index][threshold_index]
                slowdown = rel_matrix[algorithm_index][threshold_index]
                ax.text(
                    threshold_index,
                    algorithm_index,
                    f"{runtime:.1f}\n{slowdown:.2f}x",
                    ha="center",
                    va="center",
                    color="black" if slowdown < 8 else "white",
                    fontsize=8.5,
                )

    colorbar = fig.colorbar(last_image, ax=axes, shrink=0.9)
    colorbar.set_label("Relative slowdown vs fastest cell")
    fig.suptitle(f"{title_prefix}: geometric mean runtime by threshold and weight")
    fig.savefig(output_dir / "runtime_threshold_weight_heatmaps.png", dpi=300)
    plt.close(fig)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input-root", type=Path, default=ROOT / "output_ch" / "sparse_networks")
    parser.add_argument("--output-dir", type=Path, default=ROOT / "plots" / "runtime_ch")
    parser.add_argument("--title-prefix", default="CH sparse graphs")
    args = parser.parse_args()

    args.output_dir.mkdir(parents=True, exist_ok=True)
    write_summary(args.input_root, args.output_dir)
    plot_runtime_ratios_by_threshold(args.input_root, args.output_dir, args.title_prefix)
    plot_absolute_runtime_by_threshold_weight(args.input_root, args.output_dir, args.title_prefix)
    plot_runtime_heatmaps(args.input_root, args.output_dir, args.title_prefix)

    print(f"Wrote runtime plots and summary to {args.output_dir}")


if __name__ == "__main__":
    main()
