SUMMARY = "DISCO 2 Image processing pipeline"
SECTION = "pipeline"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "https://github.com/discosat/DIPP.git;branch=dtp;rev=a09c1bd98d90fe68ba69e165f1c0f8a7a5bfc790"

SRC_URI += " \
    https://github.com/spaceinventor/libcsp.git;destsuffix=git/lib/csp;name=libcsp;branch=master;rev=7ba36fb06ec21a5ade61672c2a55e3917619f58f \
    https://github.com/discosat/libparam.git;destsuffix=git/lib/param;name=libparam;branch=master;rev=768970c6320a455250ddd88903bbd9f58db81216 \
    https://github.com/discosat/libdtp.git;destsuffix=git/lib/dtp;name=libdtp;branch=master;rev=504e2cd3bdffeec7b092895c564b6af947a6008f \
"

S = "${WORKDIR}/git"

DEPENDS = "curl openssl libsocketcan can-utils zeromq libyaml meson-native ninja-native pkgconfig python3-pip-native elfutils libbsd protobuf-c libjxl opencv"
RDEPENDS:${PN} += "libcsp opencv libjxl"

inherit meson pkgconfig

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

do_configure() {
    export CC="${TARGET_PREFIX}gcc"
    export CXX="${TARGET_PREFIX}g++"
    export CPP="${TARGET_PREFIX}gcc -E"
    export LD="${TARGET_PREFIX}ld"
    export AR="${TARGET_PREFIX}ar"
    export AS="${TARGET_PREFIX}as"
    export NM="${TARGET_PREFIX}nm"
    export RANLIB="${TARGET_PREFIX}ranlib"
    export STRIP="${TARGET_PREFIX}strip"

    export CFLAGS="${TARGET_CC_ARCH} -fstack-protector-strong -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security --sysroot=${STAGING_DIR_TARGET} -I${WORKDIR}/git/include"
    export CXXFLAGS="${CFLAGS}"

    meson setup ${S} ${B} --cross-file ${WORKDIR}/cross.txt -Dprefix=${D}${prefix}
}

do_install() {
    ninja -C ${B} install
    install -d ${D}/usr/share/pipeline
    install -m 0644 ${WORKDIR}/git/external_modules/demosaic.so ${D}/usr/share/pipeline
    install -m 0644 ${WORKDIR}/git/external_modules/encoder.so ${D}/usr/share/pipeline  
}

FILES:${PN} += "${libdir}/*"
FILES:${PN} += "/usr/csp /usr/csp/csp_autoconfig.h"
FILES_${PN} += "/usr/share/pipeline"
