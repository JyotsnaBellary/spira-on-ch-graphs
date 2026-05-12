#!/usr/bin/env python3
"""Plot runtime heatmaps for threshold and weight-distribution experiments."""

import csv
import math
import os
from pathlib import Path

os.environ.setdefault("MPLCONFIGDIR", "/tmp/matplotlib")

import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm


ROOT = Path(__file__).resolve().parent

THRESHOLDS = [
    ("25%", "output_fourth"),
    ("50%", "output_halb"),
    ("75%", "output_3_fourth"),
]

WEIGHT_DISTRIBUTIONS = [
    ("Original weights", "original_weights"),
    ("Exponential weights", "exponential_weights"),
]

ALGORITHMS = [
    ("Dijkstra", "avg_d_time"),
    ("Spira", "avg_s_time"),
    ("New Variant", "avg_nv_time"),
]


def geometric_mean(values):
    positive_values = [value for value in values if value > 0]
    if not positive_values:
        raise ValueError("Cannot compute geometric mean without positive values.")
    return math.exp(sum(math.log(value) for value in positive_values) / len(positive_values))


def read_average_rows(threshold_dir, weight_dir):
    path = (
        ROOT
        / threshold_dir
        / "sparse_networks"
        / weight_dir
        / "spt_benchmark"
        / "average.csv"
    )

    rows_by_filename = {}
    with path.open(newline="") as file:
        for row in csv.DictReader(file):
            # Keeps one row per graph if a file accidentally contains duplicates.
            rows_by_filename.setdefault(row["filename"], row)

    return list(rows_by_filename.values())


def runtime_matrix(weight_dir):
    matrix = []

    for algorithm_name, column in ALGORITHMS:
        row_values = []
        for threshold_label, threshold_dir in THRESHOLDS:
            rows = read_average_rows(threshold_dir, weight_dir)
            runtimes = [float(row[column]) for row in rows]
            row_values.append(geometric_mean(runtimes))
        matrix.append(row_values)

    return matrix


def relative_slowdown_matrix(matrix):
    fastest = min(value for row in matrix for value in row)
    return [[value / fastest for value in row] for row in matrix]


def write_summary_csv(output_path, matrices, relative_matrices):
    with output_path.open("w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(
            [
                "weight_distribution",
                "algorithm",
                "threshold",
                "geometric_mean_runtime",
                "relative_slowdown",
            ]
        )

        for weight_label, matrix in matrices.items():
            relative_matrix = relative_matrices[weight_label]
            for algorithm_index, (algorithm_name, _) in enumerate(ALGORITHMS):
                for threshold_index, (threshold_label, _) in enumerate(THRESHOLDS):
                    writer.writerow(
                        [
                            weight_label,
                            algorithm_name,
                            threshold_label,
                            f"{matrix[algorithm_index][threshold_index]:.6f}",
                            f"{relative_matrix[algorithm_index][threshold_index]:.6f}",
                        ]
                    )


def plot_heatmaps(matrices, relative_matrices, output_path):
    max_slowdown = max(
        value
        for matrix in relative_matrices.values()
        for row in matrix
        for value in row
    )

    figure, axes = plt.subplots(1, 2, figsize=(12, 4.8), constrained_layout=True)
    norm = LogNorm(vmin=1.0, vmax=max_slowdown)

    last_image = None
    for axis, (weight_label, _) in zip(axes, WEIGHT_DISTRIBUTIONS):
        matrix = matrices[weight_label]
        relative_matrix = relative_matrices[weight_label]

        last_image = axis.imshow(relative_matrix, cmap="coolwarm", norm=norm)
        axis.set_title(weight_label)
        axis.set_xticks(range(len(THRESHOLDS)), [label for label, _ in THRESHOLDS])
        axis.set_yticks(range(len(ALGORITHMS)), [label for label, _ in ALGORITHMS])
        axis.set_xlabel("Settled-node threshold")

        for algorithm_index in range(len(ALGORITHMS)):
            for threshold_index in range(len(THRESHOLDS)):
                runtime = matrix[algorithm_index][threshold_index]
                slowdown = relative_matrix[algorithm_index][threshold_index]
                axis.text(
                    threshold_index,
                    algorithm_index,
                    f"{runtime:.1f}\n{slowdown:.2f}x",
                    ha="center",
                    va="center",
                    color="black" if slowdown < 8 else "white",
                    fontsize=9,
                )

    colorbar = figure.colorbar(last_image, ax=axes, shrink=0.9)
    colorbar.set_label("Relative slowdown, lower is faster")
    figure.suptitle("Runtime comparison by algorithm and threshold")
    figure.savefig(output_path, dpi=300)


def main():
    output_dir = ROOT / "plots"
    output_dir.mkdir(exist_ok=True)

    matrices = {}
    relative_matrices = {}

    for weight_label, weight_dir in WEIGHT_DISTRIBUTIONS:
        matrix = runtime_matrix(weight_dir)
        matrices[weight_label] = matrix
        relative_matrices[weight_label] = relative_slowdown_matrix(matrix)

    write_summary_csv(
        output_dir / "runtime_heatmap_values.csv",
        matrices,
        relative_matrices,
    )
    plot_heatmaps(
        matrices,
        relative_matrices,
        output_dir / "runtime_heatmaps.png",
    )

    print("Wrote plots/runtime_heatmaps.png")
    print("Wrote plots/runtime_heatmap_values.csv")


if __name__ == "__main__":
    main()
