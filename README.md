# RESCUE — core firmware submission

Developer: Saeed Nik. ArduPlane 4.6.3; verified RESCUE firmware 3.0.1.

This independent repository contains the five deliverables requested in section 6 of the specification. It does not contain the Flight Lab panel, Python interpreter/dependencies, application launchers, installer or QGroundControl distribution.

| Required deliverable | Location |
| --- | --- |
| Patch: mode, supervisor, parameters and parachute integration | [01_Patch/rescue.patch](01_Patch/rescue.patch), all 30 modified source files and build manifest |
| Complete SITL parameter profile | [02_Parameters/rescue.parm](02_Parameters/rescue.parm) and the test-site coordinates |
| Compiled ArduPlane 4.6.3 SITL firmware | [03_SITL/arduplane.exe](03_SITL/arduplane.exe), with the five required native DLLs |
| Original BIN recordings and short phase analysis | [04_Logs/Phase_Analysis.md](04_Logs/Phase_Analysis.md), eight acceptance-flight BINs |
| Parameters and implementation decisions | [05_README/README.md](05_README/README.md), the 26 custom parameters and requirement traceability |

The eight-page [English specification response](05_README/RESCUE_Response_EN_3.0.1.docx) is included. Its content and embedded Golos Text fonts are unchanged from the accepted English document.

## Downloads

- [Core submission ZIP](https://github.com/saeednikakhtar/rescue-core-submission/releases/download/v3.0.1-core/RESCUE-Core-Submission-3.0.1-20261020.zip): only the core deliverables listed above and their documentation/licenses.
- [Complete corresponding firmware source](https://github.com/saeednikakhtar/rescue-core-submission/releases/download/v3.0.1-core/RESCUE-Corresponding-Source-3.0.1.tar.gz): separate source archive for rebuilding, including the initialized SITL build modules. Upstream build scripts are source files; this is not a bundled Python interpreter or application runtime.

The GitHub source ZIP also contains the core deliverables. There is no runtime download bootstrap in this repository.

## Native simulator

`arduplane.exe` is the compiled SITL flight firmware requested by the specification, not a Flight Lab launcher. Keep its five DLLs beside it. See [native execution instructions](03_SITL/RUNNING.md). An independently installed GCS or MAVLink client controls it; the simulator starts disarmed and does not take off automatically.

The original flight binary, full parameter profile, patch and eight reference BINs are byte-identical to the verified firmware delivery. The seven requested scenarios use separate MANUAL and AUTO capture flights. Additional low-height and pilot-input behavior is explicitly disclosed in the technical README.

The supplied binary is Windows x64 Cygwin SITL, not a physical flight-controller flash image. [Build instructions](01_Patch/BUILD.md), [GPL notice](COPYING.txt) and [third-party notices](THIRD_PARTY.txt) are supplied.

`SHA256SUMS.json` identifies every sealed core payload file. Download verification is published as a separate release attachment after the public download has been tested.
