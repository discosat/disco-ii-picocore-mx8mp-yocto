DESCRIPTON = "F&S Install script"
LICENSE = "MIT"

inherit deploy

#default license in /mnt/yocto/fsl-release-bsp-l4.1.15_2.0.0-ga/sources/poky/LICENSE
#LIC_FILES_CHKSUM = "file://${COREBASE}/LICENSE;md5=4d92cd373abda3937c2bc47fbc49d690"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
PR = "r0"
ARCH ?= "arm"
ARCH_armv7a = "arm"
ARCH_aarch64 = "arm64"

SRC_URI = " \
           file://install.txt \
           file://update-uboot-nboot-sysimg.txt\
		  "

DEPENDS:append = " u-boot-mkimage-native"

do_compile() {
    uboot-mkimage -A ${ARCH} -O u-boot -T script -C none -n "F&S install script" -d ${WORKDIR}/install.txt ${WORKDIR}/install.scr
	uboot-mkimage -A ${ARCH} -O u-boot -T script -C none -n "F&S install script" -d ${WORKDIR}/update-uboot-nboot-sysimg.txt ${WORKDIR}/update-uboot-nboot-sysimg.scr
}

do_deploy() {
	install -d ${DEPLOYDIR}/
	install -m 0777 ${WORKDIR}/install.scr ${DEPLOYDIR}
	install -m 0777 ${WORKDIR}/update-uboot-nboot-sysimg.scr ${DEPLOYDIR}
}
addtask deploy before do_build after do_compile

FILES:${PN} = "install.scr update-uboot-nboot-sysimg.scr"
