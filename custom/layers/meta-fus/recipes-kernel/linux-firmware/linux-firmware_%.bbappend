# Copyright 2017-2020 F&S Elektronik Systeme

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# Firmware for atmel mxt touches
SRC_URI += " file://mxt224.cfg \
		   file://mxt224e_v2.cfg \
		   file://mxt1066.cfg \
		   file://mxt336u-gloves.cfg \
		   file://mxt336u-extra-glass.cfg \
		   file://wifi_mod_para.conf	"

do_install:append () {
    install -d ${D}${nonarch_base_libdir}/firmware/atmel
    # Install Atmel MXT Touch firmware
    install -m 0644 ${WORKDIR}/mxt224.cfg ${D}${nonarch_base_libdir}/firmware/atmel
    install -m 0644 ${WORKDIR}/mxt224e_v2.cfg ${D}${nonarch_base_libdir}/firmware/atmel
    install -m 0644 ${WORKDIR}/mxt1066.cfg ${D}${nonarch_base_libdir}/firmware/atmel
    install -m 0644 ${WORKDIR}/mxt336u-gloves.cfg ${D}${nonarch_base_libdir}/firmware/atmel
    install -m 0644 ${WORKDIR}/mxt336u-extra-glass.cfg ${D}${nonarch_base_libdir}/firmware/atmel
    # Create softlink for sdsd8997_combo_v4
    ln -sf sdsd8997_combo_v4.bin ${D}${nonarch_base_libdir}/firmware/mrvl/sd8997_uapsta.bin

    # Install NXP Connectivity
    install -d ${D}${nonarch_base_libdir}/firmware/nxp
    install -m 0644 ${WORKDIR}/wifi_mod_para.conf    ${D}${nonarch_base_libdir}/firmware/nxp
}

PACKAGES =+ " ${PN}-atmel-mxt "

FILES:${PN}-atmel-mxt = " \
       ${nonarch_base_libdir}/firmware/atmel/mxt224.cfg \
       ${nonarch_base_libdir}/firmware/atmel/mxt224e_v2.cfg \
       ${nonarch_base_libdir}/firmware/atmel/mxt1066.cfg \
       ${nonarch_base_libdir}/firmware/atmel/mxt336u-gloves.cfg \
       ${nonarch_base_libdir}/firmware/atmel/mxt336u-extra-glass.cfg \
"
