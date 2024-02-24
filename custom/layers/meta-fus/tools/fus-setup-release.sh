#!/bin/sh
#
# This is a wrapper script for NXPs fsl_setup_release.sh.
# It calls the fsl_setup_release script with the given parameters
# and adds some F&S specific configurations to the build

source ./yocto-f+s-utilities

# Get the command line parameters
while getopts "b:m:h" setup_flag
do
    case $setup_flag in
        b) BUILD_DIR="$OPTARG";
		;;
		m) FS_MODE="$OPTARG";
		;;
		h) SHOW_HELP="1";
    esac
done

if [ -n "$SHOW_HELP" ]; then
	print_usage
	SHOW_HELP=""
	return 1
fi

if [ -z "$DISTRO" ]; then
   	print_usage
	return 1
fi

if [ -z "$MACHINE" ]; then
   	print_usage
	return 1
fi

if [ -z "$BUILD_DIR" ]; then
   	BUILD_DIR=build-$MACHINE-$DISTRO
fi

# Path to fsl-setup-release.sh script
FSL_SETUP_RELEASE=sources/meta-imx/tools/imx-setup-release.sh

# Call NXPs fsl_setup_release script
DISTRO=$DISTRO MACHINE=$MACHINE . $FSL_SETUP_RELEASE -b $BUILD_DIR

# Add FuS-Layer
echo "" >> $BUILD_DIR/conf/bblayers.conf
echo "BBLAYERS += \" \${BSPDIR}/sources/meta-fus \"" >> $BUILD_DIR/conf/bblayers.conf

# Determine root file system mode
if [ "$FS_MODE" == "ro" ]
then
	echo "" >> $BUILD_DIR/conf/local.conf
	echo "# Remove this to make your file system read-writeable:" >> $BUILD_DIR/conf/local.conf
	echo "EXTRA_IMAGE_FEATURES += \"read-only-rootfs \"" >> $BUILD_DIR/conf/local.conf
	echo
	echo "Building read-only root file system"
else
	echo
	echo "Building read/write root file system"
fi

##
# Run layer dependend init
##

OLD_PWD=$PWD
cd ..
LIST_OF_INIT_SCRIPTS=$(ls -d sources/*)
cd $OLD_PWD

for INIT_SCRIPT in $LIST_OF_INIT_SCRIPTS; do

	if [[ -e ../${INIT_SCRIPT}/scripts/fus_setup.sh ]]; then
		echo "Following extra layer init run": "../${INIT_SCRIPT}/scripts/fus_setup.sh"
		export BUILD_DIR=$BUILD_DIR
		sh ../${INIT_SCRIPT}/scripts/fus_setup.sh $BUILD_DIR ${INIT_SCRIPT}/scripts
	fi
done
###
