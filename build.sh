
#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./build.sh [Debug|Release] [run] [-- app args...]
# Examples:
#   ./build.sh
#   ./build.sh Release run
#   ./build.sh Debug run -- --input foo

BUILD_TYPE="${1:-Debug}"
MODE="${2:-}"

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build/$BUILD_TYPE"
APP="$BUILD_DIR/spira"

echo "[spira] Build dir: $BUILD_DIR (type=$BUILD_TYPE)"
mkdir -p "$BUILD_DIR"

# Optional KaHIP hints (set as env vars before calling this script)
EXTRA_CMAKE=()
[[ -n "${KAHIP_NODE_ORDERING:-}" ]] && EXTRA_CMAKE+=("-DKAHIP_NODE_ORDERING=${KAHIP_NODE_ORDERING}")
[[ -n "${KAHIP_HOME:-}"          ]] && EXTRA_CMAKE+=("-DKAHIP_HOME=${KAHIP_HOME}")

# Parse app args after '--'
APP_ARGS=()
for i in $(seq 1 $#); do
  if [[ "${!i}" == "--" ]]; then
    shift $i; APP_ARGS=("$@"); break
  fi
done

# Configure and build
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "${EXTRA_CMAKE[@]}"
cmake --build "$BUILD_DIR" -j$(nproc)

# Run if requested
if [[ "$MODE" == "run" ]]; then
  echo "[spira] Running..."
  exec "$APP" "${APP_ARGS[@]}"
fi
