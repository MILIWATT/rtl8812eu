# Validation status

- Portable monitor-frame parser and production reset-function regression tests: PASS.
- Firmware, BB/RF tables and cfg80211 source preserved from vendor import.
- Native kernel builds: PASS for 6.1.84-8-rk2410 (AU) and 6.1.84-10-rk2410-nocsf (GS). Radxa ABI validation uses generated UTS_RELEASE.
- Installed loaded version on both: v5.15.25-68, srcversion 11ADB7057151075FB97230B.
- Plain WPA2 AP/STA: GS AP / AU station, channel161 HT20, MTU1500.
- Automatic driver rate control (RA), automatic transmit power, monitor_data=N. No custom rate, retry/ACK or aggregation writes were made for this setup. ACK readback is the fresh driver default 33us.
- Existing module option files bypassed for this temporary test; existing country BO retained. Persistent module option files were not edited.
- Production link/video/FEC/telemetry/recovery services stopped and runtime-masked.
- RF routes verified through wlan1; five of five ping replies in each direction.
- No iperf server/client started; bandwidth measurement belongs to the operator.
- Raw monitor injection, long-duration RF stability and forced recovery remain untested on this release.

Deployment records and per-kernel hashes: PORT-MANIFEST.json. Old modules and active-service records are backed up on each board under /var/backups/iris-vendor68-20261119-041833/. Standalone configuration and process files are under /run/iris-vendor68-test/. This temporary test setup ends on reboot; the installed new module remains on disk.
