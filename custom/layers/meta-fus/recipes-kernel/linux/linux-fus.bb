# Copyright (C) 2022 F&S Elektronik Systeme GmbH
# Released under the GPLv2 license

LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

inherit kernel

SUMMARY = "Linux Kernel for F&S i.MX6/7/8-based boards and modules"

DEPENDS += "lzop-native bc-native"

COMPATIBLE_MACHINE = "(mx6|mx7|mx8)"

# SRC_URI and SRCREV are set in the bbappend file

S = "${WORKDIR}/git"
PV = "+git${SRCPV}"

# We need to pass it as param since kernel might support more then one
# machine, with different entry points
KERNEL_EXTRA_ARGS += "LOADADDR=${UBOOT_ENTRYPOINT}"

KBUILD_DEFCONFIG:mx6-nxp-bsp = "fsimx6_defconfig"
KBUILD_DEFCONFIG:mx6sx-nxp-bsp = "fsimx6sx_defconfig"
KBUILD_DEFCONFIG:mx6ul-nxp-bsp = "fsimx6ul_defconfig"
KBUILD_DEFCONFIG:mx7ulp-nxp-bsp = "fsimx7ulp_defconfig"

KBUILD_DEFCONFIG:mx8mm-nxp-bsp = "fsimx8mm_defconfig"
KBUILD_DEFCONFIG:mx8mn-nxp-bsp = "fsimx8mn_defconfig"
KBUILD_DEFCONFIG:mx8mp-nxp-bsp = "fsimx8mp_defconfig"

# Prevent the galcore-module from beeing build, because it is already
# included in the F&S-Linux-Kernel as a build-in
RPROVIDES:kernel-image += "kernel-module-imx-gpu-viv"

do_extraunpack () {
	mv ${WORKDIR}/linux-fus/* ${S}/
}


kernel_do_configure:prepend() {
	install -m 0644 ${S}/arch/${ARCH}/configs/${KBUILD_DEFCONFIG} ${WORKDIR}/defconfig
}

kernel_do_configure:append() {
	rm -f ${B}/.scmversion ${S}/.scmversion
}

# Rename kernel image to be confirmed with naming convention
# for iMX8M based boards
kernel_do_deploy:append:mx8m-nxp-bsp() {
	# Set soft link to the kernel image and remove not needed links.
    # Use conform naming to documentation.
	cd ${DEPLOYDIR}
	ln -sf Image-${KERNEL_IMAGE_NAME}.bin Image-${MACHINE}
	rm -f Image-${MACHINE}.bin
	cd -
}
