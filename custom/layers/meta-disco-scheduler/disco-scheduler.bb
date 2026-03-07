DESCRIPTION = "Disco II Cortex-M7 Scheduler"
SECTION = "DISCO"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://COPYING-BSD-3;md5=0858ec9c7a80c4a2cf16e4f825a2cc91"

SRC_URI = "git://github.com/discosat/disco-ii-cortex-m7-scheduler.git;nobranch=1;protocol=https \
           https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2;name=gcc-arm-none-eabi \
           git://github.com/spaceinventor/libcsp.git;destsuffix=git/lib/csp;name=libcsp;nobranch=1;protocol=https \
           git://github.com/spaceinventor/libparam.git;destsuffix=git/lib/param;name=libparam;nobranch=1;protocol=https \
           git://github.com/discosat/csp_proc.git;destsuffix=git/lib/csp_proc;name=csp_proc;nobranch=1;protocol=https"

SRCREV = "2aad62d76a3e7de452de19fb3ed4450abaa2ea95"
SRCREV_libcsp = "544635f292b7a15ea46b95cd2861102129c329e7"
SRCREV_libparam = "fdf62e155a965df99a1012174677c6f2958a7e4f"
SRCREV_csp_proc = "1cd64b7040c96b34c0db62052b7fd87d186ffa09"

SRC_URI[gcc-arm-none-eabi.sha256sum] = "97dbb4f019ad1650b732faffcc881689cedc14e2b7ee863d390e0a41ef16c9a3"

DEPENDS = "curl tar bzip2 bash make cmake-native patch ca-certificates"

S = "${WORKDIR}/git"

do_compile() {
    export PATH=$PATH:${STAGING_BINDIR_NATIVE}
    export ARMGCC_DIR=${WORKDIR}/gcc-arm-none-eabi-10.3-2021.10
    export REPO_ROOT=${S}
    cd ${REPO_ROOT}/fsimx8mp-m7-sdk
    echo -e "6\nd" | ./prepare.sh
    sed -i 's|/disco-ii-cortex-m7-scheduler|'${S}'|g' ${S}/src/armgcc/CMakeLists.txt
    sed -i 's|/disco-ii-cortex-m7-scheduler|'${S}'|g' ${S}/src/armgcc/build.sh
    sed -i 's|/usr/local/arm/gcc-arm-none-eabi-10.3-2021.10/arm-none-eabi/include/machine|'${ARMGCC_DIR}'/arm-none-eabi/include/machine|g' ${S}/src/armgcc/CMakeLists.txt
    sed -i '/cp -r \/tmp-src\/\* ${REPO_ROOT}\/src/d' ${S}/src/armgcc/build-service-start.sh
    mkdir -p ${S}/bin
    chmod +x ${S}/src/armgcc/build-service-start.sh
    ${S}/src/armgcc/build-service-start.sh
}

do_install() {
    install -d ${D}/home/root
    install -m 0755 ${S}/bin/disco_scheduler.bin ${D}/home/root/disco_scheduler.bin
}

FILES:${PN} += "/home/root/disco_scheduler.bin"
