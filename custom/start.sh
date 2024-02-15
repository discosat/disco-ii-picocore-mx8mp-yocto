#!/bin/bash
set -x

if [ "$(stat -c '%U:%G' /build)" != "yocto:yocto" ]; then
    sudo chown -R yocto:yocto /build
fi

if [ ! -d "/build/yocto-fus" ]; then 
    cp -r /prep-build/disco-ii-yocto/* /build/
fi

mkdir -p /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/conf
cp /custom/conf/* /build/yocto-fus/build-fsimx8mp-fus-imx-wayland/conf/

cd /build/yocto-fus/
source setup-environment build-fsimx8mp-fus-imx-wayland
bitbake fus-image-std
