# Copyright (C) 2014 F&S Elektronik Systeme GmbH
# Released under the GPLv2 license
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

inherit kernel

SUMMARY = "Linux Kernel with added drivers and board support for F&S Vybrid-based boards and modules"

# Revision of 3.0.15_vybrid branch
SRC_URI = "file://linux-3.0.15-fsvybrid-Y0.1.tar.bz2"
S = "${WORKDIR}/linux-3.0.15-fsvybrid-Y0.1"
PV = "0.1"

FSCONFIG = "fsvybrid_defconfig"

COMPATIBLE_MACHINE = "(vf60)"

kernel_do_configure:prepend() {
	oe_runmake ${FSCONFIG}
}
