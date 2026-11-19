# Experimental IRIS port: Realtek v5.15.25-68

Separate development repository based on pristine vendor
`v5.15.25-68-gebc5ded85.20260825_COEX20240913-390e` (25 August 2026).
The vendor import is commit `93f987f`; subsequent commits contain our port.
This has a separate source history from the previous -277 checkout.
The experimental build was deployed for the requested plain AP/STA bench test
on 19 November 2026; see `docs/VALIDATION.md` and `docs/PORT-MANIFEST.json`.

## Ported behavior

- Opt-in `monitor_data` unicast injection through station/MAC-ID state and the
  normal DATA transmit queue, without AP/STA association.
- USB batching for these DATA frames. This does **not** negotiate over-air
  A-MPDU, Block ACK or A-MSDU: those remain disabled for raw injected frames.
- Radiotap rate, MCS/VHT, bandwidth, GI, LDPC, STBC and NOACK handling; preserve
  raw sequence numbers and frame contents. Rate fallback stays disabled.
- Runtime `monitor_tx_retry_limit` (descriptor field 0..63); NOACK frames use
  retry zero. Existing proc ACK/CTS timing controls are retained.
- `/proc/net/rtl88x2eu/<iface>/monitor_peer` registration and queue/batch counters.
- Per-chain SNR reporting, radiotap alignment/length fixes, peer teardown and
  management-buffer fallback for firmware reload when DATA buffers are exhausted.
- Kimi's failed-silent-reset detection: `dbg_sreset_fail_cnt`, BIT7 failure
  status and stopped queues on failed `_ips_leave()`. On this newer vendor
  branch, the failure path also signals `sreset_done` to release waiting
  scan/connect callers.

`monitor_data` remains disabled by default. Module compilation enables monitor
mode; that is separate from opting into the DATA path at runtime. Registration
of the peer and configuration of matching raw-frame addresses are required.
The inherited standalone retry default is 6; the carrier must explicitly set
the selected test retry count. Power calibration belongs to the existing card
profiles, not to this driver port.

## New vendor behavior preserved

The vendor firmware arrays and BB/RF tables are unchanged by our port. Normal
NIC firmware has header version/subversion 30/4 (release label V0030.04).
Keep the new RX pointer-progress detector, disabled old USB TX-buffer-empty
hang heuristic, user-command/reset coordination, USB receive-frame pointer
cleanup, updated regulatory code and all other vendor changes. No old source
files were substituted wholesale for their new equivalents.

Three patch conflicts were resolved manually: the monitor TX entry's newer
preprocessor guard, the relocated receive SNR field, and transmit prototypes.
The reset-completion adaptation is an additional change required by this branch.

## Build only

On a Linux build host with the target kernel headers:

```sh
ARCH=arm64 KSRC=/path/to/matching/kernel/headers CROSS_COMPILE=aarch64-linux-gnu- \
  JOBS=4 bash scripts/build-iris-driver.sh
```

For native ARM64, omit CROSS_COMPILE. The script compiles and checks vermagic;
it never installs, unloads modules, starts services or changes networking.
Use different source/build copies for the AU and GS kernel builds.

The AU kernel was `6.1.84-8-rk2410`; ZERO3 GS was
`6.1.84-10-rk2410-nocsf`. A module built for one must not be loaded on the other.
The vendor `install.sh` is part of the untouched import; it is not used by this
build-only workflow.

## Validation and later testing

Portable frame-parser regression test:

```sh
cc -std=c99 -Wall -Wextra -Werror -Iinclude tests/monitor_data_header_test.c \
  -o /tmp/monitor-data-header-test
/tmp/monitor-data-header-test
```

`tests/sreset_failure_test.py` compiles the production reset function with
mocked kernel services and checks successful reset, failed initialization,
completion notification, lock release and the user-command guard.

```sh
python3 tests/sreset_failure_test.py
```

The example hostapd/supplicant files follow the supplied guides for a simple
WPA2 HT20 link. Edit both passphrases, interfaces and regulatory/channel
settings before use. They do not contain production pairing credentials.
Do not enable monitor_data while testing AP/managed mode. Test AP/STA first,
then the monitor carrier as a separate experiment, keeping power, retry, ACK,
packet size and offered load recorded. See `docs/TEST-PLAN.md`.

Kernel-module compilation and live RF testing are separate validation gates;
portable tests do not establish kernel ABI compatibility or radio stability.
