DESCRIPTION = "F&S Basler camera support image"
LICENSE = "MIT"

require recipes-config/images/fus-image-std.bb

IMAGE_INSTALL:append = " \
		coreutils \
		packagegroup-dart-bcon-mipi \
		packagegroup-imx-isp \
"
