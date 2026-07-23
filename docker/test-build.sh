#!/usr/bin/env bash

set -e

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

cd "$ROOT_DIR"

SYSTEMS=(
    debian
    ubuntu
    fedora
    arch
    alpine
)

PROJECT_NAME="miniexiv"

echo "================================="
echo "MiniExiv Docker build test"
echo "================================="


for system in "${SYSTEMS[@]}"
do
    IMAGE="${PROJECT_NAME}-${system}"

    echo
    echo "================================="
    echo "Building: ${system}"
    echo "Image: ${IMAGE}"
    echo "================================="

    docker build \
        -f docker/Dockerfile.${system} \
        -t ${IMAGE} \
        .

    echo "✓ ${system} OK"
done


echo
echo "================================="
echo "All builds passed!"
echo "================================="