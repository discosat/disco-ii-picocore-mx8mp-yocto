#!/bin/bash
set -e

##### Prepare the build environment with customizations #####

if [ "$(stat -c '%U:%G' /build)" != "yocto:yocto" ]; then
    sudo chown -R yocto:yocto /build
fi

if [ ! -d "/build/yocto-fus" ]; then 
    cp -rL /prep-build/disco-ii-yocto/* /build/
fi

# conf
cp /custom/conf/* /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/conf/

# layers
cp -rL /custom/layers/* /build/yocto-fus/sources/

# patches
cp /custom/patch/scons.bbclass /build/yocto-fus/sources/poky/meta/classes/scons.bbclass
cp /custom/patch/meta-disco-camera-opencv.bb /build/yocto-fus/sources/meta-disco-camera/recipes/opencv/opencv_4.1.0.bb
if [ -f "/build/yocto-fus/sources/meta-disco-camera/recipes/libcsp/libcsp.bb" ]; then rm /build/yocto-fus/sources/meta-disco-camera/recipes/libcsp/libcsp.bb; fi
if [ -f "/build/yocto-fus/sources/meta-disco-pipeline/recipes/libcsp/libcsp.bb" ]; then rm /build/yocto-fus/sources/meta-disco-pipeline/recipes/libcsp/libcsp.bb; fi

cd /build/yocto-fus/
source setup-environment build-fsimx8mp-fus-imx-wayland

##### Build the image #####

bitbake disco-fus-image

##### Prepare deploy volume #####

sudo cp -L /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/tmp/deploy/images/fsimx8mp/picocoremx8mp.dtb /deploy/
sudo cp -L /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/tmp/deploy/images/fsimx8mp/emmc-fsimx8mp.sysimg /deploy/
sudo cp -L /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/tmp/deploy/images/fsimx8mp/Image-fsimx8mp /deploy/
sudo cp -L /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/tmp/deploy/images/fsimx8mp/update.scr /deploy/

echo "System files ready to flash onto a PicoCore™MX8MP are now available in the deploy directory"
