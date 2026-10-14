# Experimental monitor unicast DATA path

Based on svpcom/rtl8812eu `v5.15.0.1` commit
`48e6e449e089fa954e4e15079bd864039e2960da`.

Implementation status: source changes and executable C header tests complete.
No Linux kernel module build or hardware validation has been completed here.
The boards are offline, and the local Docker Linux engine was unavailable.
Do not treat this as a production-ready module.

## Implemented path

Radiotap parsing → validated raw unicast WLAN frame → explicit monitor peer and
TID/access-category preparation → normal `rtw_hal_xmitframe_enqueue` → normal
station service queues → existing USB batching worker → raw-aware
`rtw_xmitframe_coalesce` → normal DATA descriptor branch → normal USB submission.

The raw frame is not converted from Ethernet, wrapped in a second WLAN header,
or given a second LLC header. Supplied sequence numbers are retained. A supplied
radiotap FCS is removed so hardware can append its own. Length accounting uses
the complete raw frame size. The skb remains owned by its data frame until the
worker copies it or the normal teardown/failure path releases it.

This is opt-in (`monitor_data=0` by default). Broadcast/multicast and management
injection retain the legacy path. The new DATA path rejects missing peers,
nonlocal transmitter addresses, protected/fragmented/null frames, QoS NoACK or
Block ACK policies, A-MSDU, and oversized/truncated headers. Interface MAC must
be a unicast address. Initial individual frame bound is the existing management
buffer payload capacity; USB batches use the normal larger data buffer.

## Peer and firmware state

Both Linux interfaces stay in monitor mode; no authentication, association,
beacons, or AP/STA join operation is issued. Register the receiver MAC explicitly
through the new `monitor_peer` proc node. This allocates the driver's normal peer
service queues and a MACID and submits a firmware media-status report with the
ADHOC peer role. That role is an experimental firmware interpretation, not a
verified association-free firmware contract. Linux monitor mode does not imply
the firmware will accept every DATA operation in its NOLINK MAC state.

Ordinary associated DATA still requires the existing association check. Its
exception is limited to a raw frame and an explicitly registered monitor peer.
Leaving monitor hardware mode disconnects these firmware peer reports and uses
normal station teardown to drain their queues and release MACIDs. Re-register
peers when entering monitor again. There is no live peer-deletion interface.

## Policy controls

- `/sys/module/8812eu/parameters/monitor_data`: enable the shared DATA path.
- `/sys/module/8812eu/parameters/monitor_tx_retry_limit`: 0..63, default 6.
  Radiotap NOACK programs retry limit 0, matching the old driver's policy;
  this does not independently prove ACK waiting is disabled in hardware.
- `/proc/net/rtl88x2eu/<interface>/ack_timeout`: existing shared hardware timer,
  documented by the driver in microseconds. Writes now reject values beyond
  its eight-bit field and safely terminate input. Configure after channel/mode
  setup and read back. Reset/reconfiguration persistence is not established.
- `/proc/net/rtl88x2eu/<interface>/monitor_peer`: register a remote MAC; read
  peer MACIDs and queued/dropped/copied/USB batch/frame counters.

The retry parameter is module-wide. Peer registration and counters are per
adapter. Counters wrap at 32 bits and reset with driver initialization. `dropped`
counts ingress rejection; later worker/USB failures are not all included.
`usb_batches`/`usb_frames` count batches handed to the USB submission path,
not completed URBs, delivered frames, or received ACKs. Existing USB completion
and driver TX reports remain their normal mechanisms; no per-packet userspace
ACK notification or new synchronous ACK waiter is introduced.

Radiotap PHY rate/width/GI remain explicit. Rate fallback stays disabled because
no negotiated peer rate mask exists yet. Normal firmware rate adaptation is not
implemented. On-air A-MPDU and automatic ADDBA are deliberately disabled for
these peers; USB batching works independently. Supporting A-MPDU requires a
separate working sequence/TID and Block ACK state implementation on both ends.

## Offline validation

The actual header parser is tested by `tests/monitor_data_header_test.c`: ordinary
and QoS DATA, three/four-address layouts, HT-control length, truncation, invalid
version, multicast, protected/fragmented/null frames, unsupported QoS policies,
and preservation of sequence numbers with Retry set. It was compiled with MSVC
using `/W4 /WX` and executed successfully. `git diff --check` also passes.
These tests do not compile or exercise the kernel driver, its queues, or USB.

On Linux the header tests can be run with:

```sh
cc -Wall -Wextra -Werror -Iinclude tests/monitor_data_header_test.c -o /tmp/monitor-header-test
/tmp/monitor-header-test
```

Build the kernel module for the exact target kernel before a bench load. Example
for the previous ROCK 5C environment, with the existing kernel headers installed:

```sh
make -j2 ARCH=arm64 KVER="$(uname -r)" KSRC="/lib/modules/$(uname -r)/build" \
  CONFIG_PLATFORM_I386_PC=n USER_MODULE_NAME=8812eu \
  USER_EXTRA_CFLAGS='-DCONFIG_LITTLE_ENDIAN -DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT -DCONFIG_USB_INBAND' \
  KCFLAGS=-Wno-error
modinfo -F vermagic ./8812eu.ko
modinfo -F parm ./8812eu.ko
```

## First hardware test, when available

Use the existing bench channel/power settings and save the current module for
rollback. After loading the built module and setting both interfaces to monitor:

```sh
# On each board, use the OTHER board's unicast interface MAC.
echo 02:11:22:33:44:55 | sudo tee /proc/net/rtl88x2eu/wlan1/monitor_peer
echo Y | sudo tee /sys/module/8812eu/parameters/monitor_data
cat /proc/net/rtl88x2eu/wlan1/ack_timeout
cat /proc/net/rtl88x2eu/wlan1/monitor_peer

# Explicit transmission: this is not run by any offline test.
sudo python3 tools/monitor_data_probe.py wlan1 02:11:22:33:44:55 --count 100
cat /proc/net/rtl88x2eu/wlan1/monitor_peer
```

Replace the example MAC. Capture receiver data and, preferably with a separate
observer, ACK frames. A successful syscall or increased USB counter proves only
host submission. Repeat both directions and compare NOACK, retry field values,
and ACK timeout. Use USB capture to verify multiple descriptor/frame records per
URB and unchanged bytes/sequence. A burst should permit `usb_frames > usb_batches`;
sparse sends may legitimately form one-frame batches.

Also test malformed frames, unknown peer, queue exhaustion, interface mode exit,
module removal, and re-entry/re-registration. Do not infer throughput improvement
until matched baseline/modified measurements and retry/completion observations
are available.
