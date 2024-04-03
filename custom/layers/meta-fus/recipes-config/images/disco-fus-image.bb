DESCRIPTION = "DISCO II Image"
LICENSE = "MIT"

# (Modified from F&S standard image)

inherit core-image

## Select Image Features
IMAGE_FEATURES += " \
    debug-tweaks \
    splash \
    hwcodecs \
    package-management \
"

CORE_IMAGE_EXTRA_INSTALL += " \
    alsa-utils \
    alsa-tools \
    dosfstools \
    evtest \
    e2fsprogs-mke2fs \
    fbset \
    i2c-tools \
    spitools \
    iproute2 \
    memtester \
    ethtool \
    mtd-utils \
    mtd-utils-ubifs \
    lmbench \
    libgpiod \
    libgpiod-tools \
    fbida \
    firmwared \
    strace \
    ltrace \
    gdb \
    kbd \
    pciutils \
    libsndfile1 \
    libusb1 \
    libxml2 \
    bluez5 \
    can-utils \
    iw \
    openssh \
    wpa-supplicant \
    hostapd \
    liberation-fonts \
    linux-firmware-wl12xx \
    linux-firmware-wl18xx \
    linux-firmware-sd8787 \
    linux-firmware-sd8997 \
    linux-firmware-pcie8997 \
    linux-firmware-atmel-mxt \
    nxp-wlan-sdk \
    kernel-module-nxp89xx \
    v4l-utils \
    procps \
"
