# Deployed modified driver versus monitor DATA experiment

Cross-reference performed on 14 October 2026 against the source recovered from the boards, not a vendor source archive.

## Baseline identity

Both boards report `v5.15.0.1-249-g9245f8bd9.20241218_COEX20240913-390e`. Their packaged `8812eu.ko` files have identical SHA-256 `1a5ce37ff6d008948a39f7ca06adb6000a84b339f1d3bc952fb062959f0d72ad`. Both loaded modules report source version `F9ACD8107A2ED980281E16E`, matching that package's module metadata.

GS has `/home/radxa/rtl8812eu` and `/usr/src/rtl88x2eu-5.15.0.1`. All 719 recovered source/config files match between these two trees. Neither contains Git metadata. No corresponding source tree was found under AU's `/home` or `/usr/src`; AU's matching packaged module and loaded metadata establish that it uses the same identified build, but do not constitute an independent AU source-tree comparison.

The GS tree matches svpcom commit `48e6e449e089fa954e4e15079bd864039e2960da` except for four files, after normalizing line endings:

| File | Deployed customization |
|---|---|
| `Makefile` | AP/P2P enabled; ARM64 platform selected instead of x86. |
| `core/rtw_br_ext.c` | Passes the computed PPPoE relay-tag length explicitly rather than calculating it through `tag->tag_len`. |
| `core/rtw_recv.c` | Formatting-only differences in MP loopback diagnostics. |
| `realtek_88x2eu.conf` | Board interface naming, channel/country options, aggregation, GRO/NAPI, WMM and disabled power-saving options. |

These four files are preserved on branch `board-modified-baseline` (commit `9fe9a8735c54cfc7e7792bbfc7ceae4b572f9c50`) and merged into `monitor-unicast-data`. This avoids accidentally removing AP support or changing build/platform defaults during the experiment. Embedded firmware source is unchanged between the recovered baseline and experimental branch.

## Behavior differences

| Area | Recovered board driver | Experimental branch |
|---|---|---|
| Monitor injection submission | `dump_mgntframe`, management-frame allocation and submission. | Opt-in validated raw unicast DATA frames use normal peer service queues and USB DATA worker. Other injection retains the legacy path. |
| Peer state | No explicit monitor peer registration interface. | `monitor_peer` allocates a host peer/MACID and sends an experimental firmware ADHOC-role media-status report without association. |
| USB batching | Injected frames bypass the ordinary queued DATA batching path. | Uses that batching path, with raw-length accounting and batching counters. Hardware benefit remains to be measured. |
| Retry limit | Injected descriptor hard-codes field value 6, or 0 for radiotap NOACK. | Module parameter `monitor_tx_retry_limit`, range 0..63, default 6; NOACK selects 0. This programs the field, not proof of ACK receipt. |
| ACK timeout | Existing shared `ack_timeout` proc control. | Same hardware control, with bounded 0..255 input and terminated parsing. No automatic distance-based timeout policy. |
| Rate policy | Explicit injected rate and disabled fallback. | Same policy. Firmware rate adaptation is not added. |
| On-air aggregation | Ordinary associated traffic has its existing aggregation path. | Monitor DATA explicitly disables A-MPDU/A-MSDU; no negotiated Block ACK state is implemented. USB batching is separate. |
| Observability | Existing driver counters. | Adds peer/MACID and queued, rejected, copied and submitted USB-batch/frame counters. These are not ACK/completion counters. |

The implementation modifies eight existing files and adds four files relative to the preserved board baseline; the other 711 recovered files match. Changes span `core/rtw_debug.c`, `core/rtw_xmit.c`, `hal/rtl8822e/rtl8822e_ops.c`, `hal/rtl8822e/usb/rtl8822eu_xmit.c`, `include/rtw_debug.h`, `include/rtw_xmit.h`, `include/sta_info.h` and `os_dep/linux/rtw_proc.c`. New implementation files are the parser header, parser test, probe tool and implementation notes; this comparison report is an additional documentation file.

## Defect found during cross-reference

The initial experimental patch placed its raw-frame copying block in `rtw_xmitframe_coalesce_amsdu`, while the USB DATA worker calls `rtw_xmitframe_coalesce`. The misplaced block also referenced an uninitialized packet pointer and a counter-owner variable unavailable in that function. Commit `63c4b12` moves raw copying into the actual worker coalescer and rejects monitor raw DATA in the A-MSDU coalescer. The initial published experimental commit must not be used alone.

AU's kernel also emits an existing `proc_next_txpwr_total_dbm` seq-file position warning. That iterator is shared with the recovered baseline and is not introduced by this experiment. Its throughput impact has not been established.

## Experiment boundary

Parser tests pass with GCC `-Wall -Wextra -Werror`. The complete native module build passes against the actual board kernel `6.1.84-10-rk2410-nocsf`. Build source revision is `a8bc78616c08bfb1bb97633769f52567756a474d`; later commits update documentation only. The module exposes `monitor_data` and `monitor_tx_retry_limit`, and its ARM64 vermagic matches that kernel. Its SHA-256 is `fb6219e23862da993765b18e48faf946860a65f35a4f24f837f50adfc2043422`, with source version `93340DD34A48B572D3325B5`. The build emits a compiler patch-release mismatch warning: kernel GCC 12.2.0-14 versus installed GCC 12.2.0-14+deb12u1; compilation and MODPOST succeed.

No experimental module was installed or loaded as part of this comparison. Queueing, firmware acceptance, received ACKs, retry counts and USB batching under two monitor-only peers still require hardware measurements. The firmware ADHOC peer interpretation remains experimental.

Local audit artifacts include the recovered source archives, source manifests, full file-comparison JSON and `upstream-to-board.patch` / `board-to-experimental.patch` in `output/driver-comparison-20261014` of the parent workspace.
