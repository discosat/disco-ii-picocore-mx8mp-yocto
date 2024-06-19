# DISCO-II PicoCore™MX8MP system image Yocto Project setup

This repository contains the necessary Yocto Project files to build the system image for the PicoCore™MX8MP SoM aboard the DISCO-II CubeSat. The setup builds upon work done by NXP and [F&S Elektronik Systeme GmbH](https://github.com/FSEmbedded/releases-fus/tree/714007497e754a32c23d52a9e900b26bf5e70ab5) with custom layers and configurations for the DISCO-II CubeSat mostly present in the `custom` directory.

## TLDR:
Install [docker](https://www.docker.com/) and then run
```bash
docker compose run yocto-build-service
```
This will setup and run your build environment (in the `.build` subdirectory), yielding files in the `deploy` subdirectory ready to be flashed onto a PicoCore™MX8MP - keep in mind building from scratch can take several hours and requires significant compute, memory and storage.

Transient errors are fairly likely, especially if using a weak internet connection, so re-running the command may be necessary. Additionally, you may want to limit the number of parallel workers with the `BB_NUMBER_THREADS` environment variable, e.g.:

```bash
docker compose run -e BB_NUMBER_THREADS=4 yocto-build-service
```

## Applications

The system contains the following main applications.

### Camera control
The camera control application is responsible for interfacing with the cameras aboard the satellite, acquiring images and forwarding them to the image processing pipeline. The details of this application can be found in the [meta-disco-camera](custom/layers/) layer.

### Image processing pipeline (DIPP)
The image processing pipeline - also denoted DIPP - is responsible for processing the images acquired by the camera control application. It is configurable with different pipeline stages, allowing for a variety of image processing tasks to be performed. The details of this application can be found in the [meta-disco-pipeline](custom/layers/) layer.

### A53 Application/System Manager
The A53 Application/System Manager provides a remote interface to run select system calls on the Linux system, including starting and stopping the camera control and image processing pipeline applications, as well as suspending the system. The details of this application can be found in the [disco-app-sys-manager](custom/layers/meta-disco-scheduler/disco-app-sys-manager.bb) recipe.

### Cortex-M7 binary
The Cortex-M7 is a low-power auxiliary core that is responsible for operations while the A53 cores are in a low-power state with Linux suspended. The binary for this core is built with the [disco-scheduler](custom/layers/meta-disco-scheduler/disco-scheduler.bb) recipe, and more details can be found in the [discosat/disco-ii-cortex-m7-scheduler](https://github.com/discosat/disco-ii-cortex-m7-scheduler) repository.

## Build environment and setup
As demonstrated above, we provide a docker-based environment for ease of use. You can refer to the Dockerfile for a detailed description of the outer build environment. In broad strokes, the build environment is based on Ubuntu 22.04 and includes the following dependencies:

```bash
git curl expect python3 python3-distutils gnupg bzip2 chrpath cpio cpp diffstat file g++ gawk gcc lz4 make binutils zstd wget locales sudo gcc-multilib build-essential socat python3-pip python3-pexpect xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa libsdl1.2-dev xterm python3-subunit mesa-common-dev libarchive-zip-perl
```

Then, the majority of the initial setup work is delegated to the `setup-yocto` script, which fetches a number of necessary git repositories specified in `fs-release-manifest.xml` for things such as the BSP, OpenEmbedded build system, poky including BitBake, etc. After this, the `fus-setup-release.sh` script is available in the resulting `yocto-fus` directory to set up the final build environment.

At this point, the base of the build environment is ready. What's left is to merge the custom layers and configurations for the DISCO-II PicoCore™MX8MP system image in `custom` directory. The environment should be properly sourced already if the previous steps are taken in the same shell, but otherwise the `yocto-fus/setup-environment` can be used to source the environment in subsequent sessions. After that, `bitbake` can be used to build recipes, e.g. `bitbake disco-fus-image` to build the entire system image and related files for the PicoCore™MX8MP. These latter steps are done automatically in the `custom/start.sh` script, which is also what should be modified if you wish to use custom bitbake commands when using the docker environment - e.g. if you want to only build a specific recipe rather than the entire image.

### NXP EULA
The automated build environment setup with Docker automatically accepts the [NXP EULA](https://github.com/Freescale/meta-freescale/blob/9537272254a2898a35672af357fe7689bf27636b/EULA) for the `meta-freescale` layer. Thus, by using the Docker environment, you agree to the terms of the EULA, which can be found here: [LA_OPT_NXP_Software_License v39 August 2022](https://github.com/Freescale/meta-freescale/blob/9537272254a2898a35672af357fe7689bf27636b/EULA)

### Modified ATF, N-boot & U-boot
The system operates in a custom low-power state for the majority of the time. This entails a power-state change request in the Linux kernel, consequently putting the Cortex-A53 cores into WFI mode. The Cortex-M7 core must keep running, and the system PLL1 must not be disabled, since we need greater than 24MHz clock frequency for the CAN peripheral operating at 1,000,000 baud rate with our default CAN timing config. Most other peripherals are disabled and PLLs are disabled or bypassed.

To achieve this, modifications must be made to the ARM Trusted Firmware (ATF) and consequently the F&S-specific N-Boot. F&S has kindly performed the compilation of the ATF and N-Boot based on these changes, ensuring that the system can be booted. The changes made follow that of [https://github.com/discosat/atf-fus](https://github.com/discosat/atf-fus). This version of N-Boot is available in `custom/layers/meta-fus/recipes-bsp/u-boot/files/nboot-disco-mod.fs.gz`. For a more thorough description, see the following forum thread: [4850-configuring-atf-for-power-optimization](https://forum.fs-net.de/index.php?thread/4850-configuring-atf-for-power-optimization/&postID=16819#post16819)
