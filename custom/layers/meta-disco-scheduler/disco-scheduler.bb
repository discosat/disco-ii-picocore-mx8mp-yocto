DESCRIPTION = "Disco II Cortex-M7 Scheduler"
SECTION = "DISCO"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://COPYING-BSD-3;md5=0858ec9c7a80c4a2cf16e4f825a2cc91"

SRC_URI = "https://github.com/discosat/disco-ii-cortex-m7-scheduler.git;branch=master \
           https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2;name=gcc-arm-none-eabi \
           https://github.com/spaceinventor/libcsp.git;destsuffix=git/lib/csp;name=libcsp;branch=master \
           https://github.com/discosat/libparam.git;destsuffix=git/lib/param;name=libparam;branch=master \
           https://github.com/discosat/csp_proc.git;destsuffix=git/lib/csp_proc;name=csp_proc;branch=main"

SRCREV = "33e5183c7641168f063771bd60a10a89907ea78d"
SRCREV_libcsp = "7ba36fb06ec21a5ade61672c2a55e3917619f58f"
SRCREV_libparam = "768970c6320a455250ddd88903bbd9f58db81216"
SRCREV_csp_proc = "78d4b0de7921b4e155452103b839c701c64c17ca"

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
