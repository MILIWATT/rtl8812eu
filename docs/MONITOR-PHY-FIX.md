# Monitor injection PHY controls and receive reporting

## Evidence and scope (2026-11-25)

The original WFB-compatible driver handles `pkt_attrib.inject == 0xa5` in
`hal/rtl8822e/usb/rtl8822eu_xmit.c`. That branch explicitly sets software
sequence, retry limit, fixed rate, fallback disable, short GI, bandwidth,
LDPC and STBC. Git blame places the branch in the old imported base
48e6e44; the configurable monitor retry limit was added later.

Vendor-68 port 7f6869d preserved the override for `raw_inject` (the new
DATA worker path), but legacy `monitor_data=N` frames only reached the
ordinary management descriptor branch. That branch does not transfer
radiotap STBC/LDPC/width/GI and uses retry limits 6/12. WFB-ng and IRIS
encode the same HT radiotap STBC/LDPC flags, so changing the userspace
header or forcing DATA queues is not the remedy.

The fix adds `monitor_inject`, set only by the legacy monitor entry point,
and shares the existing final descriptor override with `raw_inject`.
The markers remain distinct: legacy injection keeps its management queue,
while DATA injection retains its peer/aggregation/worker behavior.
Unicast hardware ACK and configured monitor retries remain available.
Ordinary AP/STA data and management frames do not select this override.

## Receive metadata

A pre-fix five-second GS capture saw 5999 MCS7 frames with STBC and LDPC
reported zero, plus 392 frames without MCS metadata. Those zeros were not
independent proof of the RF coding: rx_pkt_attrib fields were zeroed in
rtl8822e_rxdesc2attribute but never populated from the PHY report.

The chipset's JGR3 report pages 1 through 5 share DW1 byte 7, with LDPC
in bit 5 and STBC in bit 6, as defined by phy_sts_rpt_jgr3_type1,
type2_3, type4 and type5 in hal/phydm/phydm_phystatus.h. The monitor
parser now decodes those current-packet bytes after checking physt,
report size and page. It does not read the shared debug cache.
Missing/short/unknown reports cannot retain the previous packet's bits.
A validity flag distinguishes an observed zero from absent information;
HT radiotap only marks FEC/STBC known when there is a source for them.
The STBC PHY bit is boolean; on the one-stream HT MCS0..7 link it maps
to one STBC stream. This change does not add arbitrary multi-stream
STBC-count decoding or repair other unrelated VHT radiotap fields.

## Validation

`python3 tests/monitor_phy_test.py` compiles the production descriptor
override and RX decoder. It checks legacy and DATA paths, ordinary-frame
non-selection, rate/sequence, retries and NOACK, GI/width, both coding
bits on/off, supported PHY pages, missing metadata, and HT known bits.
A second fixture uses Realtek's actual descriptor macros and verifies
serialized descriptor bit positions. `tests/power_readback_test.py` also
passes. These are source-level checks; an updated receiver capture is
needed to validate the live transmitter/receiver combination.

## Live ZERO3 H09 pair result

The kernel-matched module built successfully for
`6.1.84-10-rk2410-nocsf` and was installed on both boards. Module SHA256:
`2be7a694bab1d484d63550694e313dfdc0ecb54618af4792f28731af1f03d149`.
Loaded and on-disk srcversion both read `6EEC743AC64258C9E80C6A8`.

Five-second received-packet captures after reload:

| Receiver | Peer frames | MCS | HT known | HT flags | STBC | LDPC | Unknown / errors |
|---|---:|---:|---|---|---:|---:|---|
| GS | 7295 | 7 | 0x37 | 0x30 | 1 | 1 | 0 / 0 |
| AU | 2313 | 7 | 0x37 | 0x30 | 1 | 1 | 0 / 0 |

Both remain IRIS monitor unicast, `monitor_data=N`, channel 161 HT20,
retry limit 20 and ACK timeout 200 us. Protected configuration hashes
were unchanged. Both directions answered 3/3 pings; the GS observed
approximately 27 Mbps received traffic. This is a functional bench check,
not a long-range, sustained-load or AP/STA stability qualification.

The first install attempt rejected missing opposite-role services before
installing the new module and restored the old module. The installer was
corrected to stop only loaded units. The successful installation backups
are `/var/backups/iris-monitor-phy-20261125-090644-gs` and
`/var/backups/iris-monitor-phy-20261125-090644-au`; both were committed
after verification and their rollback timers stopped.
