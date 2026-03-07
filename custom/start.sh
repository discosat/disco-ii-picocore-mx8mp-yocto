#!/bin/bash
set -e

##### Prepare the build environment with customizations #####

if [ "$(stat -c '%U:%G' /build)" != "yocto:yocto" ]; then
    sudo chown -R yocto:yocto /build
fi

if [ ! -d "/build/yocto-fus" ]; then 
    cp -rL /prep-build/disco-ii-yocto/yocto-fus /build/yocto-fus
fi

# conf
cp /custom/conf/* /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/conf/

# layers
cp -rL /custom/layers/* /build/yocto-fus/sources/

# patches
cp /custom/patch/scons.bbclass /build/yocto-fus/sources/poky/meta/classes/scons.bbclass
# Fix kernel.bbclass: inline Python ${@oe.utils.conditional(...)} for vmlinux
# is not evaluated by this BitBake version, producing broken RDEPENDS.
# Since KERNEL_IMAGETYPE="Image" (not vmlinux), this dependency is never needed.
sed -i '/RDEPENDS.*conditional.*vmlinux/d' \
    /build/yocto-fus/sources/poky/meta/classes/kernel.bbclass
if [ -f "/build/yocto-fus/sources/meta-disco-camera/recipes/libcsp/libcsp.bb" ]; then rm /build/yocto-fus/sources/meta-disco-camera/recipes/libcsp/libcsp.bb; fi
if [ -f "/build/yocto-fus/sources/meta-disco-pipeline/recipes/libcsp/libcsp.bb" ]; then rm /build/yocto-fus/sources/meta-disco-pipeline/recipes/libcsp/libcsp.bb; fi

cd /build/yocto-fus/
source setup-environment build-fsimx8mp-fus-imx-wayland

##### Build the image #####

# Honor SLURM/env parallelism overrides if set
if [ -n "$BB_NUMBER_THREADS" ]; then
    export BB_NUMBER_THREADS
    echo "BB_NUMBER_THREADS=${BB_NUMBER_THREADS}"
fi
if [ -n "$PARALLEL_MAKE" ]; then
    export PARALLEL_MAKE
    echo "PARALLEL_MAKE=${PARALLEL_MAKE}"
fi

bitbake disco-fus-image

##### Prepare deploy volume #####

sudo cp -L /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/tmp/deploy/images/fsimx8mp/picocoremx8mp.dtb /deploy/
sudo cp -L /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/tmp/deploy/images/fsimx8mp/emmc-fsimx8mp.sysimg /deploy/
sudo cp -L /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/tmp/deploy/images/fsimx8mp/Image-fsimx8mp /deploy/
sudo cp -L /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/tmp/deploy/images/fsimx8mp/update.scr /deploy/
sudo cp -L /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/tmp/deploy/images/fsimx8mp/uboot-fsimx8mp.fs /deploy/
sudo cp /build/yocto-fus/sources/meta-fus/recipes-bsp/u-boot/files/nboot-disco-mod.fs.gz /deploy/ && sudo gunzip /deploy/nboot-disco-mod.fs.gz && sudo mv /deploy/nboot-disco-mod.fs /deploy/nboot-fsimx8mp.fs

echo "System files ready to flash onto a PicoCore™MX8MP are now available in the deploy directory"
