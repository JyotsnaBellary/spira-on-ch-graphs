#!/usr/bin/env python3
"""
Metrics summary for dense graphs.

Outputs CSV summaries:
  1) NV pops per edge (avg_nv_pops / m) per weight and benchmark.
  2) NV pops to pertinent edges ratio (avg_nv_pops / avg_pert) per weight and benchmark.
  3) Pertinent out:in ratio (ratio_spt_out / ratio_spt_in) per weight and benchmark.
  4) Pertinent share (ratio_pert_m) per weight and benchmark.
  5) Pertinent share vs n (ratio_pert_n) per weight and benchmark.
"""

from pathlib import Path
from typing import Dict, List

import pandas as pd

ROOT = Path(__file__).parent
BENCHMARKS = ["spt_benchmark", "src_dst_benchmark"]
WEIGHTS: Dict[str, str] = {
    "exponential_weights": "Exponential",
    "original_weights": "Geometric",
    "random_weights": "Random",
}


def load_csv(weight_dir: str, bench: str) -> pd.DataFrame | None:
    csv_path = ROOT / weight_dir / bench / "average.csv"
    if not csv_path.exists():
        print(f"Missing: {csv_path}")
        return None
    df = pd.read_csv(csv_path)
    return df


def agg_nv_per_m() -> Dict[str, Dict[str, float]]:
    """Average of (avg_nv_pops / m) per weight and benchmark."""
    out = {bench: {} for bench in BENCHMARKS}
    for bench in BENCHMARKS:
        for wdir, wlabel in WEIGHTS.items():
            df = load_csv(wdir, bench)
            if df is None or "avg_nv_pops" not in df or "m" not in df:
                continue
            vals = (df["avg_nv_pops"] / df["m"]).dropna()
            if not vals.empty:
                out[bench][wlabel] = vals.mean()
    return out


def agg_nv_to_pert() -> Dict[str, Dict[str, float]]:
    """Average of (avg_nv_pops / avg_pert) per weight and benchmark."""
    out = {bench: {} for bench in BENCHMARKS}
    for bench in BENCHMARKS:
        for wdir, wlabel in WEIGHTS.items():
            df = load_csv(wdir, bench)
            if df is None or "avg_nv_pops" not in df or "avg_pert" not in df:
                continue
            vals = (df["avg_nv_pops"] / df["avg_pert"].replace(0, pd.NA)).dropna()
            if not vals.empty:
                out[bench][wlabel] = vals.mean()
    return out


def agg_out_in_ratio() -> Dict[str, Dict[str, float]]:
    """Average of (ratio_spt_out / ratio_spt_in) per weight and benchmark."""
    out = {bench: {} for bench in BENCHMARKS}
    for bench in BENCHMARKS:
        for wdir, wlabel in WEIGHTS.items():
            df = load_csv(wdir, bench)
            if df is None or "ratio_spt_out" not in df or "ratio_spt_in" not in df:
                continue
            df = df[df["ratio_spt_in"] != 0]
            vals = (df["ratio_spt_out"] / df["ratio_spt_in"]).dropna()
            if not vals.empty:
                out[bench][wlabel] = vals.mean()
    return out


def agg_pert_share() -> Dict[str, Dict[str, float]]:
    """Average of ratio_pert_m per weight and benchmark."""
    out = {bench: {} for bench in BENCHMARKS}
    for bench in BENCHMARKS:
        for wdir, wlabel in WEIGHTS.items():
            df = load_csv(wdir, bench)
            if df is None or "ratio_pert_m" not in df:
                continue
            vals = df["ratio_pert_m"].dropna()
            if not vals.empty:
                out[bench][wlabel] = vals.mean()
    return out


def agg_pert_share_n() -> Dict[str, Dict[str, float]]:
    """Average of ratio_pert_n per weight and benchmark."""
    out = {bench: {} for bench in BENCHMARKS}
    for bench in BENCHMARKS:
        for wdir, wlabel in WEIGHTS.items():
            df = load_csv(wdir, bench)
            if df is None or "ratio_pert_n" not in df:
                continue
            vals = df["ratio_pert_n"].dropna()
            if not vals.empty:
                out[bench][wlabel] = vals.mean()
    return out


def make_table(means: Dict[str, Dict[str, float]], caption: str) -> str:
    labels: List[str] = sorted({*means["spt_benchmark"].keys(), *means["src_dst_benchmark"].keys()})
    lines = []
    lines.append(r"\begin{table}[h]")
    lines.append(r"  \centering")
    lines.append(r"  \begin{tabular}{lrr}")
    lines.append(r"  \toprule")
    lines.append(r"  Edge weight distribution & spt & src\_dst \\")
    lines.append(r"  \midrule")
    for label in labels:
        spt = means["spt_benchmark"].get(label, float("nan"))
        src = means["src_dst_benchmark"].get(label, float("nan"))
        spt_val = "-" if pd.isna(spt) else f"{spt:.6f}"
        src_val = "-" if pd.isna(src) else f"{src:.6f}"
        lines.append(f"  {label} & {spt_val} & {src_val} \\\\")
    lines.append(r"  \bottomrule")
    lines.append(r"  \end{tabular}")
    lines.append(f"  \\caption{{{caption}}}")
    lines.append(r"\end{table}")
    return "\n".join(lines)


def write_csv(means: Dict[str, Dict[str, float]], path: Path) -> None:
    rows = []
    labels: List[str] = sorted({*means["spt_benchmark"].keys(), *means["src_dst_benchmark"].keys()})
    for label in labels:
        rows.append(
            {
                "Edge weight distribution": label,
                "spt": means["spt_benchmark"].get(label),
                "src_dst": means["src_dst_benchmark"].get(label),
            }
        )
    pd.DataFrame(rows).to_csv(path, index=False)


def main() -> None:
    nv_per_m = agg_nv_per_m()
    nv_to_pert = agg_nv_to_pert()
    out_in = agg_out_in_ratio()
    pert_share = agg_pert_share()
    pert_share_n = agg_pert_share_n()

    # CSV outputs
    write_csv(nv_per_m, ROOT / "nv_pops_per_m_dense.csv")
    write_csv(nv_to_pert, ROOT / "nv_pops_to_pertinent_dense.csv")
    write_csv(out_in, ROOT / "pertinent_out_in_ratio_dense.csv")
    write_csv(pert_share, ROOT / "pertinent_share_dense.csv")
    write_csv(pert_share_n, ROOT / "pertinent_share_n_dense.csv")
    print(
        f"Written CSVs: {ROOT / 'nv_pops_per_m_dense.csv'}, "
        f"{ROOT / 'nv_pops_to_pertinent_dense.csv'}, "
        f"{ROOT / 'pertinent_out_in_ratio_dense.csv'}, "
        f"{ROOT / 'pertinent_share_dense.csv'}, "
        f"{ROOT / 'pertinent_share_n_dense.csv'}"
    )


if __name__ == "__main__":
    main()
