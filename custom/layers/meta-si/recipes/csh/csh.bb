DESCRIPTION = "CSP Shell (csh)"
SECTION = "csh"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://README.md;md5=6d605ad2c2fa2e72d8ad3a27b647ebcd"

SRC_URI = "https://github.com/spaceinventor/csh.git;branch=master;rev=6cc787e6b8d1fb17b82bb4810e10d42770445f20"

SRC_URI += " \
    https://github.com/spaceinventor/libcsp.git;destsuffix=git/lib/csp;name=libcsp;branch=master;rev=7ba36fb06ec21a5ade61672c2a55e3917619f58f \
    https://github.com/discosat/libparam.git;destsuffix=git/lib/param;name=libparam;branch=master;rev=768970c6320a455250ddd88903bbd9f58db81216 \
    https://github.com/spaceinventor/slash.git;destsuffix=git/lib/slash;name=slash;branch=master;rev=8858d39be041d2de4585dfee4dcd59b31eea7d1d \
    https://github.com/yaml/libyaml.git;destsuffix=git/lib/yaml;name=yaml;branch=master;rev=840b65c40675e2d06bf40405ad3f12dec7f35923 \
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
