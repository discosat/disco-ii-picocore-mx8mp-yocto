# DISCO-II Yocto Build — Branch Guide

## Branches

### `master`
Common base for all flight builds. Contains all shared recipe updates, layers, and configurations:
- All recipes pinned to verified flight branch commits (DIPP, DiscoCameraController, upload_sat-client, csh)
- Submodule dependencies (libcsp, libparam, libdtp, slash, csp_proc) pinned to the exact commits from each app's flight branch submodules
- `meta-disco-camera` de-submoduled and embedded directly with the flight recipe
- `meta-disco-upload` layer added for the upload_client recipe
- Pre-compiled pipeline modules (.so) baked into `meta-disco-pipeline`
- `disco-scheduler.bb` defaults to `flight-SOM2` on this branch — use the SOM-specific branches for builds

### `flight-SOM1`
Build branch for **SOM1** (CAN node address `5421`). Merges `master` plus:
- `disco-scheduler.bb` → branch `flight-SOM1`, SRCREV `dc0091329609`
- `a53-app-sys-manager.service` → `ExecStart` with node `5421`
- SOM1-specific `a53-app-sys-manager` source code (`main.c`, `meson.build`)

### `flight-SOM2`
Build branch for **SOM2** (CAN node address `5475`). Merges `master` plus:
- `disco-scheduler.bb` → branch `flight-SOM2`, SRCREV `2aad62d76a3e`
- `a53-app-sys-manager.service` → `ExecStart` with node `5475`
- SOM2-specific `a53-app-sys-manager` source code (`main.c`, `meson.build`)

## Build

```bash
git checkout flight-SOM1  # or flight-SOM2
docker compose run yocto-build-service
```

## What differs between SOM1 and SOM2

Only two things:
1. `disco-scheduler.bb` — different cortex-m7-scheduler branch/commit
2. `a53-app-sys-manager.service` — different CAN node address

Everything else (DIPP, camera, upload_client, csh, pipeline modules) is identical.
