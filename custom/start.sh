#!/bin/bash
set -x

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

cd /build/yocto-fus/
source setup-environment build-fsimx8mp-fus-imx-wayland

##### Build the image #####

bitbake disco-fus-image
