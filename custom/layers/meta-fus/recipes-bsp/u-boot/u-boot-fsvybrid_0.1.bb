# Copyright (C) 2014 F&S Elektronik Systeme GmbH
# Released under the MIT license (see COPYING.MIT for the terms)

DESCRIPTION = "bootloader for F&S boards and modules based on Freescale Vybrid"
require recipes-bsp/u-boot/u-boot.inc

PROVIDES += "u-boot"

LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://COPYING;md5=1707d6db1d42237583f50183a5651ecb"

SRC_URI = "file://u-boot-2012.07-f+s-Y0.1.tar.bz2"
S = "${WORKDIR}/u-boot-2012.07-f+s-Y0.1"
PV = "0.1"

UBOOT_MAKE_TARGET = "all"
PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(vf60)"
UBOOT_BINARY = "uboot.${UBOOT_SUFFIX}"
