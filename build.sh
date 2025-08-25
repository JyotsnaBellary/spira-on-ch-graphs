# #!/usr/bin/env bash
# set -euo pipefail

# # Default build type = Debug, override with ./build.sh Release
# BUILD_TYPE=${1:-Debug}

# # Where to put compiled files
# BUILD_DIR="build/$BUILD_TYPE"

# echo "[erp] Building in $BUILD_DIR (type=$BUILD_TYPE)"

# # Create build dir if missing
# mkdir -p "$BUILD_DIR"
# cd "$BUILD_DIR"

# # Run CMake configure
# cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ../..

# # Build (use all cores)
# cmake --build . -- -j$(nproc)

# # Optionally run tests if they exist
# # if command -v ctest &> /dev/null; then
# #     echo "[erp] Running tests..."
# #     ctest --output-on-failure
# # fi

# # Run the main program
# echo "[erp] Running CLI..."
# cd ../..                          # go back to project root
# ./build/"$BUILD_TYPE"/erp_cli  


#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./build.sh [Debug|Release|RelWithDebInfo] [run|perf|perf-top|flame|callgrind|massif] [-- app args...]
# Examples:
#   ./build.sh RelWithDebInfo perf -- --input data.txt
#   ./build.sh RelWithDebInfo run
#   ./build.sh RelWithDebInfo callgrind

BUILD_TYPE="${1:-Debug}"
MODE="${2:-run}"

# Resolve project root even if called from anywhere
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$SCRIPT_DIR"
BUILD_DIR="$ROOT/build/$BUILD_TYPE"
APP="$BUILD_DIR/erp_cli"

echo "[erp] Building in $BUILD_DIR (type=$BUILD_TYPE, mode=$MODE)"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Ensure symbols + frame pointers (good stacks in perf/callgrind)
EXTRA_FLAGS="-fno-omit-frame-pointer -g"
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
      -DCMAKE_CXX_FLAGS="$EXTRA_FLAGS ${CMAKE_CXX_FLAGS:-}" \
      "$ROOT"

cmake --build . -- -j"$(nproc)"

cd "$ROOT"

# Verify the binary exists
if [[ ! -x "$APP" ]]; then
  echo "[erp] ERROR: $APP not found or not executable."
  echo "       Check target name in CMake (should build 'erp_cli'), or adjust APP path."
  exit 1
fi

# Parse optional args after '--'
APP_ARGS=()
if [[ "${3-}" == "--" ]]; then
  shift 2
  APP_ARGS=("$@")
else
  # If only 1 arg given (BUILD_TYPE) we didn't shift MODE—handle common case:
  if [[ $# -gt 1 ]]; then
    shift 2
    APP_ARGS=("$@")
  else
    shift 1 || true
  fi
fi

case "$MODE" in
  run)
    echo "[erp] Running CLI..."
    exec "$APP" "${APP_ARGS[@]}"
    ;;
  perf)
    echo "[erp] perf record (99 Hz, with call stacks)"
    # Use sudo for perf on WSL; add --call-graph dwarf if stacks look bad
    sudo perf record -F 99 -g -- "$APP" "${APP_ARGS[@]}"
    echo
    echo "[erp] perf report"
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
    echo "[erp] Valgrind Callgrind (slow; use smaller input)"
    valgrind --tool=callgrind --callgrind-out-file="$BUILD_DIR/callgrind.out" \
      "$APP" "${APP_ARGS[@]}"
    echo "[erp] Open with: kcachegrind $BUILD_DIR/callgrind.out"
    ;;
  massif)
    echo "[erp] Valgrind Massif (heap profiling; slow)"
    valgrind --tool=massif --massif-out-file="$BUILD_DIR/massif.out" \
      "$APP" "${APP_ARGS[@]}"
    ms_print "$BUILD_DIR/massif.out" | less
    ;;
  *)
    echo "Unknown mode: $MODE"
    exit 1
    ;;
esac
