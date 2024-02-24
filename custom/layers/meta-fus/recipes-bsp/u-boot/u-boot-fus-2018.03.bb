# Copyright (C) 2020 F&S Elektronik Systeme GmbH
# Released under the MIT license (see COPYING.MIT for the terms)

DESCRIPTION = "bootloader for F&S boards and modules"
require recipes-bsp/u-boot/u-boot.inc

PROVIDES += "u-boot"
DEPENDS:append = " python3 dtc-native bison-native"
RDEPENDS:${PN}:append = " fs-installscript"

LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://Licenses/README;md5=a2c678cfd4a4d97135585cad908541c6"
SRC_URI:append:mx8mm-nxp-bsp = " file://0002-Fix-compilation-errors-with-new-toolchain.patch"
UBOOT_INITIAL_ENV= ""

# SRC_URI and SRCREV are set in the bbappend file

S = "${WORKDIR}/git"
PV = "+git${SRCPV}"

# Set the u-boot environment variable "mode" to rw if it is not a read-only-rootfs
SRC_URI += '${@bb.utils.contains("IMAGE_FEATURES", "read-only-rootfs", "", "file://0001-Set-file-system-RW.patch",d)}'

S = "${WORKDIR}/git"
B = "${WORKDIR}/build"

UBOOT_MAKE_TARGET = "all"
COMPATIBLE_MACHINE = "(mx6|vf60|mx7ulp|mx8)"

# Necessary ???
# FIXME: Allow linking of 'tools' binaries with native libraries
#        used for generating the boot logo and other tools used
#        during the build process.
#EXTRA_OEMAKE += 'HOSTCC="${BUILD_CC} ${BUILD_CPPFLAGS}" \
#                 HOSTLDFLAGS="${BUILD_LDFLAGS}" \
#                 HOSTSTRIP=true'

do_deploy:append:mx8m-nxp-bsp() {
	install -m 644 ${B}/${UBOOT_WIC_BINARY} ${DEPLOY_DIR_IMAGE}/
}

PACKAGE_ARCH = "${MACHINE_ARCH}"
