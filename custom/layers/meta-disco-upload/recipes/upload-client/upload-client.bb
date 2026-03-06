DESCRIPTION = "DISCO-2 DTP Upload Client"
SECTION = "upload-client"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "https://github.com/discosat/upload_sat-client.git;branch=flight;rev=21c1b6522f987d988157b53a4c249ab4debaff60"

SRC_URI += " \
    https://github.com/spaceinventor/libcsp.git;destsuffix=git/lib/csp;name=libcsp;branch=master;rev=8cc13c663c6db1d333bd1af6546d1f7fc2599770 \
    https://github.com/discosat/libparam.git;destsuffix=git/lib/param;name=libparam;branch=master;rev=768970c6320a455250ddd88903bbd9f58db81216 \
    https://github.com/discosat/libdtp.git;destsuffix=git/lib/dtp;name=libdtp;branch=master;rev=504e2cd3bdffeec7b092895c564b6af947a6008f \
    https://github.com/spaceinventor/slash.git;destsuffix=git/lib/slash;name=slash;branch=master;rev=7b0c33b39d8b73c861efd1ddbcd10c4fe69f2308 \
"

S = "${WORKDIR}/git"

DEPENDS = "curl openssl libsocketcan can-utils zeromq meson-native ninja-native pkgconfig python3-pip-native elfutils libbsd protobuf-c"
RDEPENDS:${PN} += "libcsp"

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

    export CFLAGS="${TARGET_CC_ARCH} -fstack-protector-strong -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security --sysroot=${STAGING_DIR_TARGET} -I${WORKDIR}/git/src/include"
    export CXXFLAGS="${CFLAGS}"

    meson setup ${S} ${B} --cross-file ${WORKDIR}/cross.txt -Dprefix=${D}${prefix}
}

do_install() {
    ninja -C ${B} install
}
