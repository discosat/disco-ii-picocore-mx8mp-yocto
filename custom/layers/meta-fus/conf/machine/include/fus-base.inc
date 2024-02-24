# Common settings for all F&S boards

UBOOT_ENTRYPOINT:mx6-generic-bsp    = "0x10800000"
UBOOT_ENTRYPOINT:mx6sx-generic-bsp  = "0x80800000"
UBOOT_ENTRYPOINT:mx6ul-generic-bsp  = "0x80800000"
UBOOT_ENTRYPOINT:mx7ulp-generic-bsp = "0x60008000"

UBOOT_ENTRYPOINT:mx8mm-generic-bsp  = "0x40008000"
UBOOT_ENTRYPOINT:mx8mn-generic-bsp  = "0x40008000"
UBOOT_ENTRYPOINT:mx8mp-generic-bsp  = "0x40008000"

PREFERRED_PROVIDER_virtual/kernel:mx6-nxp-bsp    ??= "linux-fus"
PREFERRED_PROVIDER_virtual/kernel:mx6sx-nxp-bsp  ??= "linux-fus"
PREFERRED_PROVIDER_virtual/kernel:mx6ul-nxp-bsp  ??= "linux-fus"
PREFERRED_PROVIDER_virtual/kernel:mx7ulp-nxp-bsp ??= "linux-fus"

PREFERRED_PROVIDER_virtual/kernel:mx8mm-nxp-bsp ??= "linux-fus"
PREFERRED_PROVIDER_virtual/kernel:mx8mn-nxp-bsp ??= "linux-fus"
PREFERRED_PROVIDER_virtual/kernel:mx8mp-nxp-bsp ??= "linux-fus"

PREFERRED_PROVIDER_u-boot = "u-boot-fus"
PREFERRED_PROVIDER_virtual/bootloader:mx6-nxp-bsp    ??= "u-boot-fus"
PREFERRED_PROVIDER_virtual/bootloader:mx6sx-nxp-bsp  ??= "u-boot-fus"
PREFERRED_PROVIDER_virtual/bootloader:mx6ul-nxp-bsp  ??= "u-boot-fus"
PREFERRED_PROVIDER_virtual/bootloader:mx7ulp-nxp-bsp ??= "u-boot-fus"

PREFERRED_PROVIDER_virtual/bootloader:mx8mm-nxp-bsp  ??= "u-boot-fus"
PREFERRED_PROVIDER_virtual/bootloader:mx8mn-nxp-bsp  ??= "u-boot-fus"
PREFERRED_PROVIDER_virtual/bootloader:mx8mp-nxp-bsp  ??= "u-boot-fus"
