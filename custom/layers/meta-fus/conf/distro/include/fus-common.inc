# Common settings for all F&S boards

MKUBIFS_ARGS ??= "-m 2048 -e 126976 -c 2048"

# Remove image_types_fsl because there is a awk command which search after a
# point(.) in the devicetrees. But we have 2 points in the devicetree
# picocom1.2.dtb so the awk command find the first point, so the new name is
# picocom1 instead of picocom1.2. And thats why you get an error at compile
# time. Thats why we copy the image_types_fsl and rename it as image_types_fus
# and changed the line in the original file from
# "DTS_BASE_NAME=`basename ${DTS_FILE} | awk -F "." '{print $1}'`" to
# "DTS_BASE_NAME=`basename ${DTS_FILE} | awk -F ".dtb" '{print $1}'`"
# The Original file will be find in sources/meta-fsl-arm/classes/
# image_types_fsl.bbclass
IMAGE_CLASSES:remove = "image_types_fsl"
IMAGE_CLASSES += "image_types_fus"

# need hosttools crc32 for U-Boot addfsheader.sh
HOSTTOOLS_NONFATAL:append:mx8 = " crc32"

CORE_IMAGE_EXTRA_INSTALL:append = " fs-startscript "

# Remove Kernel Image from rootfs
RRECOMMENDS:${KERNEL_PACKAGE_NAME}-base = ""

# This is necessary because there will be some debian packages created and
# these packages must be lower case. Basically we set CONFIG_LOCALVERSION to
# "-F+S". This appends the string "-F+S". Due to the string is upper case we
# need to set variable to convert it to lower case.
KERNEL_MODULE_PACKAGE_SUFFIX = "${@legitimize_package_name(d.getVar('KERNEL_VERSION'))}"

INHERIT:append ?= " image-buildinfo"
