SUMMARY = "Lossless compression library and tool"
DESCRIPTIOM = "Brotli is a generic-purpose lossless compression algorithm \
that it is similar in speed to deflate but offers more dense compression."
HOMEPAGE = "https://github.com/google/brotli"
BUGTRACKER = "https://github.com/google/brotli/issues"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=941ee9cd1609382f946352712a319b4b"
UPSTREAM_CHECK_URI = "https://github.com/google/brotli/releases"

SRC_URI = "https://github.com/google/brotli/archive/v${PV}.tar.gz"
SRC_URI[sha256sum] = "425591040a467c0d670f770b21adf85764053d9e00ed7814059c664216c5ea15"

inherit cmake lib_package