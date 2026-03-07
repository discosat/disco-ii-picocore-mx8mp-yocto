#!/bin/bash
set -e

##### Initial setup (mirrors runscript logic) #####

if [ ! -d "/build/yocto-fus" ]; then
    echo "First run: copying Yocto build tree to /build (this may take a while)..."
    cp -rL /prep-build/disco-ii-yocto/yocto-fus /build/yocto-fus
fi

# Pre-populate private repo mirror AFTER initial copy
DL_GIT2=/build/yocto-fus/downloads/git2
if [ -d "/mirrors" ] && [ ! -d "$DL_GIT2/github.com.discosat.upload_sat-client.git" ]; then
    echo "Pre-populating upload_sat-client mirror from bind-mount..."
    mkdir -p "$DL_GIT2"
    cp -r /mirrors/github.com.discosat.upload_sat-client.git "$DL_GIT2/"
fi

# Copy custom configs and layers
cp /custom/conf/* /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/conf/
cp -rL /custom/layers/* /build/yocto-fus/sources/

# Apply patches
cp /custom/patch/scons.bbclass /build/yocto-fus/sources/poky/meta/classes/scons.bbclass
sed -i '/RDEPENDS.*conditional.*vmlinux/d' \
    /build/yocto-fus/sources/poky/meta/classes/kernel.bbclass
[ -f "/build/yocto-fus/sources/meta-disco-camera/recipes/libcsp/libcsp.bb" ] && \
    rm /build/yocto-fus/sources/meta-disco-camera/recipes/libcsp/libcsp.bb
[ -f "/build/yocto-fus/sources/meta-disco-pipeline/recipes/libcsp/libcsp.bb" ] && \
    rm /build/yocto-fus/sources/meta-disco-pipeline/recipes/libcsp/libcsp.bb

##### Build #####

cd /build/yocto-fus
source /build/yocto-fus/setup-environment build-fsimx8mp-fus-imx-wayland

if [ -n "$BB_NUMBER_THREADS" ]; then
    export BB_NUMBER_THREADS
    echo "BB_NUMBER_THREADS=${BB_NUMBER_THREADS}"
fi
if [ -n "$PARALLEL_MAKE" ]; then
    export PARALLEL_MAKE
    echo "PARALLEL_MAKE=${PARALLEL_MAKE}"
fi

bitbake disco-fus-image

##### Deploy #####

cp -L /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/tmp/deploy/images/fsimx8mp/picocoremx8mp.dtb /deploy/
cp -L /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/tmp/deploy/images/fsimx8mp/emmc-fsimx8mp.sysimg /deploy/
cp -L /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/tmp/deploy/images/fsimx8mp/Image-fsimx8mp /deploy/
cp -L /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/tmp/deploy/images/fsimx8mp/update.scr /deploy/
cp -L /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/tmp/deploy/images/fsimx8mp/uboot-fsimx8mp.fs /deploy/
cp /build/yocto-fus/sources/meta-fus/recipes-bsp/u-boot/files/nboot-disco-mod.fs.gz /deploy/ \
    && gunzip -f /deploy/nboot-disco-mod.fs.gz \
    && mv /deploy/nboot-disco-mod.fs /deploy/nboot-fsimx8mp.fs

echo "System files ready to flash onto a PicoCoreMX8MP are now available in the deploy directory"
