#!/usr/bin/env python3
"""
Compare pertinent out/in percentages between sparse graphs and exp-complete-from-sparse graphs.
One plot per weight family and benchmark, bars are % of m on a log scale to keep small values visible.
"""

from pathlib import Path
from typing import Dict

import matplotlib.pyplot as plt
from matplotlib import colors
import pandas as pd


ROOT = Path(__file__).parent
SPARSE_ROOT = ROOT.parent / "sparse_networks"

# Fixed matched m-pairs (sparse_m, expcomplete_m)
M_PAIRS = [
    (1080, 1122),
    (2168, 2256),
    (4380, 4422),
    (10814, 10920),
    (21498, 21756),
    (42790, 43056),
    (107402, 107912),
    (214362, 214832),
    (428362, 428370),
    (1070252, 1072260),
    (2125904, 2127222),
]


def lighten(color: str, factor: float = 0.5) -> tuple[float, float, float]:
    """Lighten a color toward white by the given factor."""
    r, g, b = colors.to_rgb(color)
    return (r + (1 - r) * factor, g + (1 - g) * factor, b + (1 - b) * factor)


def load_avg(base: Path, weight_dir: str, benchmark: str) -> pd.DataFrame | None:
    csv_path = base / weight_dir / benchmark / "average.csv"
    if not csv_path.exists():
        print(f"Missing: {csv_path}")
        return None
    df = pd.read_csv(csv_path)
    needed = ["filename", "m", "avg_out", "avg_in"]
    missing = [c for c in needed if c not in df.columns]
    if missing:
        raise ValueError(f"Missing columns {missing} in {csv_path}")
    df = df.sort_values("m")
    df["out_per_m"] = df["avg_out"] / df["m"] * 100.0
    df["in_per_m"] = df["avg_in"] / df["m"] * 100.0
    return df


def plot_compare(weight_dir: str, weight_label: str, weight_color: str, benchmark: str) -> None:
    sparse_df = load_avg(SPARSE_ROOT, weight_dir, benchmark)
    exp_df = load_avg(ROOT, weight_dir, benchmark)
    if sparse_df is None or exp_df is None:
        print(f"Skip {weight_dir} / {benchmark} (missing data)")
        return

    # Align to fixed m-pairs
    sparse_lookup = {m: row for m, row in sparse_df.set_index("m").iterrows()}
    exp_lookup = {m: row for m, row in exp_df.set_index("m").iterrows()}
    xs = []
    labels = []
    sparse_out = []
    sparse_in = []
    exp_out = []
    exp_in = []

    for idx, (m_sparse, m_exp) in enumerate(M_PAIRS):
        if m_sparse not in sparse_lookup or m_exp not in exp_lookup:
            continue
        xs.append(idx)
        labels.append(f"{m_sparse}/{m_exp}")
        sparse_row = sparse_lookup[m_sparse]
        exp_row = exp_lookup[m_exp]
        sparse_out.append(sparse_row["out_per_m"])
        sparse_in.append(sparse_row["in_per_m"])
        exp_out.append(exp_row["out_per_m"])
        exp_in.append(exp_row["in_per_m"])

    if not xs:
        print(f"No matched m-pairs found for {weight_dir} / {benchmark}")
        return

    plt.figure(figsize=(12, 5))

    # Use distinct colors per dataset
    sparse_out_color = weight_color
    sparse_in_color = lighten(weight_color, 0.25)
    exp_out_color = "#ff7f0e"  # orange
    exp_in_color = lighten("#ff7f0e", 0.35)

    # Sparse lines
    plt.plot(xs, sparse_out, marker="o", linestyle="-", color=sparse_out_color, label=f"{weight_label} sparse out")
    plt.plot(xs, sparse_in, marker="s", linestyle="--", color=sparse_in_color, label=f"{weight_label} sparse in")

    # Exp-complete-from-sparse lines
    plt.plot(xs, exp_out, marker="o", linestyle="-", color=exp_out_color, alpha=0.8, label=f"{weight_label} exp-complete out")
    plt.plot(xs, exp_in, marker="s", linestyle="--", color=exp_in_color, alpha=0.9, label=f"{weight_label} exp-complete in")

    plt.yscale("log")
    plt.xticks(xs, labels, rotation=30, ha="right")
    plt.ylabel("Pertinent edges (% of total edges)")
    plt.xlabel("Matched edges (sparse / exp-complete)")
    plt.title(f"Pertinent edges (out/in) — {weight_label} — {benchmark}")
    plt.grid(True, linestyle=":", alpha=0.4, which="both")

    positives = []
    for df in (sparse_df, exp_df):
        positives.extend(df["out_per_m"].tolist())
        positives.extend(df["in_per_m"].tolist())
    positives = [v for v in positives if v > 0]
    if positives:
        ymin = max(min(positives) * 0.5, 1e-6)
        plt.ylim(bottom=ymin)

    plt.legend(ncol=2, title="Dataset × direction")
    plt.tight_layout()
    out = ROOT / f"pertinent_compare_sparse_vs_expcomplete_{weight_dir}_{benchmark}_line.png"
    plt.savefig(out, dpi=200)
    print(f"Saved {out}")
    plt.close()


def main() -> None:
    weight_config = {
        "exponential_weights": ("Exponential", "#1f77b4"),
        "random_weights": ("Random", "#d62728"),
    }
    for benchmark in ("spt_benchmark", "src_dst_benchmark"):
        for wdir, (label, color) in weight_config.items():
            plot_compare(wdir, label, color, benchmark)


if __name__ == "__main__":
    main()
