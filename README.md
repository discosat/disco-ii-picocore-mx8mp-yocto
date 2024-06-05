# DISCO II PicoCore™MX8MP system image Yocto setup

## TLDR:
Install [docker](https://www.docker.com/) and then run
```bash
docker compose run yocto-build-service
```
This will setup and run your build environment (in the `.build` subdirectory), yielding files in the `deploy` subdirectory ready to be flashed onto a PicoCore™MX8MP - keep in mind building from scratch can take several hours and requires significant compute, memory and storage.

Transient errors are fairly likely, especially if using a weak internet connection, so re-running the command may be necessary. Additionally, you may want to limit the number of parallel workers with the `BB_NUMBER_THREADS` environment variables, e.g.:

```bash
docker compose run -e BB_NUMBER_THREADS=4 yocto-build-service
```
