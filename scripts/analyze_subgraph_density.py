#!/usr/bin/env python3
"""Analyze density distributions for DAG/upward subgraphs.

Expected input: a CSV with one row per subgraph. The script needs node and edge
counts, and can optionally group by instance and anchor layer/depth.

Accepted column aliases:
  instance: instance, graph, graph_id, filename
  subgraph id: subgraph_id, subgraph, id, query_id
  node count: node_count, num_nodes, n, vertices
  edge count: edge_count, num_edges, m, edges
  anchor layer/depth: anchor_layer, layer, anchor_depth, depth

Density is computed as the maximum-simple-DAG density:
  density = m / (n * (n - 1) / 2)

This is equivalent to undirected simple graph density, but interpreted as:
"how many of the possible acyclic/upward edges between nodes are present?"
"""

from __future__ import annotations

import argparse
import csv
import math
from collections import defaultdict
from pathlib import Path
from statistics import mean, median, stdev
from typing import Iterable


COLUMN_ALIASES = {
    "instance": ("instance", "graph", "graph_id", "filename"),
    "subgraph_id": ("subgraph_id", "subgraph", "id", "query_id"),
    "node_count": ("node_count", "num_nodes", "n", "vertices"),
    "edge_count": ("edge_count", "num_edges", "m", "edges"),
    "anchor_depth": ("anchor_depth", "depth", "anchor_layer", "layer"),
}


def find_column(fieldnames: Iterable[str], logical_name: str, required: bool) -> str | None:
    normalized = {name.strip().lower(): name for name in fieldnames}

    for alias in COLUMN_ALIASES[logical_name]:
        if alias in normalized:
            return normalized[alias]

    if required:
        aliases = ", ".join(COLUMN_ALIASES[logical_name])
        raise ValueError(f"Missing required column for {logical_name}. Accepted names: {aliases}")

    return None


def parse_int(row: dict[str, str], column: str, row_number: int) -> int:
    value = row[column].strip()
    try:
        return int(value)
    except ValueError as error:
        raise ValueError(f"Row {row_number}: column {column!r} must be an integer, got {value!r}") from error


def dag_density(node_count: int, edge_count: int) -> float:
    max_edges = node_count * (node_count - 1) / 2
    if max_edges <= 0:
        return 0.0
    return edge_count / max_edges


def percentile(sorted_values: list[float], percent: float) -> float:
    if not sorted_values:
        return 0.0

    position = (len(sorted_values) - 1) * percent
    lower = math.floor(position)
    upper = math.ceil(position)

    if lower == upper:
        return sorted_values[int(position)]

    lower_value = sorted_values[lower]
    upper_value = sorted_values[upper]
    return lower_value + (upper_value - lower_value) * (position - lower)


def summarize(values: list[float]) -> dict[str, float | int]:
    if not values:
        return {
            "subgraph_count": 0,
            "mean_density": 0.0,
            "median_density": 0.0,
            "stddev_density": 0.0,
            "iqr_density": 0.0,
            "min_density": 0.0,
            "max_density": 0.0,
        }

    sorted_values = sorted(values)
    q1 = percentile(sorted_values, 0.25)
    q3 = percentile(sorted_values, 0.75)

    return {
        "subgraph_count": len(values),
        "mean_density": mean(values),
        "median_density": median(values),
        "stddev_density": stdev(values) if len(values) > 1 else 0.0,
        "iqr_density": q3 - q1,
        "min_density": sorted_values[0],
        "max_density": sorted_values[-1],
    }


def read_subgraphs(input_path: Path) -> list[dict[str, str | int | float]]:
    with input_path.open(newline="") as file:
        reader = csv.DictReader(file)
        if reader.fieldnames is None:
            raise ValueError(f"{input_path} has no CSV header")

        instance_col = find_column(reader.fieldnames, "instance", required=False)
        subgraph_col = find_column(reader.fieldnames, "subgraph_id", required=False)
        node_col = find_column(reader.fieldnames, "node_count", required=True)
        edge_col = find_column(reader.fieldnames, "edge_count", required=True)
        anchor_depth_col = find_column(reader.fieldnames, "anchor_depth", required=False)

        subgraphs = []
        for row_number, row in enumerate(reader, start=2):
            node_count = parse_int(row, node_col, row_number)
            edge_count = parse_int(row, edge_col, row_number)
            max_edges = node_count * (node_count - 1) // 2

            if node_count < 0:
                raise ValueError(f"Row {row_number}: node count must be non-negative")
            if edge_count < 0:
                raise ValueError(f"Row {row_number}: edge count must be non-negative")
            if edge_count > max_edges:
                raise ValueError(
                    f"Row {row_number}: edge count {edge_count} exceeds simple DAG maximum {max_edges}"
                )

            subgraphs.append(
                {
                    "instance": row[instance_col].strip() if instance_col else "all",
                    "subgraph_id": row[subgraph_col].strip() if subgraph_col else str(row_number - 1),
                    "node_count": node_count,
                    "edge_count": edge_count,
                    "max_dag_edges": max_edges,
                    "density": dag_density(node_count, edge_count),
                    "anchor_depth": row[anchor_depth_col].strip() if anchor_depth_col else "",
                }
            )

    return subgraphs


def write_rows(path: Path, rows: list[dict[str, object]]) -> None:
    if not rows:
        return

    with path.open("w", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def add_summary_row(group: str, key: object, subgraphs: list[dict[str, object]]) -> dict[str, object]:
    densities = [float(row["density"]) for row in subgraphs]
    summary = summarize(densities)
    return {
        "group": group,
        "key": key,
        **summary,
        "mean_node_count": mean([int(row["node_count"]) for row in subgraphs]),
        "mean_edge_count": mean([int(row["edge_count"]) for row in subgraphs]),
    }


def grouped_rows(group_name: str, subgraphs: list[dict[str, object]], column: str) -> list[dict[str, object]]:
    groups = defaultdict(list)
    for row in subgraphs:
        key = row[column]
        if key != "":
            groups[key].append(row)

    return [add_summary_row(group_name, key, groups[key]) for key in sorted(groups, key=str)]


def format_float(value: object) -> object:
    if isinstance(value, float):
        return f"{value:.8f}"
    return value


def format_rows(rows: list[dict[str, object]]) -> list[dict[str, object]]:
    return [{key: format_float(value) for key, value in row.items()} for row in rows]


def analyze(input_path: Path, output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    subgraphs = read_subgraphs(input_path)

    detailed_rows = [
        {
            "instance": row["instance"],
            "subgraph_id": row["subgraph_id"],
            "node_count": row["node_count"],
            "edge_count": row["edge_count"],
            "max_dag_edges": row["max_dag_edges"],
            "density": row["density"],
            "anchor_depth": row["anchor_depth"],
        }
        for row in subgraphs
    ]

    summary_rows = [add_summary_row("all", "all", subgraphs)]
    summary_rows.extend(grouped_rows("instance", subgraphs, "instance"))

    by_size_rows = grouped_rows("node_count", subgraphs, "node_count")
    by_anchor_rows = grouped_rows("anchor_depth", subgraphs, "anchor_depth")

    write_rows(output_dir / "subgraph_density_detailed.csv", format_rows(detailed_rows))
    write_rows(output_dir / "subgraph_density_summary.csv", format_rows(summary_rows))
    write_rows(output_dir / "subgraph_density_by_size.csv", format_rows(by_size_rows))
    write_rows(output_dir / "subgraph_density_by_anchor_depth.csv", format_rows(by_anchor_rows))

    overall = summary_rows[0]
    print(f"Read {len(subgraphs)} subgraphs from {input_path}")
    print(f"Mean density:   {overall['mean_density']:.8f}")
    print(f"Median density: {overall['median_density']:.8f}")
    print(f"Stddev:         {overall['stddev_density']:.8f}")
    print(f"IQR:            {overall['iqr_density']:.8f}")
    print(f"Wrote CSV files to {output_dir}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input_csv", type=Path, help="CSV containing one row per subgraph")
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("plots") / "subgraph_density",
        help="Directory for output CSV files",
    )
    args = parser.parse_args()

    analyze(args.input_csv, args.output_dir)


if __name__ == "__main__":
    main()
