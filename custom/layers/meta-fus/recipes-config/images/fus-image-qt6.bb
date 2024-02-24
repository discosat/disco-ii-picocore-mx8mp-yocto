DESCRIPTION = "F&S Image, adds Qt6 to F&S standard image"
LICENSE = "MIT"

require recipes-config/images/fus-image-std.bb

inherit populate_sdk_qt6_base

CONFLICT_DISTRO_FEATURES = "directfb"

IMAGE_INSTALL += " \
    packagegroup-qt6-fsimx \
"
