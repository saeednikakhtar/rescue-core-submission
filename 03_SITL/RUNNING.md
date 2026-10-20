# Native SITL execution

This directory contains the requested Windows x64 SITL firmware and its native DLL dependencies. No Python interpreter, application launcher or GCS installer is included. RESCUE operates inside arduplane.exe.

Open a terminal in this directory and start the binary directly:

```text
arduplane.exe -w --model plane-elevrev --home 56.0953903,35.883429,161.106674,314.4957409771742 --speedup 1 --defaults ../02_Parameters/rescue.parm --serial0 udpclient:127.0.0.1:14650
```

In PowerShell use `.\arduplane.exe` for the executable name. Keep every DLL beside the binary. The relative parameter path is resolved from this working directory. `-w` resets this local simulator's parameter storage; use a fresh copy/directory when preserving another trial. The command starts disarmed on the configured Alferyevo runway; it does not arm, take off or execute a test scenario. The simulated model is fixed-wing with the supplied progressive canopy behavior.

Configure an independently installed QGroundControl or other MAVLink GCS to listen on UDP 14650. This separate port avoids the Flight Lab relay ports. Connect, wait for normal sensor readiness, arm and take off using the GCS. TAKEOFF controls its own throttle; MANUAL/FBWA use the pilot throttle position. A fresh simulator process repacks the modeled parachute. Stop the terminal process to end this native trial.

The simulator creates EEPROM, terrain and native BIN recordings in its working directory. Use ../02_Parameters/rescue.parm to restore the tested profile for each scenario. Engine failure and disabled-supervisor scenarios use the deliberate case overrides described by the supplied acceptance records.

QGC may report that 4.6.3 is older than current upstream firmware. This deliverable deliberately uses the version required by the specification. Replacing it with stock upstream firmware removes the custom RESCUE implementation.
