# Copyright 2017-2020 NXP
# Released under the MIT license (see COPYING.MIT for the terms)

SUMMARY = "i.MX M4/M7 core Demo images"
SECTION = "app"
LICENSE = "Proprietary"

inherit deploy

SOC        ?= "INVALID"
SOC_mx7ulp  = "imx7ulp"

MCORE_TYPE  ?= "m4"

IMX_PACKAGE_NAME = "${SOC}-${MCORE_TYPE}-demo-${PV}"

SRC_URI = "file://power_mode_switch.img"
SCR = "SCR-${SOC}-${MCORE_TYPE}-demo.txt"
SRC_URI += "file://${SCR}"

DEPLOY_FILE_EXT       ?= "bin"
DEPLOY_FILE_EXT_mx7ulp = "img"

do_deploy () {
   # Install the demo binaries
   install -m 0644 ${WORKDIR}/power_mode_switch.img ${DEPLOYDIR}/
}

addtask deploy after do_install

PACKAGE_ARCH = "${MACHINE_SOCARCH}"
