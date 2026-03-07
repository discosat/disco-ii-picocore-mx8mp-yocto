#!/bin/bash
#SBATCH --job-name=yocto-SOM2
#SBATCH --partition=acltr
#SBATCH --gres=gpu:1
#SBATCH --nodes=1
#SBATCH --cpus-per-task=28
#SBATCH --mem=64G
#SBATCH --time=08:00:00
#SBATCH --output=yocto-build-%j.log
#SBATCH --error=yocto-build-%j.log

set -e

###############################################################################
# DISCO-II Yocto Build via SLURM + Apptainer
#
# Requires acltr partition (--gres=gpu:1) for access to nodes with local SSD.
# The GPU is not used — the partition policy requires requesting one.
#
# Yocto parallelism knobs:
#   BB_NUMBER_THREADS  = how many BitBake tasks run in parallel
#   PARALLEL_MAKE      = -j flag for each task's make invocation
#
# Rule of thumb: ~2-4 GB RAM per thread.
###############################################################################

CPUS=${SLURM_CPUS_PER_TASK:-28}
BB_THREADS=$(( CPUS ))
MAKE_JOBS=$(( CPUS ))

PROJECT_DIR="$HOME/disco-ii-picocore-mx8mp-yocto"
SIF_IMAGE="${PROJECT_DIR}/disco-yocto.sif"

# Use /scratch for build artifacts — local SSD, vastly faster than NFS /home.
# BitBake refuses to run with TMPDIR on NFS.
if [ -d "/scratch/$USER" ] || mkdir -p "/scratch/$USER" 2>/dev/null; then
    BUILD_DIR="/scratch/$USER/yocto-build"
else
    echo "WARNING: /scratch/$USER not available, using /tmp (may run out of space)"
    BUILD_DIR="/tmp/$USER-yocto-build"
fi

DEPLOY_DIR="${PROJECT_DIR}/deploy"
MIRROR_DIR="${PROJECT_DIR}/.mirrors"

mkdir -p "$BUILD_DIR"
mkdir -p "$DEPLOY_DIR"

# Remove stale/incomplete build trees (e.g. from a previously failed job
# that created the yocto-fus dir but never populated it fully)
if [ -d "$BUILD_DIR/yocto-fus" ] && [ ! -d "$BUILD_DIR/yocto-fus/sources" ]; then
    echo "Removing stale incomplete build tree..."
    rm -rf "$BUILD_DIR/yocto-fus"
fi

if [ ! -f "$SIF_IMAGE" ]; then
    echo "Error: ${SIF_IMAGE} not found."
    echo "Build it first with:"
    echo "  sbatch --partition=acltr --gres=gpu:1 --cpus-per-task=4 --mem=16G --time=02:00:00 \\"
    echo "    --wrap='cd ${PROJECT_DIR} && apptainer build --fakeroot disco-yocto.sif disco-yocto.def'"
    exit 1
fi

echo "=== DISCO-II Yocto Build (flight-SOM2) ==="
echo "Node: $(hostname) | CPUs: ${CPUS} | Started: $(date)"
echo "BB: ${BB_THREADS} | Make: -j${MAKE_JOBS}"
echo "Build dir: ${BUILD_DIR}"

# Build bind-mount args
BIND_ARGS="--bind ${BUILD_DIR}:/build"
BIND_ARGS="${BIND_ARGS} --bind ${PROJECT_DIR}/custom:/custom"
BIND_ARGS="${BIND_ARGS} --bind ${DEPLOY_DIR}:/deploy"

# If we have a pre-populated mirror for private repos, bind it in
if [ -d "$MIRROR_DIR" ]; then
    BIND_ARGS="${BIND_ARGS} --bind ${MIRROR_DIR}:/mirrors"
fi

apptainer exec \
    ${BIND_ARGS} \
    --env BB_NUMBER_THREADS="${BB_THREADS}" \
    --env PARALLEL_MAKE="-j${MAKE_JOBS}" \
    "$SIF_IMAGE" \
    /bin/bash "${PROJECT_DIR}/build-wrapper.sh"

echo "[$(date)] Build complete. Artifacts in: ${DEPLOY_DIR}"
