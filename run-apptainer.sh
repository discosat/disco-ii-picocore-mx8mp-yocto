#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SIF_IMAGE="${SCRIPT_DIR}/disco-yocto.sif"

if [ ! -f "$SIF_IMAGE" ]; then
    echo "Error: ${SIF_IMAGE} not found."
    echo ""
    echo "Build it first with:"
    echo "  apptainer build disco-yocto.sif disco-yocto.def"
    echo ""
    echo "Or with fakeroot (no sudo):"
    echo "  apptainer build --fakeroot disco-yocto.sif disco-yocto.def"
    exit 1
fi

mkdir -p "${SCRIPT_DIR}/.build"
mkdir -p "${SCRIPT_DIR}/deploy"

# Pass through BB_NUMBER_THREADS if set (limits parallel BitBake workers)
ENV_ARGS=""
if [ -n "$BB_NUMBER_THREADS" ]; then
    ENV_ARGS="--env BB_NUMBER_THREADS=${BB_NUMBER_THREADS}"
fi

echo "Starting Yocto build in Apptainer container..."
apptainer run \
    --bind "${SCRIPT_DIR}/.build:/build" \
    --bind "${SCRIPT_DIR}/custom:/custom" \
    --bind "${SCRIPT_DIR}/deploy:/deploy" \
    $ENV_ARGS \
    "$SIF_IMAGE"
