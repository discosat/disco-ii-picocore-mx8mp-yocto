# Copyright 2020 F&S Elektronik Systeme GmbH

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI[md5sum] = "ce864e0f149993d41e9480bf4c2e8510"
SRC_URI += "file://0001-Add-option-arg-for-nand-device.patch"
SRC_URI[md5sum] = "b8282a2b78c1f40043f50e9333e07769"
SRC_URI += "file://0002-Correct-dtbs-env-to-use-different-DTs.patch"

BOOT_CONFIG_MACHINE_FS = "u-boot-${MACHINE}-${UBOOT_CONFIG}.${UBOOT_SUFFIX}"

do_compile() {
    compile_${SOC_FAMILY}
    # Copy TEE binary to SoC target folder to mkimage
    if ${DEPLOY_OPTEE}; then
        cp ${DEPLOY_DIR_IMAGE}/tee.bin                       ${BOOT_STAGING}
    fi
    # mkimage for i.MX8
    for target in ${IMXBOOT_TARGETS}; do
        if [ "$target" = "flash_linux_m4_no_v2x" ]; then
           # Special target build for i.MX 8DXL with V2X off
           bbnote "building ${SOC_TARGET} - ${REV_OPTION} V2X=NO ${target}"
           make SOC=${SOC_TARGET} DTB=${UBOOT_DTB_NAME} ${REV_OPTION} V2X=NO  flash_linux_m4
        else
           bbnote "building ${SOC_TARGET} - ${REV_OPTION} ${target}"
           make SOC=${SOC_TARGET} DTB=${UBOOT_DTB_NAME} ${REV_OPTION} ${target}
        fi
        if [ -e "${BOOT_STAGING}/flash.bin" ]; then
            cp ${BOOT_STAGING}/flash.bin ${S}/${BOOT_CONFIG_MACHINE_FS}
        fi
    done
}

# TODO: extract u-boot only for fsimx8mp because nboot support is not available now
extract_nb0_fsimx8mp() {
    dd  if=${BOOT_CONFIG_MACHINE_FS} of=uboot-${MACHINE}-${UBOOT_CONFIG}.nb0 bs=1K skip=352
}

do_install () {
    install -d ${D}/boot
    for target in ${IMXBOOT_TARGETS}; do
        install -m 0644 ${S}/${BOOT_CONFIG_MACHINE_FS} ${D}/boot/
    done
}

do_deploy() {
    deploy_${SOC_FAMILY}
    # copy tee.bin to deploy path
    if "${DEPLOY_OPTEE}"; then
        install -m 0644 ${DEPLOY_DIR_IMAGE}/tee.bin          ${DEPLOYDIR}/${BOOT_TOOLS}
    fi
    # copy the tool mkimage to deploy path and sc fw, dcd and uboot
    install -m 0644 ${DEPLOY_DIR_IMAGE}/${UBOOT_NAME}        ${DEPLOYDIR}/${BOOT_TOOLS}
    # copy makefile (soc.mak) for reference
    install -m 0644 ${BOOT_STAGING}/soc.mak                  ${DEPLOYDIR}/${BOOT_TOOLS}
    # copy the generated boot image to deploy path
    for target in ${IMXBOOT_TARGETS}; do
        # Use first "target" as IMAGE_IMXBOOT_TARGET
        if [ "$IMAGE_IMXBOOT_TARGET" = "" ]; then
            IMAGE_IMXBOOT_TARGET="$target"
            echo "Set boot target as $IMAGE_IMXBOOT_TARGET"
        fi
        install -m 0644 ${S}/${BOOT_CONFIG_MACHINE_FS} ${DEPLOYDIR}
    done
    cd ${DEPLOYDIR}
    ln -sf ${BOOT_CONFIG_MACHINE_FS} ${BOOT_NAME}
	extract_nb0_${MACHINE}
    cd -
}
