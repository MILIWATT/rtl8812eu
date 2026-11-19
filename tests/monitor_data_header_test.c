#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "rtw_monitor_data.h"

int main(void)
{
	unsigned char p[80] = {0};
	struct rtw_monitor_data_header h;
	unsigned int n;

	p[0] = 0x08; p[4] = 0x02; p[10] = 0x02;
	p[22] = 0x30; p[23] = 0x12;
	assert(!rtw_monitor_data_parse(p, 40, &h));
	assert(h.hdrlen == 24 && !h.qos && h.tid == 0 && h.seq == 0x123);
	for (n = 0; n <= 24; n++)
		assert(rtw_monitor_data_parse(p, n, &h));
	assert(rtw_monitor_data_parse(NULL, 40, &h));
	assert(rtw_monitor_data_parse(p, 40, NULL));
	p[0] = 0x88; p[24] = 5;
	assert(!rtw_monitor_data_parse(p, 40, &h));
	assert(h.qos && h.tid == 5 && h.hdrlen == 26);
	assert(rtw_monitor_data_parse(p, 25, &h));
	p[24] = 0x25; assert(rtw_monitor_data_parse(p, 40, &h));
	p[24] = 0x65; assert(rtw_monitor_data_parse(p, 40, &h));
	p[24] = 0x85; assert(rtw_monitor_data_parse(p, 40, &h));
	p[24] = 0; p[1] = 3; p[30] = 7;
	assert(!rtw_monitor_data_parse(p, 40, &h));
	assert(h.hdrlen == 32 && h.tid == 7);
	p[1] = 0x83;
	assert(!rtw_monitor_data_parse(p, 40, &h));
	assert(h.hdrlen == 36);
	for (n = 24; n <= 36; n++)
		assert(rtw_monitor_data_parse(p, n, &h));
	p[1] = 0x40; assert(rtw_monitor_data_parse(p, 40, &h));
	p[1] = 4; assert(rtw_monitor_data_parse(p, 40, &h));
	p[1] = 0; p[22] |= 1; assert(rtw_monitor_data_parse(p, 40, &h));
	p[22] &= 0xf0; p[4] = 0xff; assert(rtw_monitor_data_parse(p, 40, &h));
	p[4] = 2; p[10] = 1; assert(rtw_monitor_data_parse(p, 40, &h));
	p[10] = 2; p[0] = 0x48; assert(rtw_monitor_data_parse(p, 40, &h));
	p[0] = 0x08; p[1] = 8; /* Existing Retry bit does not alter raw sequence. */
	assert(!rtw_monitor_data_parse(p, 40, &h) && h.seq == 0x123);
	p[0] |= 1; assert(rtw_monitor_data_parse(p, 40, &h));
	puts("monitor data header cases passed");
	return 0;
}
