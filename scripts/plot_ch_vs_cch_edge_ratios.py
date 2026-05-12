#!/usr/bin/env python3
"""Compare CH and CCH edge-normalized metrics on matching benchmark output."""

from __future__ import annotations

import argparse
import csv
import os
import re
from pathlib import Path
from statistics import mean

os.environ.setdefault("MPLCONFIGDIR", "/tmp/matplotlib")

import matplotlib.pyplot as plt


ROOT = Path(__file__).resolve().parents[1]

THRESHOLDS = [
    ("output_1_4", "25%"),
    ("output_half", "50%"),
    ("output_3_4", "75%"),
]

WEIGHTS = [
    ("original_weights", "Original"),
    ("uniform_weights", "Uniform"),
    ("exponential_weights", "Exponential"),
]

POP_METRICS = [
    ("d_pops", "Dijkstra"),
    ("s_pops", "Spira"),
    ("nv_pops", "New Variant"),
]

PERT_METRICS = [
    ("in_pert", "In"),
    ("out_pert", "Out"),
    ("pert", "Total"),
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


def read_query_ratio_map(root: Path, threshold: str, weight: str, metric: str) -> dict[str, float]:
    spt_dir = root / threshold / weight / "spt_benchmark"
    ratios = {}

    for path in sorted(spt_dir.glob("osm*.csv"), key=lambda item: osm_sort_key(item.stem)):
        values = []
        with path.open(newline="") as file:
            for row in csv.DictReader(file):
                query_edges = float(row["query_graph_edges"])
                values.append(float(row[metric]) / query_edges if query_edges > 0 else 0.0)
        if values:
            ratios[path.stem] = mean(values)

    return ratios


def collect_pop_rows(ch_root: Path, cch_root: Path) -> list[dict[str, object]]:
    rows = []
    for threshold, threshold_label in THRESHOLDS:
        for weight, weight_label in WEIGHTS:
            for column, metric_label in POP_METRICS:
                ch_ratios = read_query_ratio_map(ch_root, threshold, weight, column)
                cch_ratios = read_query_ratio_map(cch_root, threshold, weight, column)
                common_instances = sorted(ch_ratios.keys() & cch_ratios.keys(), key=osm_sort_key)

                for instance in common_instances:
                    ch_value = ch_ratios[instance]
                    cch_value = cch_ratios[instance]
                    rows.append(
                        {
                            "threshold": threshold_label,
                            "weight": weight_label,
                            "instance": instance,
                            "metric": metric_label,
                            "ch_value": ch_value,
                            "cch_value": cch_value,
                            "cch_to_ch_ratio": cch_value / ch_value if ch_value > 0 else 0.0,
                        }
                    )
    return rows


def collect_pert_rows(ch_root: Path, cch_root: Path) -> list[dict[str, object]]:
    rows = []
    for threshold, threshold_label in THRESHOLDS:
        for weight, weight_label in WEIGHTS:
            for column, metric_label in PERT_METRICS:
                ch_ratios = read_query_ratio_map(ch_root, threshold, weight, column)
                cch_ratios = read_query_ratio_map(cch_root, threshold, weight, column)
                common_instances = sorted(ch_ratios.keys() & cch_ratios.keys(), key=osm_sort_key)

                for instance in common_instances:
                    ch_value = ch_ratios[instance]
                    cch_value = cch_ratios[instance]
                    rows.append(
                        {
                            "threshold": threshold_label,
                            "weight": weight_label,
                            "instance": instance,
                            "metric": metric_label,
                            "ch_value": ch_value,
                            "cch_value": cch_value,
                            "cch_to_ch_ratio": cch_value / ch_value if ch_value > 0 else 0.0,
                        }
                    )
    return rows


def write_rows(rows: list[dict[str, object]], path: Path) -> None:
    with path.open("w", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def write_summary(rows: list[dict[str, object]], path: Path) -> None:
    groups: dict[tuple[str, str, str], list[dict[str, object]]] = {}
    for row in rows:
        key = (str(row["threshold"]), str(row["weight"]), str(row["metric"]))
        groups.setdefault(key, []).append(row)

    with path.open("w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(["threshold", "weight", "metric", "matched_instances", "mean_ch", "mean_cch", "mean_cch_to_ch"])
        for key in sorted(groups):
            group_rows = groups[key]
            ch_values = [float(row["ch_value"]) for row in group_rows]
            cch_values = [float(row["cch_value"]) for row in group_rows]
            ratios = [float(row["cch_to_ch_ratio"]) for row in group_rows]
            writer.writerow(
                [
                    *key,
                    len(group_rows),
                    f"{mean(ch_values):.8f}",
                    f"{mean(cch_values):.8f}",
                    f"{mean(ratios):.8f}",
                ]
            )


def plot_grid(
    rows: list[dict[str, object]],
    metrics: list[tuple[str, str]],
    output_path: Path,
    title: str,
    ylabel: str,
) -> None:
    fig, axes = plt.subplots(
        len(THRESHOLDS),
        len(WEIGHTS),
        figsize=(15.5, 10.5),
        sharey=True,
        constrained_layout=True,
    )

    x_positions = list(range(len(metrics)))
    labels = [label for _, label in metrics]
    width = 0.36

    for row_index, (_, threshold_label) in enumerate(THRESHOLDS):
        for col_index, (_, weight_label) in enumerate(WEIGHTS):
            ax = axes[row_index][col_index]
            ch_values = []
            cch_values = []

            for _, metric_label in metrics:
                subset = [
                    row
                    for row in rows
                    if row["threshold"] == threshold_label
                    and row["weight"] == weight_label
                    and row["metric"] == metric_label
                ]
                ch_values.append(mean(float(row["ch_value"]) for row in subset))
                cch_values.append(mean(float(row["cch_value"]) for row in subset))

            ax.bar(
                [position - width / 2 for position in x_positions],
                ch_values,
                width=width,
                color="#8ecae6",
                edgecolor="#2f4858",
                label="CH",
            )
            ax.bar(
                [position + width / 2 for position in x_positions],
                cch_values,
                width=width,
                color="#ffb703",
                edgecolor="#6c4700",
                label="CCH",
            )

            for position, ch_value, cch_value in zip(x_positions, ch_values, cch_values):
                ratio = cch_value / ch_value if ch_value > 0 else 0.0
                ax.text(
                    position,
                    max(ch_value, cch_value) * 1.06,
                    f"{ratio:.2f}x",
                    ha="center",
                    va="bottom",
                    fontsize=8,
                )

            ax.set_title(f"{threshold_label}, {weight_label}")
            ax.set_xticks(x_positions, labels, rotation=20, ha="right")
            ax.grid(axis="y", linestyle=":", alpha=0.4)

            if col_index == 0:
                ax.set_ylabel(ylabel)

            if row_index == 0 and col_index == len(WEIGHTS) - 1:
                ax.legend(framealpha=0.9)

    fig.suptitle(title)
    fig.savefig(output_path, dpi=300)
    plt.close(fig)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ch-root", type=Path, default=ROOT / "output_ch" / "sparse_networks")
    parser.add_argument("--cch-root", type=Path, default=ROOT / "output_cch" / "sparse_networks")
    parser.add_argument("--output-dir", type=Path, default=ROOT / "plots" / "CH_vs_CCH" / "edge_ratios")
    args = parser.parse_args()

    args.output_dir.mkdir(parents=True, exist_ok=True)

    pop_rows = collect_pop_rows(args.ch_root, args.cch_root)
    pert_rows = collect_pert_rows(args.ch_root, args.cch_root)

    write_rows(pop_rows, args.output_dir / "ch_vs_cch_pops_over_query_edges.csv")
    write_summary(pop_rows, args.output_dir / "ch_vs_cch_pops_over_query_edges_summary.csv")
    write_rows(pert_rows, args.output_dir / "ch_vs_cch_pertinent_over_query_edges.csv")
    write_summary(pert_rows, args.output_dir / "ch_vs_cch_pertinent_over_query_edges_summary.csv")

    plot_grid(
        pop_rows,
        POP_METRICS,
        args.output_dir / "ch_vs_cch_pops_over_query_edges_grid.png",
        "CH vs CCH priority-queue pops per reachable query edge",
        "Mean pops / query edge",
    )
    plot_grid(
        pert_rows,
        PERT_METRICS,
        args.output_dir / "ch_vs_cch_pertinent_over_query_edges_grid.png",
        "CH vs CCH pertinent edges per reachable query edge",
        "Mean pertinent edges / query edge",
    )

    print(f"Wrote CH vs CCH edge-normalized plots to {args.output_dir}")


if __name__ == "__main__":
    main()
