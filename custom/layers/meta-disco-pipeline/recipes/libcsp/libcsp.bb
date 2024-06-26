DESCRIPTION = "Cubesat Space Protocol (CSP)"
SECTION = "pipeline"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "git://github.com/spaceinventor/libcsp.git;protocol=https;branch=master"
SRCREV = "6d0c670ac1c31b43083ab157cd2ed66a2ae8df35"

S = "${WORKDIR}/git"
B = "${S}/builddir"

inherit cmake

EXTRA_OECMAKE = "-DCMAKE_INSTALL_PREFIX=/usr"

do_configure() {
    cmake -B ${B} ${S} -GNinja -DCMAKE_SYSTEM_NAME=Linux ${EXTRA_OECMAKE}
}

do_compile() {
    ninja -C ${B}
}

do_install() {
    DESTDIR=${D} ninja -C ${B} install

    # Create symbolic link for shared library
    cd ${D}${libdir}
    if [ -f libcsp.so ]; then
        mv libcsp.so libcsp.so.1.0
        ln -s libcsp.so.1.0 libcsp.so
    fi
}

FILES_${PN} += "${prefix}/lib/* ${prefix}/include/*"