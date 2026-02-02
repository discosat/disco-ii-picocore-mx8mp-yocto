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

# Applications

The system contains the following main applications.

## Camera control
The camera control application is responsible for interfacing with the cameras aboard the satellite, acquiring images and forwarding them to the image processing pipeline. The details of this application can be found in the [meta-disco-camera](custom/layers/) layer.

## Image processing pipeline (DIPP)
The image processing pipeline - also denoted DIPP - is responsible for processing the images acquired by the camera control application. It is configurable with different pipeline stages, allowing for a variety of image processing tasks to be performed. The details of this application can be found in the [meta-disco-pipeline](custom/layers/) layer.

## A53 Application System Manager Documentation

**Recipe:** `disco-app-sys-manager` **Binary:** `a53-app-sys-manager` **Service:** `a53-app-sys-manager.service` 

### Overview

The **A53 Application System Manager** acts as the primary "init system" and remote interface for the Linux side of the PicoCoreMX8MP. It utilizes the CubeSat Space Protocol (CSP) to expose system controls, file paths, and process management hooks as network parameters, allowing remote operators (or the OBC) to control the Linux payload without direct shell access.

### 1. Runtime Command-Line Arguments

The application is launched via the `a53-app-sys-manager` binary. The arguments define the initial network configuration and storage location for persistent settings.

**Usage:**

```bash
/usr/bin/a53-app-sys-manager <ADDR> <NETMASK> <IFACE_TYPE> <BAUDRATE> -v <VMEM_PATH>
```

| Position | Argument | Type | Description |
| --- | --- | --- | --- |
| **1** | `ADDR` | `uint16` | **Default CSP Address**: The node address for this application (e.g., `5421`). |
| **2** | `NETMASK` | `uint16` | **Netmask**: The bit-width of the CSP network mask (e.g., `8`). |
| **3** | `IFACE_TYPE` | `uint8` | **Interface Selector**: `0` = **SocketCAN** (`can0`), `1` = **KISS/UART** (`/dev/ttymxc3`). |
| **4** | `BAUDRATE` | `uint32` | **Baud Rate**: Speed for KISS interface (ignored if using CAN). |
| **Flag** | `-v` | `string` | **VMEM Directory**: Path to the directory where `sys-man-config.vmem` is stored (e.g., `/home/root/a53vmem`). |

**Example (from systemd service):**

```bash
/usr/bin/a53-app-sys-manager 5421 8 0 100000 -v /home/root/a53vmem
```

### 2. CSP Parameter Reference

The manager exposes two types of parameters: **RAM** (volatile, resets on reboot) and **VMEM** (persistent, stored in `sys-man-config.vmem`).

### System Control Parameters (RAM)

These parameters trigger immediate actions or control active processes.

| ID | Name | Type | Description |
| --- | --- | --- | --- |
| **31** | `suspend_a53` | `UINT8` | **System Suspend**: Writing `1` triggers Linux suspend (`STOP` A53 cores). |
| **32** | `vimba_install` | `UINT8` | **Driver Install**: Triggers the Vimba USBTL installation script. |
| **33** | `mng_camera_control` | `UINT16` | **Camera Process**: Set to Node ID to start `Disco2CameraControl`. Set to `0` to kill it. |
| **34** | `mng_dipp` | `UINT16` | **DIPP Process**: Set to Node ID to start the Image Processing Pipeline. Set to `0` to kill it. |
| **35** | `switch_m7_bin` | `UINT8` | **M7 Switch**: Placeholder for switching Cortex-M7 binaries (Currently Disabled). |
| **36** | `mng_dipp_interface` | `UINT8` | **DIPP Iface**: `0`=CAN, `1`=KISS. Controls how DIPP communicates. |
| **38** | `mng_camera_interface` | `UINT8` | **Camera Iface**: `0`=CAN, `1`=KISS. Controls how Camera app communicates. |
| **41** | `stdout_buf` | `STRING` | **Console Log**: Read-only buffer containing the last ~100 chars of application stdout. |
| **44** | `mng_util_interface` | `UINT8` | **Uploader Iface**: `0`=CAN, `1`=KISS. Controls how the file uploader communicates. |
| **52** | `time_sync_request` | `UINT16` | **Time Sync**: Set to a Node ID to request CSP CMP time from that node. |
| **54** | `time_sync_last_error` | `UINT32` | **Sync Status**: Read-only error code from the last time sync attempt. |

### Persistent Configuration (VMEM)

These parameters are stored in `sys-man-config.vmem`.

| ID | Name | Type | Description |
| --- | --- | --- | --- |
| **21** | `can_addr` | `UINT16` | Default CSP address loaded at boot. |
| **22** | `can_netmask` | `UINT16` | Default CSP netmask loaded at boot. |
| **24** | `dipp_kiss_baudrate` | `UINT32` | Baudrate configuration for DIPP KISS interface. |
| **25** | `dipp_kiss_netmask` | `UINT16` | Netmask configuration for DIPP KISS interface. |
| **37** | `mng_dipp_vmem_path` | `STRING` | Directory path for DIPP configuration files. |
| **39** | `mng_camera_vmem_path` | `STRING` | Directory path for Camera configuration files. |
| **42** | `mng_util` | `UINT16` | **Uploader Client**: Set address to start `upload_client`. Set `0` to stop. |
| **46** | `mng_util_server` | `UINT16` | **Uploader Server**: Target server address for the uploader client. |
| **47** | `mng_util_rec` | `UINT16` | **Recovery Client**: Set address to start `upload_client_rec`. Set `0` to stop. |
| **48** | `mng_util_rec_server` | `UINT16` | **Recovery Server**: Target server address for the recovery uploader. |
| **45** | `a53_vmem_path` | `STRING` | Directory path for general A53 VMEM files. |

---

### 3. Managed Processes

The system manager wraps external binaries. When specific parameters are modified, the manager constructs command strings dynamically and executes them.

| Managed Binary | Trigger Parameter | Interfaces Supported | Notes |
| --- | --- | --- | --- |
| **Disco2CameraControl** | `mng_camera_control` | CAN, KISS | Automatically kills previous instances before starting. Injects VMEM path and Node ID. |
| **dipp** | `mng_dipp` | CAN, KISS | Image Processing Pipeline. Injects baudrate and netmask if KISS is selected. |
| **upload_client** | `mng_util` | CAN, KISS | Standard file uploader. Starts when `mng_util` (Client Addr) is non-zero. |
| **upload_client_rec** | `mng_util_rec` | CAN, KISS | Recovery uploader. Useful if the main uploader fails. Starts when `mng_util_rec` is non-zero. |

## Cortex-M7 binary
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
