# Later bench validation

No installation has been performed by the source port.

1. Build the module separately for each target kernel and retain installed-module
   backups. Require matching vermagic and clean module load before RF tests.
2. Confirm `dbg_sreset_fail_cnt=0` in the interface's ps_dbg_info proc node.
   Use the existing Kimi recovery service only with this marker available.
3. Establish standalone AP/STA with the example configuration, managers removed
   from the RF interface only, MTU1500, and static test RF addresses. Verify WPA
   COMPLETED and packets in both directions. Keep LAN management independent.
4. Record channel, width, rate, GI, LDPC/STBC, driver power request, card profile,
   retry limits, ACK/CTS timing and aggregation readbacks. Neither a configured
   power request nor the monitor-mode 327dBm iw sentinel is measured RF output.
5. Repeat the directional rate matrix: AU0/GS0, AU5/GS0, AU0/GS5, AU5/GS5,
   followed by MCS3 as needed. Keep all other settings fixed. Start iperf in the
   operator's terminal so UDP receiver intervals are visible; use 1200-byte UDP
   datagrams initially. Do not silently start or leave a background server.
6. Stop AP/STA users before moving to monitor mode. Configure peer MACs,
   register the monitor peer, set monitor_tx_retry_limit explicitly, then enable
   monitor_data. Verify monitor_peer queue/copy/batch counters advance and that
   NOACK/retry policy and raw sequence numbers are preserved.
7. Compare monitor_data disabled/enabled, recording RX/TX progression and reset
   markers. USB batching is not over-air aggregation or Block ACK negotiation.
8. Exercise a healthy silent reset, then controlled failed-initialization recovery
   separately. Verify stopped queues and completion notification on failure,
   bounded module reload, fresh peer traffic and video. Do not ship any failure-
   injection toggle. A successful install is not proof of stall recovery.

The recorded AP fault on the installed -277 driver was AU-to-GS failure at AU
MCS3/5 while AU MCS0 passed. The historical 6W-D baseline used other hardware
and -249, so preserve that distinction when interpreting results.
