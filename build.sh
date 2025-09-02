#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./build.sh [Debug|Release|RelWithDebInfo] [build|run|run-only|perf|perf-top|flame|callgrind|massif] [-- app args...]
#
# Examples:
#   KAHIP_HOME=/path/to/KaHIP ./build.sh
#   KAHIP_NODE_ORDERING=/path/to/node_ordering ./build.sh Release run
#   ./build.sh Debug run -- --input foo
#   ./build.sh Debug run my-arg                      # (no '--' needed)
#   ./build.sh Debug build                           # build only
#   ./build.sh Debug run-only                        # run without rebuilding

BUILD_TYPE="${1:-Debug}"
MODE="${2:-run}"

# Resolve paths
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$SCRIPT_DIR"
BUILD_DIR="$ROOT/build/$BUILD_TYPE"
APP="$BUILD_DIR/erp_cli"

echo "[erp] Build dir: $BUILD_DIR  (type=$BUILD_TYPE, mode=$MODE)"
mkdir -p "$BUILD_DIR"

# Optional KaHIP hints (set as env vars before calling this script)
EXTRA_CMAKE=()
[[ -n "${KAHIP_NODE_ORDERING:-}" ]] && EXTRA_CMAKE+=("-DKAHIP_NODE_ORDERING=${KAHIP_NODE_ORDERING}")
[[ -n "${KAHIP_HOME:-}"          ]] && EXTRA_CMAKE+=("-DKAHIP_HOME=${KAHIP_HOME}")

# Parse optional app args:
# 1) If '--' present, everything after it are app args.
# 2) Otherwise, treat args after MODE as app args (so './build.sh Debug run arg1 arg2' works).
APP_ARGS=()
DASH_DASH_INDEX=""
for i in $(seq 1 $#); do
  if [[ "${!i}" == "--" ]]; then
    DASH_DASH_INDEX="$i"
    break
  fi
done
if [[ -n "$DASH_DASH_INDEX" ]]; then
  # shellcheck disable=SC2086
  APP_ARGS=("${@:$((DASH_DASH_INDEX+1))}")
elif [[ $# -ge 3 ]]; then
  APP_ARGS=("${@:3}")
fi

# Build control
do_build=1
case "$MODE" in
  build)    do_build=1 ;;
  run-only) do_build=0 ;;
  *)        do_build=1 ;;
esac

# Configure + Build (out-of-source)
if (( do_build )); then
  EXTRA_FLAGS="-fno-omit-frame-pointer -g"
  cmake -S "$ROOT" -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_CXX_FLAGS="$EXTRA_FLAGS ${CMAKE_CXX_FLAGS:-}" \
        "${EXTRA_CMAKE[@]}"
  cmake --build "$BUILD_DIR" -j"$(nproc)"
  if [[ "$MODE" == "build" ]]; then
    echo "[erp] Build finished: $APP"
    exit 0
  fi
fi

# Ensure binary exists (for run modes)
if [[ ! -x "$APP" ]]; then
  echo "[erp] ERROR: $APP not found or not executable."
  exit 1
fi

# Run / profiling modes
case "$MODE" in
  run)
    echo "[erp] Running CLI..."
    exec "$APP" "${APP_ARGS[@]}"
    ;;
  run-only)
    echo "[erp] Running CLI (no rebuild)..."
    exec "$APP" "${APP_ARGS[@]}"
    ;;
  perf)
    echo "[erp] perf record (99 Hz, with call stacks)"
    sudo perf record -F 99 -g -- "$APP" "${APP_ARGS[@]}"
    echo; echo "[erp] perf report"
    exec sudo perf report --demangle
    ;;
  perf-top)
    echo "[erp] perf top (press q to quit)"
    exec sudo perf top --demangle -- "$APP" "${APP_ARGS[@]}"
    ;;
  flame)
    echo "[erp] Generating flame graph..."
    sudo perf record -F 99 -g -- "$APP" "${APP_ARGS[@]}"
    sudo perf script > "$BUILD_DIR/out.perf"
    ~/.flamegraph/stackcollapse-perf.pl "$BUILD_DIR/out.perf" | \
      ~/.flamegraph/flamegraph.pl > "$BUILD_DIR/flame.svg"
    echo "[erp] Flame graph -> $BUILD_DIR/flame.svg"
    ;;
  callgrind)
    echo "[erp] Valgrind Callgrind"
    valgrind --tool=callgrind --callgrind-out-file="$BUILD_DIR/callgrind.out" "$APP" "${APP_ARGS[@]}"
    echo "[erp] Open with: kcachegrind $BUILD_DIR/callgrind.out"
    ;;
  massif)
    echo "[erp] Valgrind Massif"
    valgrind --tool=massif --massif-out-file="$BUILD_DIR/massif.out" "$APP" "${APP_ARGS[@]}"
    ms_print "$BUILD_DIR/massif.out" | less
    ;;
  *)
    echo "Unknown mode: $MODE"; exit 1;;
esac
