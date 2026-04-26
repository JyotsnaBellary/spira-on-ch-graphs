#!/bin/bash
set -e

USER="bellary"
HOST="b31.algo.uni-konstanz.de"

LOCAL_DIR="/mnt/c/Users/Jyotsna Bellary/UniKonstanz/Master Project and Thesis/spira-on-ch/"
REMOTE_DIR="~/master-thesis/spira-on-ch"

echo "Syncing project to server..."

ssh "$USER@$HOST" "mkdir -p '$REMOTE_DIR'"

rsync -av --delete \
  "$LOCAL_DIR" \
  "$USER@$HOST:$REMOTE_DIR"

echo "Starting build on server..."

ssh "$USER@$HOST" "
    cd $REMOTE_DIR &&
    chmod +x build.sh &&
    nohup ./build.sh build > output.log 2>&1 &
"

echo "Tailing logs..."

ssh -t "$USER@$HOST" "
    cd $REMOTE_DIR &&
    tail -f output.log
"