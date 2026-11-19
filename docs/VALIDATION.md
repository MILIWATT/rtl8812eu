# Validation status

- Monitor frame-parser test: PASS (ordinary/QoS/WDS headers, sequence preservation, malformed lengths, protected/fragmented/multicast rejection).
- Production silent-reset function with mocked kernel services: PASS (success, failed initialization, restored IPS mode, stopped adapters on failure, lock release, completion signaling, user-command guard and hardware-not-ready case).
- Firmware, BB/RF tables and cfg80211 source: byte-identical to the pristine new vendor import.
- Patch conflicts: resolved; no reject files remain.
- Kernel module compilation: not yet performed. VPS and boards did not answer at their existing addresses; local Docker was not used for validation.
- Live AP/STA and monitor tests: not performed. No modules installed and no board configurations changed.

Portable tests validate only the exercised userspace-compilable logic. Kernel compilation, ABI checks and radio behavior remain to be validated on a reachable Linux build host.
