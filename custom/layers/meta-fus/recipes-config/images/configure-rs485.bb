SUMMARY = "PicoCoreMX8MP RS485 configuration"
SECTION = "rs485"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://configure-rs485.c;md5=8670485c9cd63e52a91606f611000f60"

SRC_URI = "file://configure-rs485.c"

S = "${WORKDIR}"

do_compile() {
    ${CC} ${LDFLAGS} configure-rs485.c -o configure-rs485
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${S}/configure-rs485 ${D}${bindir}
}
