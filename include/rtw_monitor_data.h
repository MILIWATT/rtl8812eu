/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTW_MONITOR_DATA_H__
#define __RTW_MONITOR_DATA_H__

/* Raw 802.11 content is independent of the DATA queue/descriptor policy. */
#define RTW_MONITOR_DATA_INJECT 0xa6

struct rtw_monitor_data_header {
	unsigned int hdrlen;
	unsigned int tid;
	unsigned int qos;
	unsigned int seq;
};

/* No negotiated keys, fragmentation, or Block ACK state in this first path. */
static inline int rtw_monitor_data_parse(const unsigned char *p,
	unsigned int len, struct rtw_monitor_data_header *out)
{
	unsigned int fc, hdrlen, qos, seq;

	if (!p || !out || len < 24)
		return -1;
	fc = p[0] | ((unsigned int)p[1] << 8);
	if ((fc & 3) || (fc & 0x0c) != 0x08 || (fc & 0x40))
		return -1; /* Not an ordinary data-bearing frame. */
	if ((fc & 0x4400) || (p[22] & 0x0f))
		return -1; /* Protected or fragmented. */
	if ((p[4] & 1) || (p[10] & 1))
		return -1;
	hdrlen = (fc & 0x0300) == 0x0300 ? 30 : 24;
	qos = !!(fc & 0x0080);
	if (len < hdrlen + (qos ? 2 : 0))
		return -1;
	out->tid = 0;
	if (qos) {
		/* No A-MSDU or implicit Block ACK/NoACK policy. */
		if (p[hdrlen] & 0xe0)
			return -1;
		out->tid = p[hdrlen] & 0x0f;
		hdrlen += 2;
		if (fc & 0x8000)
			hdrlen += 4;
	}
	if (len <= hdrlen)
		return -1;
	seq = p[22] | ((unsigned int)p[23] << 8);
	out->hdrlen = hdrlen;
	out->qos = qos;
	out->seq = seq >> 4;
	return 0;
}

#endif
