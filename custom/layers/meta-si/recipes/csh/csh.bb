DESCRIPTION = "CSP Shell (csh)"
SECTION = "csh"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://README.md;md5=fcfc09a5e736efc9442a7529f3aacbcc"



SRC_URI = "git://github.com/discosat/csh.git;protocol=https;branch=master;rev=cb7a17837ea941081d12f3de6196ac3a311923f2"

SRC_URI += " \
    git://github.com/spaceinventor/libcsp.git;protocol=https;destsuffix=git/lib/csp;name=libcsp;rev=6d0c670ac1c31b43083ab157cd2ed66a2ae8df35 \
    git://github.com/discosat/libparam.git;protocol=https;destsuffix=git/lib/param;name=libparam;branch=master;rev=768970c6320a455250ddd88903bbd9f58db81216 \
    git://github.com/spaceinventor/slash.git;protocol=https;destsuffix=git/lib/slash;name=slash;branch=master;rev=05c2b9efde0ac6181608f0232ece13a0e1c14f81 \
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
