require fsimx-mcore-demos-2020.03.1.inc

LICENSE = "MIT & BSD-3-Clause"
SRC_URI += " file://COPYING"

# Make sure to check the licenses of the used components when adding an other image
LIC_FILES_CHKSUM = "file://${WORKDIR}/COPYING;md5=0640397c4f5c71dbd4e4179d634a62b2"

COMPATIBLE_MACHINE = "(mx7ulp)"
