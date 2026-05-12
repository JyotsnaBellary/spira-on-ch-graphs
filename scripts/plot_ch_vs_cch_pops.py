#!/usr/bin/env python3
"""Compare CH and CCH priority-queue pops on matching benchmark rows."""

from __future__ import annotations

import argparse
import csv
import os
import re
from pathlib import Path
from statistics import median

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

METRICS = [
    ("avg_d_pops", "Dijkstra"),
    ("avg_s_pops", "Spira"),
    ("avg_nv_pops", "New Variant"),
    ("avg_nvq_pops", "New Variant Q"),
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


def collect_rows(ch_root: Path, cch_root: Path) -> list[dict[str, object]]:
    rows = []
    for threshold, threshold_label in THRESHOLDS:
        for weight, weight_label in WEIGHTS:
            ch_rows = read_average_map(ch_root, threshold, weight)
            cch_rows = read_average_map(cch_root, threshold, weight)
            common_instances = sorted(ch_rows.keys() & cch_rows.keys(), key=osm_sort_key)

            for instance in common_instances:
                for column, metric_label in METRICS:
                    ch_value = float(ch_rows[instance][column])
                    cch_value = float(cch_rows[instance][column])
                    rows.append(
                        {
                            "threshold": threshold,
                            "threshold_label": threshold_label,
                            "weight": weight,
                            "weight_label": weight_label,
                            "instance": instance,
                            "metric": metric_label,
                            "ch_value": ch_value,
                            "cch_value": cch_value,
                            "cch_to_ch_ratio": cch_value / ch_value if ch_value > 0 else 0.0,
                        }
                    )
    return rows


def write_csvs(rows: list[dict[str, object]], output_dir: Path) -> None:
    detail_path = output_dir / "ch_vs_cch_pops.csv"
    with detail_path.open("w", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)

    summary_path = output_dir / "ch_vs_cch_pops_summary.csv"
    groups: dict[tuple[str, str, str], list[dict[str, object]]] = {}
    for row in rows:
        key = (str(row["threshold_label"]), str(row["weight_label"]), str(row["metric"]))
        groups.setdefault(key, []).append(row)

    with summary_path.open("w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(
            [
                "threshold",
                "weight",
                "metric",
                "matched_instances",
                "median_ch",
                "median_cch",
                "median_cch_to_ch",
            ]
        )
        for key in sorted(groups):
            group_rows = groups[key]
            ch_values = [float(row["ch_value"]) for row in group_rows]
            cch_values = [float(row["cch_value"]) for row in group_rows]
            ratios = [float(row["cch_to_ch_ratio"]) for row in group_rows]
            writer.writerow(
                [
                    *key,
                    len(group_rows),
                    f"{median(ch_values):.8f}",
                    f"{median(cch_values):.8f}",
                    f"{median(ratios):.8f}",
                ]
            )


def plot_combined_grid(rows: list[dict[str, object]], output_dir: Path) -> None:
    fig, axes = plt.subplots(
        len(THRESHOLDS),
        len(WEIGHTS),
        figsize=(16.5, 10.8),
        sharey=True,
        constrained_layout=True,
    )

    x_positions = list(range(len(METRICS)))
    labels = [label for _, label in METRICS]
    width = 0.36

    for row_index, (_, threshold_label) in enumerate(THRESHOLDS):
        for col_index, (_, weight_label) in enumerate(WEIGHTS):
            ax = axes[row_index][col_index]
            ch_medians = []
            cch_medians = []

            for _, metric_label in METRICS:
                subset = [
                    row
                    for row in rows
                    if row["threshold_label"] == threshold_label
                    and row["weight_label"] == weight_label
                    and row["metric"] == metric_label
                ]
                ch_medians.append(median(float(row["ch_value"]) for row in subset))
                cch_medians.append(median(float(row["cch_value"]) for row in subset))

            ax.bar(
                [position - width / 2 for position in x_positions],
                ch_medians,
                width=width,
                color="#8ecae6",
                edgecolor="#2f4858",
                label="CH",
            )
            ax.bar(
                [position + width / 2 for position in x_positions],
                cch_medians,
                width=width,
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
            ax.set_xticks(x_positions, labels, rotation=20, ha="right")
            ax.set_yscale("log")
            ax.grid(axis="y", which="both", linestyle=":", alpha=0.4)

            if col_index == 0:
                ax.set_ylabel("Median priority queue pops per query")

            if row_index == 0 and col_index == len(WEIGHTS) - 1:
                ax.legend(framealpha=0.9)

    fig.suptitle("CH vs CCH priority-queue pops by threshold and weight")
    fig.savefig(output_dir / "ch_vs_cch_pops_grid.png", dpi=300)
    plt.close(fig)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ch-root", type=Path, default=ROOT / "output_ch" / "sparse_networks")
    parser.add_argument("--cch-root", type=Path, default=ROOT / "output_cch" / "sparse_networks")
    parser.add_argument("--output-dir", type=Path, default=ROOT / "plots" / "CH_vs_CCH" / "pops")
    args = parser.parse_args()

    args.output_dir.mkdir(parents=True, exist_ok=True)
    rows = collect_rows(args.ch_root, args.cch_root)
    write_csvs(rows, args.output_dir)
    plot_combined_grid(rows, args.output_dir)

    print(f"Wrote CH vs CCH priority-queue-pop comparison to {args.output_dir}")


if __name__ == "__main__":
    main()
