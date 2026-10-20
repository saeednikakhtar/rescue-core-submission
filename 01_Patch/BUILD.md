# Rebuild the flight firmware

Base: ArduPlane tag Plane-4.6.3, commit 3fc7011a7d3dc047cbb17d8bd98ee94577d144c6. Build: Windows x64 Cygwin, GCC 14.4.0.

rescue.patch is the exact patch for the 30 modified source files. The compiled executable and required DLLs are supplied in ../03_SITL/. This is PC SITL firmware, not a Pixhawk image.

The [complete corresponding source](https://github.com/saeednikakhtar/rescue-core-submission/releases/download/v3.0.1-core/RESCUE-Corresponding-Source-3.0.1.tar.gz) is a separate release asset. It already contains the modifications and initialized SITL build modules. Do not apply rescue.patch again to that archive. source-index.json records base and submodule revisions. The upstream source includes its ordinary build scripts but no bundled Python interpreter.

For a fresh checkout of the pinned base, apply rescue.patch once and initialize the required SITL modules (waf, mavlink, gtest and the DroneCAN modules listed in source-index.json). Configure and build in an independently prepared Cygwin development environment:

```text
./waf configure --board sitl --no-submodule-update
./waf plane
```

build-manifest.json records the authoritative binary and patch SHA-256 values. Existing TECS and AP_Parachute remain responsible for energy control and release. The changes add the mode, supervisor, parameters, logging, mode metadata and progressive native canopy model. The README documents deliberate extensions to the original entry and acknowledgment rules.
