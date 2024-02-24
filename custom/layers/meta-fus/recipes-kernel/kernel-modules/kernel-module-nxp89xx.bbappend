FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

FILES:${PN} = "\
	${sysconfdir}/modprobe.d/mxm-wifiex.conf \
"

SRC_URI += " file://001-Create-module.aliases-for-the-sdio-devices.patch \
			 file://mxm-wifiex.conf"

do_install:append () {
    install -d ${D}${sysconfdir}/modprobe.d/
    install -m 0755 ${WORKDIR}/mxm-wifiex.conf ${D}${sysconfdir}/modprobe.d/mxm-wifiex.conf
}
