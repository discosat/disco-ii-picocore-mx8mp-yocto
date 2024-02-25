DESCRIPTION = "CSP Shell (csh)"
SECTION = "csh"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://README.md;md5=7da34418d8afbbe639035a9f7f4053f3"

SRC_URI = "git://github.com/spaceinventor/csh.git;protocol=https;branch=master;rev=fe0fb1a6585810308c762f4ece2112f3e51fd611"

SRC_URI += " \
    git://github.com/spaceinventor/libcsp.git;protocol=https;destsuffix=git/lib/csp;name=libcsp;branch=master;rev=544635f292b7a15ea46b95cd2861102129c329e7 \
    git://github.com/spaceinventor/libparam.git;protocol=https;destsuffix=git/lib/param;name=libparam;branch=master;rev=fdf62e155a965df99a1012174677c6f2958a7e4f \
    git://github.com/spaceinventor/slash.git;protocol=https;destsuffix=git/lib/slash;name=slash;branch=master;rev=6084abc9a64edd72e2f4ad50914bcacb2194c4a5 \
    git://github.com/yaml/libyaml.git;protocol=https;destsuffix=git/lib/yaml;name=yaml;branch=master;rev=f8f760f7387d2cc56a2fc7b1be313a3bf3f7f58c \
"

S = "${WORKDIR}/git"

DEPENDS = "curl openssl libsocketcan can-utils zeromq libyaml meson-native ninja-native pkgconfig python3-pip-native elfutils libbsd"

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
}
