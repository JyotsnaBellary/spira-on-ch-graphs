#!/usr/bin/env bash

# run once.//
# chmod +x build.sh

set -e  # exit if any command fails

CMD="$1"

if [ "$CMD" = "build" ]; then
    echo "Building project..."
    cmake --build build -j

    echo "Running sssp_cli..."
    ./build/sssp_cli

elif [ "$CMD" = "run" ]; then
    echo "Running sssp_cli..."
    ./build/sssp_cli

else
    echo "Usage: $0 {build|run}"
    exit 1
fi
