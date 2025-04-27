SUMMARY = "DISCO 2 Camera control software"
SECTION = "camera"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "git://github.com/discosat/DiscoCameraController.git;branch=main;rev=e1125c1c9e5f7760fb8c623c5575ecdd455da2c7;protocol=https"

SRC_URI += " \
    git://github.com/spaceinventor/libcsp.git;protocol=https;destsuffix=git/lib/csp;name=libcsp;branch=master;rev=7ba36fb06ec21a5ade61672c2a55e3917619f58f \
    git://github.com/spaceinventor/libparam.git;protocol=https;destsuffix=git/lib/param;name=libparam;branch=master;rev=fdf62e155a965df99a1012174677c6f2958a7e4f \
"

S = "${WORKDIR}/git"

inherit meson pkgconfig logging

RDEPENDS:${PN} += "zeromq can-utils libcsp libbsd pkgconfig opencv"
DEPENDS += "curl openssl libsocketcan can-utils zeromq libyaml meson-native ninja-native pkgconfig python3-pip-native elfutils libbsd glibc opencv protobuf"

PROVIDES += " libVmbCPP"
RPROVIDES_${PN} += " libVmbCPP.so()(64bit)"

SOLIBS = ".so"
FILES_SOLIBSDEV = ""

do_configure:prepend() {
    cat > ${WORKDIR}/cross.txt <<EOF
[binaries]
c = '${TARGET_PREFIX}gcc'
cpp = '${TARGET_PREFIX}g++'
ar = '${TARGET_PREFIX}ar'
strip = '${TARGET_PREFIX}strip'
pkgconfig = 'pkg-config'
[properties]
needs_exe_wrapper = true
EOF
}

do_install(){
    install -d ${D}${libdir}
    install -d ${D}${bindir}
    install -d ${D}${sysconfdir}/lib
    install -d 644 ${D}${sysconfdir}/profile.d
    install -d ${D}${sysconfdir}/udev
    install -d ${D}${sysconfdir}/udev/rules.d

    install -m 0644 ${WORKDIR}/git/lib/VimbaX_2023-4-ARM64/api/lib/*.so ${D}${libdir}
    install -m 0644 ${WORKDIR}/git/lib/VimbaX_2023-4-ARM64/api/lib/GenICam/*.so ${D}${libdir}
    install ${WORKDIR}/build/Disco2CameraControl ${D}${bindir}
    install ${WORKDIR}/build/receiver ${D}${bindir}

    cp -r ${WORKDIR}/git/lib/VimbaX_2023-4-ARM64 ${D}${sysconfdir}/lib/VimbaX_2023-4-ARM64
    chmod 447 ${D}${sysconfdir}/lib/VimbaX_2023-4-ARM64/cti/VimbaUSBTL_Install.sh

    install -d ${D}/home/root
    install -m 0777 ${WORKDIR}/git/experiment.sh ${D}/home/root/experiment_camera.sh

    printf "SUBSYSTEM==\"usb\", ACTION==\"add\", ATTRS{idVendor}==\"1ab2\", ATTRS{idProduct}==\"0001\", MODE=\"0666\"\nSUBSYSTEM==\"usb\", ACTION==\"add\", ATTRS{idVendor}==\"1ab2\", ATTRS{idProduct}==\"ff01\", MODE=\"0666\"" >> ${D}${sysconfdir}/udev/rules.d/99-AVTUSBTL.rules

    printf "#!/bin/sh\n\nexport GENICAM_GENTL64_PATH=\$GENICAM_GENTL64_PATH:\"${sysconfdir}/lib/VimbaX_2023-4-ARM64/cti\"" >> ${D}${sysconfdir}/profile.d/AVTUSBTL_64bit.sh
    chmod 777 ${D}${sysconfdir}/profile.d/AVTUSBTL_64bit.sh
}

INHIBIT_PACKAGE_STRIP = "1"
INSANE_SKIP:${PN} += "already-stripped"

FILES:${PN} += "${libdir}/libVmbCPP.so"
FILES:${PN} += "${libdir}/libVmbC.so"
FILES:${PN} += "/home/root/experiment_camera.sh"
FILES:${PN} += "/usr/csp /usr/csp/csp_autoconfig.h"
FILES:${PN} += "${sysconfdir}/udev"

do_package_qa[noexec] = "1"
EXCLUDE_FROM_SHLIBS = "1"
