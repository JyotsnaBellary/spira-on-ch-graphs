#!/usr/bin/env python3
"""
Minimal plots (separate images) for Spira vs new variant on sparse vs dense graphs:

1) Ratio plot: NV/Spira relaxation ratio vs m (sparse and dense lines).
2) Pops plot: NV pops vs m and Spira pops vs m (sparse and dense lines).
"""

from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt

BASE = Path(__file__).parent


def load_avg(path: Path) -> pd.DataFrame:
    df = pd.read_csv(path)
    return df.sort_values("m")


def plot_ratio_vs_m(benchmark: str, weight_dir: str, weight_label: str) -> None:
    sparse_path = BASE / "sparse_networks" / weight_dir / benchmark / "average.csv"
    dense_path = BASE / "exp_complete_from_sparse" / weight_dir / benchmark / "average.csv"

    if not sparse_path.exists() or not dense_path.exists():
        print("Skipping missing:", sparse_path, dense_path)
        return

    df_s = load_avg(sparse_path)
    df_d = load_avg(dense_path)

    ratio_s = df_s["avg_nv_pops"] / df_s["avg_s_pops"]
    ratio_d = df_d["avg_nv_pops"] / df_d["avg_s_pops"]

    plt.figure(figsize=(9, 5))
    plt.plot(df_s["m"], ratio_s, marker="o", linestyle="-", color="#1f77b4", label="NV/Spira sparse")
    plt.plot(df_d["m"], ratio_d, marker="^", linestyle="--", color="#d62728", label="NV/Spira dense")
    plt.axhline(1.0, color="gray", linestyle="--", alpha=0.6, linewidth=1)
    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("m (edges)")
    plt.ylabel("Relaxation ratio (NV / Spira)")
    plt.title(f"Relaxation ratio vs m — {weight_label} — {benchmark}")
    plt.grid(True, linestyle=":", alpha=0.4)
    plt.legend()

    plt.tight_layout()
    out = BASE / f"relaxation_ratio_vs_m_{benchmark}_{weight_dir}.png"
    plt.savefig(out, dpi=200)
    print("Saved", out)


def plot_pops_vs_m(benchmark: str, weight_dir: str, weight_label: str) -> None:
    sparse_path = BASE / "sparse_networks" / weight_dir / benchmark / "average.csv"
    dense_path = BASE / "exp_complete_from_sparse" / weight_dir / benchmark / "average.csv"

    if not sparse_path.exists() or not dense_path.exists():
        print("Skipping missing:", sparse_path, dense_path)
        return

    df_s = load_avg(sparse_path)
    df_d = load_avg(dense_path)

    plt.figure(figsize=(9, 5))

    # slight x-offsets (log domain) to prevent marker overlap; safe since m>0
    offsets = {
        "sparse_nv": 0.96,
        "dense_nv": 1.04,
        "sparse_spira": 0.90,
        "dense_spira": 1.10,
    }

    plt.plot(
        df_s["m"] * offsets["sparse_nv"],
        df_s["avg_nv_pops"],
        marker="o",
        linestyle="-",
        color="#1f77b4",
        label="NV sparse",
    )
    plt.plot(
        df_d["m"] * offsets["dense_nv"],
        df_d["avg_nv_pops"],
        marker="^",
        linestyle="--",
        color="#d62728",
        label="NV dense",
    )
    plt.plot(
        df_s["m"] * offsets["sparse_spira"],
        df_s["avg_s_pops"],
        marker="s",
        linestyle="-",
        color="#2ca02c",
        alpha=0.9,
        label="Spira sparse",
    )
    plt.plot(
        df_d["m"] * offsets["dense_spira"],
        df_d["avg_s_pops"],
        marker="D",
        linestyle="--",
        color="#9467bd",
        alpha=0.9,
        label="Spira dense",
    )

    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("m (edges)")
    plt.ylabel("PQ pops (relaxed edges)")
    plt.title(f"Pops vs m — {weight_label} — {benchmark}")
    plt.grid(True, linestyle=":", alpha=0.4)
    plt.legend()

    plt.tight_layout()
    out = BASE / f"pops_vs_m_{benchmark}_{weight_dir}.png"
    plt.savefig(out, dpi=200)
    print("Saved", out)


def main() -> None:
    weight_labels = {
        "exponential_weights": "Exponential",
        "original_weights": "Geometric",
        "random_weights": "Random",
    }

    for benchmark in ("spt_benchmark", "src_dst_benchmark"):
        for wdir, wlabel in weight_labels.items():
            plot_ratio_vs_m(benchmark, wdir, wlabel)
            plot_pops_vs_m(benchmark, wdir, wlabel)


if __name__ == "__main__":
    main()
