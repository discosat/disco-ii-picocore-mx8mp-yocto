# Copyright (C) 2020 F&S Elektronik Systeme GmbH
# Released under the MIT license (see COPYING.MIT for the terms)

DESCRIPTION = "bootloader for F&S boards and modules"
require recipes-bsp/u-boot/u-boot.inc

PROVIDES += "u-boot"
DEPENDS:append = " python3 dtc-native bison-native"
RDEPENDS:${PN}:append = " fs-installscript"

LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://Licenses/README;md5=5a7450c57ffe5ae63fd732446b988025"

# SRC_URI and SRCREV are set in the bbappend file

S = "${WORKDIR}/git"
PV = "+git${SRCPV}"

SCMVERSION ??= "y"
LOCALVERSION ??= "-F+S"

UBOOT_LOCALVERSION = "${LOCALVERSION}"

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

do_compile:prepend() {
	if [ "${SCMVERSION}" = "y" ]; then
		# Add GIT revision to the local version
		head=`cd ${S} ; git tag --points-at HEAD 2> /dev/null`
		sep="-"
		if [ -z "${head}" ]; then
			head=`cd ${S} ; git rev-parse --verify --short HEAD 2> /dev/null`
			sep="+g"
		fi
			printf "%s%s%s" "${UBOOT_LOCALVERSION}" $sep $head > ${S}/.scmversion
			printf "%s%s%s" "${UBOOT_LOCALVERSION}" $sep $head > ${B}/.scmversion
	else
		printf "%s" "${UBOOT_LOCALVERSION}" > ${S}/.scmversion
		printf "%s" "${UBOOT_LOCALVERSION}" > ${B}/.scmversion
	fi
}

PACKAGE_ARCH = "${MACHINE_ARCH}"
