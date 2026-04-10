# #!/usr/bin/env python3
# """Compare sparse graphs vs size-matched complete graphs built from sparse edge counts.

# Metrics plotted vs edge count m (sorted ascending):
#   - Relaxation ratio (Spira pops / New Variant pops)
#   - Runtime ratios (Spira/Dijkstra and NewVariant/Dijkstra)
#   - Pertinent ratio (ratio_pert_m), assuming it refers to the new variant

# Graph families:
#   - Sparse: output/output/sparse_networks/<weight>/<benchmark>/average.csv
#   - Complete-from-sparse: output/output/exp_complete_from_sparse/<weight>/<benchmark>/average.csv

# Weights compared: exponential_weights, random_weights.
# Saves PNGs next to this script.
# """
# from pathlib import Path
# from typing import Dict

# import matplotlib.pyplot as plt
# import pandas as pd
# from matplotlib.lines import Line2D


# BASE = Path(__file__).parent


# def load_avg(csv_path: Path) -> pd.DataFrame:
#     df = pd.read_csv(csv_path)
#     return df.sort_values("m")


# def collect_m_values(benchmark: str, weight_map: Dict[str, str]):
#     """Use fixed sparse/complete m pairs provided by the user; one shared label per pair."""
#     pairs = [
#         (1080, 1122),
#         (2168, 2256),
#         (4380, 4422),
#         (10814, 10920),
#         (21498, 21756),
#         (42790, 43056),
#         (107402, 107912),
#         (214362, 214832),
#         (428362, 428370),
#         (1070252, 1072260),
#         (2125904, 2127222),
#     ]
#     labels = [f"{s}/{c}" for s, c in pairs]
#     index = {}
#     for idx, (s, c) in enumerate(pairs):
#         index[s] = idx
#         index[c] = idx
#     return labels, index


# def format_m_labels(ms):
#     """Pass through prepared string labels."""
#     return [str(x) for x in ms]


# def ensure_present(df: pd.DataFrame, cols):
#     missing = [c for c in cols if c not in df.columns]
#     if missing:
#         raise ValueError(f"Missing columns {missing} in {df}")


# def plot_relaxation(benchmark: str, weight_map: Dict[str, str]) -> None:
#     fig, ax = plt.subplots(figsize=(8, 5))
#     families = [
#         ("Sparse", BASE / "sparse_networks", "o"),
#         ("Complete-from-sparse", BASE / "exp_complete_from_sparse", "^"),
#     ]
#     plotted_weights = set()
#     ms, m_index = collect_m_values(benchmark, weight_map)
#     for family_label, root, marker in families:
#         for weight_dir, weight_label in weight_map.items():
#             csv_path = root / weight_dir / benchmark / "average.csv"
#             if not csv_path.exists():
#                 continue
#             df = load_avg(csv_path)
#             ensure_present(df, ["avg_s_pops", "avg_nv_pops", "m"])
#             df = df[df["m"].isin(m_index.keys())]
#             if df.empty:
#                 continue
#             ratio = df["avg_s_pops"] / df["avg_nv_pops"]
#             xs = [m_index[m] for m in df["m"]]
#             ax.plot(
#                 xs,
#                 ratio,
#                 color=COLORS[weight_dir],
#                 marker=marker,
#                 linestyle="-",
#                 label=None,
#             )
#             plotted_weights.add(weight_dir)

#     if ms:
#         ax.set_xticks(range(len(ms)))
#         ax.set_xticklabels(format_m_labels(ms), rotation=45, ha="right")

#     ax.axhline(1.0, color="gray", linewidth=1, linestyle="--", alpha=0.5)
#     ax.set_xlabel("m edges")
#     ax.set_ylabel("Spira pops / New Variant pops")
#     ax.set_title(f"Relaxation ratio vs m ({benchmark})")
#     ax.grid(True, which="both", linestyle=":", alpha=0.4)

#     weight_handles = [
#         Line2D([0], [0], color=COLORS[w], lw=2, label=weight_map[w])
#         for w in plotted_weights
#     ]
#     family_handles = [
#         Line2D([0], [0], color="black", marker=marker, linestyle="-", label=label, markersize=6)
#         for label, _, marker in families
#     ]
#     leg1 = ax.legend(handles=weight_handles, title="Edge weights", loc="upper left")
#     leg2 = ax.legend(handles=family_handles, title="Graph family", loc="upper right")
#     ax.add_artist(leg1)
#     ax.add_artist(leg2)

#     out = BASE / f"compare_relaxation_{benchmark}.png"
#     fig.tight_layout()
#     fig.savefig(out, dpi=200)
#     print(f"Saved {out}")


# def plot_runtime(benchmark: str, weight_map: Dict[str, str]) -> None:
#     fig, axes = plt.subplots(1, 2, figsize=(11, 4), sharex=True, sharey=True)
#     families = [
#         ("Sparse", BASE / "sparse_networks", "o"),
#         ("Complete-from-sparse", BASE / "exp_complete_from_sparse", "^"),
#     ]
#     ratio_cols = [("avg_s_time", "Spira/Dijkstra"), ("avg_nv_time", "New Variant/Dijkstra")]
#     plotted_weights = set()
#     ms, m_index = collect_m_values(benchmark, weight_map)

#     for ax, (col, title) in zip(axes, ratio_cols):
#         for family_label, root, marker in families:
#             for weight_dir, weight_label in weight_map.items():
#                 csv_path = root / weight_dir / benchmark / "average.csv"
#                 if not csv_path.exists():
#                     continue
#                 df = load_avg(csv_path)
#                 ensure_present(df, [col, "avg_d_time", "m"])
#                 df = df[df["m"].isin(m_index.keys())]
#                 if df.empty:
#                     continue
#                 ratio = df[col] / df["avg_d_time"]
#                 xs = [m_index[m] for m in df["m"]]
#                 ax.plot(
#                     xs,
#                     ratio,
#                     color=COLORS[weight_dir],
#                     marker=marker,
#                     linestyle="-",
#                     label=None,
#                 )
#                 plotted_weights.add(weight_dir)

#         ax.axhline(1.0, color="gray", linewidth=1, linestyle="--", alpha=0.5)
#         ax.set_title(title)
#         ax.grid(True, which="both", linestyle=":", alpha=0.4)
#         ax.set_xlabel("m (edges, equally spaced)")
#         if ax is axes[0]:
#             ax.set_ylabel("Relative runtime compared to Dijkstra")

#     if ms:
#         axes[0].set_xticks(range(len(ms)))
#         axes[0].set_xticklabels(format_m_labels(ms), rotation=45, ha="right")
#         axes[1].set_xticks(range(len(ms)))
#         axes[1].set_xticklabels(format_m_labels(ms), rotation=45, ha="right")

#     weight_handles = [
#         Line2D([0], [0], color=COLORS[w], lw=2, label=weight_map[w])
#         for w in plotted_weights
#     ]
#     family_handles = [
#         Line2D([0], [0], color="black", marker=marker, linestyle="-", label=label, markersize=6)
#         for label, _, marker in families
#     ]
#     leg1 = axes[0].legend(handles=weight_handles, title="Edge weights", loc="upper left")
#     leg2 = axes[1].legend(handles=family_handles, title="Graph family", loc="upper right")
#     axes[0].add_artist(leg1)
#     axes[1].add_artist(leg2)

#     out = BASE / f"compare_runtime_{benchmark}.png"
#     fig.tight_layout()
#     fig.savefig(out, dpi=200)
#     print(f"Saved {out}")


# def plot_pertinent(benchmark: str, weight_map: Dict[str, str]) -> None:
#     fig, ax = plt.subplots(figsize=(8, 5))
#     families = [
#         ("Sparse", BASE / "sparse_networks", "o"),
#         ("Complete-from-sparse", BASE / "exp_complete_from_sparse", "^"),
#     ]
#     plotted_weights = set()
#     ms, m_index = collect_m_values(benchmark, weight_map)

#     for family_label, root, marker in families:
#         for weight_dir, weight_label in weight_map.items():
#             csv_path = root / weight_dir / benchmark / "average.csv"
#             if not csv_path.exists():
#                 continue
#             df = load_avg(csv_path)
#             if "ratio_pert_m" not in df.columns or "m" not in df.columns:
#                 continue
#             df = df[df["m"].isin(m_index.keys())]
#             if df.empty:
#                 continue
#             xs = [m_index[m] for m in df["m"]]
#             ax.plot(
#                 xs,
#                 df["ratio_pert_m"],
#                 color=COLORS[weight_dir],
#                 marker=marker,
#                 linestyle="-",
#                 label=None,
#             )
#             plotted_weights.add(weight_dir)

#     if ms:
#         ax.set_xticks(range(len(ms)))
#         ax.set_xticklabels(format_m_labels(ms), rotation=45, ha="right")

#     ax.set_xlabel("m (edges, equally spaced)")
#     ax.set_ylabel("Pertinent / m (new variant)")
#     ax.set_title(f"Pertinent ratio vs m ({benchmark})")
#     ax.grid(True, which="both", linestyle=":", alpha=0.4)

#     weight_handles = [
#         Line2D([0], [0], color=COLORS[w], lw=2, label=weight_map[w])
#         for w in plotted_weights
#     ]
#     family_handles = [
#         Line2D([0], [0], color="black", marker=marker, linestyle="-", label=label, markersize=6)
#         for label, _, marker in families
#     ]
#     leg1 = ax.legend(handles=weight_handles, title="Edge weights", loc="upper left")
#     leg2 = ax.legend(handles=family_handles, title="Graph family", loc="upper right")
#     ax.add_artist(leg1)
#     ax.add_artist(leg2)

#     out = BASE / f"compare_pertinent_{benchmark}.png"
#     fig.tight_layout()
#     fig.savefig(out, dpi=200)
#     print(f"Saved {out}")


# COLORS = {
#     "exponential_weights": "#1f77b4",
#     "random_weights": "#d62728",
# }


# def main() -> None:
#     weight_map = {
#         "exponential_weights": "Exponential",
#         "random_weights": "Random",
#     }
#     for benchmark in ("spt_benchmark", "src_dst_benchmark"):
#         plot_relaxation(benchmark, weight_map)
#         plot_runtime(benchmark, weight_map)
#         plot_pertinent(benchmark, weight_map)


# if __name__ == "__main__":
#     main()

#!/usr/bin/env python3
"""
Plot absolute runtimes (not ratios) for uniform random distributed weight graphs:
 - Sparse OSM graphs
 - Complete-from-sparse dense graphs (same m values)

Lines plotted:
   Spira (sparse)
   Spira (dense)
   New Variant (sparse)
   New Variant (dense)

X-axis: m edges (matched pairs)
Y-axis: runtime (milliseconds)
"""

from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt

BASE = Path(__file__).parent

# The fixed matched m-pairs
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

def load(csv_path):
    df = pd.read_csv(csv_path)
    return df.set_index("m")

def plot_exponential_runtime(benchmark: str):
    sparse_root = BASE / "sparse_networks" / "random_weights" / benchmark / "average.csv"
    dense_root  = BASE / "exp_complete_from_sparse" / "random_weights" / benchmark / "average.csv"

    if not sparse_root.exists():
        raise FileNotFoundError(sparse_root)
    if not dense_root.exists():
        raise FileNotFoundError(dense_root)

    sparse_df = load(sparse_root)
    dense_df  = load(dense_root)

    # Build x-axis in correct order
    x_sparse = []
    x_dense = []
    spira_sparse = []
    spira_dense = []
    nv_sparse = []
    nv_dense = []

    for ms, md in M_PAIRS:
        if ms in sparse_df.index and md in dense_df.index:
            x_sparse.append(ms)
            x_dense.append(md)
            spira_sparse.append(sparse_df.loc[ms, "avg_s_time"])
            spira_dense.append(dense_df.loc[md, "avg_s_time"])
            nv_sparse.append(sparse_df.loc[ms, "avg_nv_time"])
            nv_dense.append(dense_df.loc[md, "avg_nv_time"])

    # Build figure
    plt.figure(figsize=(9, 5))

    # Plot Spira and New Variant lines
    plt.plot(x_sparse, spira_sparse, "o-", color="blue",  label="Spira (sparse)")
    plt.plot(x_dense,  spira_dense,  "o--", color="cyan",  label="Spira (dense)")
    plt.plot(x_sparse, nv_sparse,   "s-", color="red",   label="New Variant (sparse)")
    plt.plot(x_dense,  nv_dense,    "s--", color="orange", label="New Variant (dense)")

    plt.xscale("log")   # recommended, because m varies from 1k to >2M
    # plt.yscale("log")   # runtimes vary by orders of magnitude too

    plt.xlabel("m (edge count)")
    plt.ylabel("Runtime (ms)")
    plt.title(f"Runtime comparison on uniformly random weighted graphs ({benchmark})")
    plt.grid(True, which="both", linestyle=":", alpha=0.4)
    plt.legend()

    out = BASE / f"runtime_random_{benchmark}.png"
    plt.tight_layout()
    plt.savefig(out, dpi=200)
    print(f"Saved {out}")


def main():
    for benchmark in ("spt_benchmark", "src_dst_benchmark"):
        plot_exponential_runtime(benchmark)


if __name__ == "__main__":
    main()
