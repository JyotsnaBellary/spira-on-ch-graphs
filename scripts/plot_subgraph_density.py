#!/usr/bin/env python3
"""Plot upward subgraph density from benchmark osm*.csv files."""

from __future__ import annotations

import argparse
import csv
import math
import os
import re
from collections import defaultdict
from pathlib import Path
from statistics import mean, median, stdev

os.environ.setdefault("MPLCONFIGDIR", "/tmp/matplotlib")

import matplotlib.pyplot as plt


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_INPUT_ROOT = ROOT / "output_ch" / "sparse_networks"
DEFAULT_OUTPUT_DIR = ROOT / "plots" / "subgraph_density_ch"
RANK_BUCKETS = 10


def osm_sort_key(name: str) -> int:
    match = re.search(r"(\d+)$", name)
    return int(match.group(1)) if match else 10**9


def percentile(sorted_values: list[float], fraction: float) -> float:
    if not sorted_values:
        return 0.0

    position = (len(sorted_values) - 1) * fraction
    lower = math.floor(position)
    upper = math.ceil(position)

    if lower == upper:
        return sorted_values[lower]

    return sorted_values[lower] + (sorted_values[upper] - sorted_values[lower]) * (position - lower)


def iqr(values: list[float]) -> float:
    sorted_values = sorted(values)
    return percentile(sorted_values, 0.75) - percentile(sorted_values, 0.25)


def density_from_counts(node_count: int, edge_count: int) -> float:
    if node_count <= 1:
        return 0.0
    return edge_count / (node_count * (node_count - 1) / 2.0)


def read_density_rows(spt_dir: Path) -> list[dict[str, float | int | str]]:
    rows = []

    for csv_path in sorted(spt_dir.glob("osm*.csv"), key=lambda path: osm_sort_key(path.stem)):
        with csv_path.open(newline="") as file:
            reader = csv.DictReader(file)
            required = {"rank", "nodes", "edges", "reachable_nodes", "query_graph_edges"}
            missing = required - set(reader.fieldnames or [])
            if missing:
                raise ValueError(f"{csv_path} is missing columns: {', '.join(sorted(missing))}")

            for row in reader:
                full_nodes = int(row["nodes"])
                reachable_nodes = int(row["reachable_nodes"])
                query_edges = int(row["query_graph_edges"])
                rank = int(row["rank"])

                if row.get("subgraph_density", "") != "":
                    density = float(row["subgraph_density"])
                    if not math.isfinite(density):
                        density = density_from_counts(reachable_nodes, query_edges)
                else:
                    density = density_from_counts(reachable_nodes, query_edges)

                rank_percentile = rank / (full_nodes - 1) if full_nodes > 1 else 0.0
                rows.append(
                    {
                        "instance": csv_path.stem,
                        "nodes": full_nodes,
                        "edges": int(row["edges"]),
                        "rank": rank,
                        "rank_percentile": rank_percentile,
                        "reachable_nodes": reachable_nodes,
                        "query_graph_edges": query_edges,
                        "subgraph_density": density,
                    }
                )

    if not rows:
        raise ValueError(f"No osm*.csv files found in {spt_dir}")

    return rows


def group_by_instance(rows: list[dict[str, float | int | str]]) -> list[tuple[str, list[dict[str, float | int | str]]]]:
    groups = defaultdict(list)
    for row in rows:
        groups[str(row["instance"])].append(row)

    return sorted(
        groups.items(),
        key=lambda item: (int(item[1][0]["nodes"]), osm_sort_key(item[0])),
    )


def write_instance_summary(instance_groups: list[tuple[str, list[dict[str, float | int | str]]]], output_dir: Path) -> None:
    output_path = output_dir / "density_by_instance.csv"
    with output_path.open("w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(
            [
                "instance",
                "nodes",
                "edges",
                "sample_count",
                "mean_density",
                "median_density",
                "stddev_density",
                "iqr_density",
                "mean_reachable_nodes",
                "median_reachable_nodes",
                "mean_query_graph_edges",
                "median_query_graph_edges",
                "mean_rank_percentile",
                "median_rank_percentile",
            ]
        )

        for instance, rows in instance_groups:
            densities = [float(row["subgraph_density"]) for row in rows]
            reachable_nodes = [int(row["reachable_nodes"]) for row in rows]
            query_edges = [int(row["query_graph_edges"]) for row in rows]
            rank_percentiles = [float(row["rank_percentile"]) for row in rows]
            writer.writerow(
                [
                    instance,
                    rows[0]["nodes"],
                    rows[0]["edges"],
                    len(rows),
                    f"{mean(densities):.8f}",
                    f"{median(densities):.8f}",
                    f"{stdev(densities):.8f}" if len(densities) > 1 else "0.00000000",
                    f"{iqr(densities):.8f}",
                    f"{mean(reachable_nodes):.8f}",
                    f"{median(reachable_nodes):.8f}",
                    f"{mean(query_edges):.8f}",
                    f"{median(query_edges):.8f}",
                    f"{mean(rank_percentiles):.8f}",
                    f"{median(rank_percentiles):.8f}",
                ]
            )


def plot_density_by_graph_size(
    instance_groups: list[tuple[str, list[dict[str, float | int | str]]]],
    output_dir: Path,
    title_suffix: str,
) -> None:
    labels = [instance for instance, _ in instance_groups]
    density_values = [[float(row["subgraph_density"]) for row in rows] for _, rows in instance_groups]
    means = [mean(values) for values in density_values]

    fig, ax = plt.subplots(figsize=(11, 5.4))
    box = ax.boxplot(
        density_values,
        labels=labels,
        showfliers=True,
        patch_artist=True,
        medianprops={"color": "black", "linewidth": 1.4},
        boxprops={"facecolor": "#8ecae6", "edgecolor": "#2f4858", "linewidth": 1.0},
        whiskerprops={"color": "#2f4858"},
        capprops={"color": "#2f4858"},
        flierprops={"marker": ".", "markersize": 3, "markerfacecolor": "#6c757d", "markeredgecolor": "#6c757d", "alpha": 0.55},
    )
    del box

    ax.plot(range(1, len(labels) + 1), means, color="#d00000", marker="o", linewidth=1.2, markersize=4, label="mean")
    ax.set_title(f"Upward subgraph density by graph size ({title_suffix})")
    ax.set_xlabel("Graph instance, sorted by full graph node count")
    ax.set_ylabel("Subgraph density")
    ax.set_ylim(bottom=0)
    ax.grid(axis="y", linestyle=":", alpha=0.45)
    ax.legend(frameon=False)
    fig.tight_layout()
    fig.savefig(output_dir / "density_by_graph_size_boxplot.png", dpi=300)
    plt.close(fig)


def rank_bucket(rank_percentile: float) -> int:
    return min(RANK_BUCKETS - 1, max(0, int(rank_percentile * RANK_BUCKETS)))


def write_rank_bucket_summary(instance_groups: list[tuple[str, list[dict[str, float | int | str]]]], output_dir: Path) -> None:
    output_path = output_dir / "density_by_rank_bucket.csv"
    with output_path.open("w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(["instance", "rank_bucket", "sample_count", "mean_density", "median_density", "iqr_density"])

        for instance, rows in instance_groups:
            buckets = defaultdict(list)
            for row in rows:
                buckets[rank_bucket(float(row["rank_percentile"]))].append(float(row["subgraph_density"]))

            for bucket in range(RANK_BUCKETS):
                values = buckets.get(bucket, [])
                bucket_label = f"{bucket * 10}-{(bucket + 1) * 10}%"
                if values:
                    writer.writerow(
                        [
                            instance,
                            bucket_label,
                            len(values),
                            f"{mean(values):.8f}",
                            f"{median(values):.8f}",
                            f"{iqr(values):.8f}",
                        ]
                    )
                else:
                    writer.writerow([instance, bucket_label, 0, "", "", ""])


def plot_density_by_rank_heatmap(
    instance_groups: list[tuple[str, list[dict[str, float | int | str]]]],
    output_dir: Path,
    title_suffix: str,
) -> None:
    labels = [instance for instance, _ in instance_groups]
    bucket_labels = [f"{bucket * 10}-{(bucket + 1) * 10}%" for bucket in range(RANK_BUCKETS)]
    matrix = []

    for _, rows in instance_groups:
        buckets = defaultdict(list)
        for row in rows:
            buckets[rank_bucket(float(row["rank_percentile"]))].append(float(row["subgraph_density"]))
        matrix.append([median(buckets[bucket]) if buckets.get(bucket) else math.nan for bucket in range(RANK_BUCKETS)])

    fig, ax = plt.subplots(figsize=(10.8, 5.8))
    cmap = plt.get_cmap("viridis").copy()
    cmap.set_bad("#f2f2f2")
    image = ax.imshow(matrix, aspect="auto", cmap=cmap)

    ax.set_title(f"Median upward subgraph density by source rank percentile ({title_suffix})")
    ax.set_xlabel("Source rank percentile")
    ax.set_ylabel("Graph instance")
    ax.set_xticks(range(RANK_BUCKETS), bucket_labels, rotation=35, ha="right")
    ax.set_yticks(range(len(labels)), labels)

    colorbar = fig.colorbar(image, ax=ax)
    colorbar.set_label("Median subgraph density")
    fig.tight_layout()
    fig.savefig(output_dir / "density_by_rank_percentile_heatmap.png", dpi=300)
    plt.close(fig)


def analyze_one(input_root: Path, threshold: str, weight: str, output_dir: Path) -> None:
    spt_dir = input_root / threshold / weight / "spt_benchmark"
    rows = read_density_rows(spt_dir)
    instance_groups = group_by_instance(rows)

    output_dir.mkdir(parents=True, exist_ok=True)
    title_suffix = f"{threshold}, {weight}"

    write_instance_summary(instance_groups, output_dir)
    write_rank_bucket_summary(instance_groups, output_dir)
    plot_density_by_graph_size(instance_groups, output_dir, title_suffix)
    plot_density_by_rank_heatmap(instance_groups, output_dir, title_suffix)

    print(f"Read {len(rows)} query subgraphs from {spt_dir}")
    print(f"Wrote {output_dir / 'density_by_graph_size_boxplot.png'}")
    print(f"Wrote {output_dir / 'density_by_rank_percentile_heatmap.png'}")
    print(f"Wrote {output_dir / 'density_by_instance.csv'}")
    print(f"Wrote {output_dir / 'density_by_rank_bucket.csv'}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input-root", type=Path, default=DEFAULT_INPUT_ROOT)
    parser.add_argument("--threshold", default="output_half")
    parser.add_argument("--weight", default="original_weights")
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    args = parser.parse_args()

    analyze_one(args.input_root, args.threshold, args.weight, args.output_dir)


if __name__ == "__main__":
    main()
