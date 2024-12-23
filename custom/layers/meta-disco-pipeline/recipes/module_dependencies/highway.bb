SUMMARY = "Highway is a C++ library for SIMD (Single Instruction, Multiple Data)"
HOMEPAGE = "https://github.com/google/highway/" 

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=2b42edef8fa55315f34f2370b4715ca9" 

inherit cmake

SRC_URI = " \
    https://github.com/google/highway.git;branch=master \
"

PV = "1.1.0"
SRCREV = "58b52a717469e62b2d9b8eaa2f5dddb44d4a4cbf"
S = "${WORKDIR}/git"

EXTRA_OECMAKE = "-DBUILD_TESTING=0 -DCMAKE_BUILD_TYPE=Release"
CXXFLAGS:append:arm = " -mfp16-format=ieee"