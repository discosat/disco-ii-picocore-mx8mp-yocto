SUMMARY = "Disco II Cortex-A53 Application System Manager"
SECTION = "DISCO"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=a70e5e582153f84e5c2c9e48db3428c9"

SRC_URI = "file://app-sys-manager/"
SRC_URI += "file://a53-app-sys-manager.service"
S = "${WORKDIR}/app-sys-manager"

inherit meson pkgconfig systemd

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "a53-app-sys-manager.service"
SYSTEMD_AUTO_ENABLE ??= "enable"

DEPENDS = "ninja-native gcc-cross-${TARGET_ARCH} pkgconfig-native libbsd libsocketcan can-utils"

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

    export CFLAGS="${TARGET_CC_ARCH} -fstack-protector-strong -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security --sysroot=${STAGING_DIR_TARGET}"
    export CXXFLAGS="${CFLAGS}"

    meson setup ${S} ${B} --cross-file ${WORKDIR}/cross.txt -Dprefix=${D}${prefix}
}

do_install() {
    ninja -C ${B} install
}

do_install:append() {
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/a53-app-sys-manager.service ${D}${systemd_system_unitdir}
}
