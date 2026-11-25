#!/usr/bin/env python3
"""Exercise the real injection descriptor override and RX PHY decoder in C."""
import pathlib
import re
import subprocess
import tempfile

root = pathlib.Path(__file__).resolve().parents[1]
tx = (root / 'hal/rtl8822e/usb/rtl8822eu_xmit.c').read_text()
start = tx.index('\tif (pattrib->raw_inject || pattrib->monitor_inject) {')
end = tx.index('\n\trtl8822e_fill_txdesc_bf', start)
override = tx[start:end]
rx = (root / 'core/monitor/rtw_radiotap.c').read_text()
start = rx.index('void rx_query_moinfo(')
end = rx.index('\n#endif /* CONFIG_WIFI_MONITOR */', start)
decoder = rx[start:end]
start = rx.index('\t\t/* FEC Type */')
end = rx.index('\t\t/* known.MCS index */', start)
radiotap = rx[start:end]
setters = sorted(set(re.findall(r'SET_TX_DESC_(\w+)_8822E', override)))
fixture = r"""
#include <assert.h>
#include <string.h>
#include <stdint.h>
#define CONFIG_RTL8822E 1
typedef uint8_t u8;
struct pkt_attrib { int raw_inject, monitor_inject, seqnum, retry_ctrl, rate, sgi, bwmode, ldpc, stbc; };
struct rx_pkt_attrib { u8 physt, drvinfo_sz, phy_coding_valid, ldpc, stbc; u8 moif[12]; };
static void _rtw_memcpy(void *d,const void *s,int n) { memcpy(d,s,n); }
static int limit;
static int rtw_monitor_retry_limit(void) { return limit; }
static int MRateToHwRate(int r) { return r; }
"""
fixture += 'enum { ' + ', '.join(setters) + ', DESC_COUNT };\n'
for setter in setters:
    fixture += '#define SET_TX_DESC_%s_8822E(d,v) ((d)[%s]=(v))\n' % (setter,setter)
fixture += 'static int fill(struct pkt_attrib *pattrib,int *ptxdesc) { int DriverFixedRate=0;\n' + override + '\nreturn DriverFixedRate; }\n'
fixture += decoder
fixture += r"""
#define IEEE80211_RADIOTAP_MCS_HAVE_FEC 0x10
#define IEEE80211_RADIOTAP_MCS_HAVE_STBC 0x20
struct monitor { struct { struct { u8 ldpc,stbc; } snif_info; } u; };
static void emit(struct rx_pkt_attrib *a, struct monitor *moif, u8 *hdr_buf) {
    int rt_len=0;
""" + radiotap + r"""
}
int main(void) {
    struct pkt_attrib t={0}; int d[DESC_COUNT],i,path,coding;
    struct rx_pkt_attrib a={0}; struct monitor m={0}; u8 phy[48]={0}, rt[3];
    /* Legacy, DATA and neither: only injection receives this override. */
    for(path=0;path<3;path++) for(coding=0;coding<4;coding++) {
        for(i=0;i<DESC_COUNT;i++) d[i]=-1;
        t.raw_inject=path==1; t.monitor_inject=path==0;
        t.seqnum=123; t.retry_ctrl=1; t.rate=19; t.sgi=1;
        t.bwmode=1; t.ldpc=coding&1; t.stbc=(coding>>1)&1; limit=20;
        assert(fill(&t,d)==(path!=2));
        if(path==2) { for(i=0;i<DESC_COUNT;i++) assert(d[i]==-1); continue; }
        assert(d[EN_HWSEQ]==0 && d[SW_SEQ]==123);
        assert(d[RTY_LMT_EN]==1 && d[RTS_DATA_RTY_LMT]==20);
        assert(d[USE_RATE]==1 && d[DATARATE]==19);
        assert(d[DISDATAFB]==1 && d[DISRTSFB]==1);
        assert(d[DATA_SHORT]==1 && d[DATA_BW]==1);
        assert(d[DATA_LDPC]==(coding&1) && d[DATA_STBC]==((coding>>1)&1));
        t.retry_ctrl=0; t.sgi=0; t.bwmode=0; fill(&t,d);
        assert(d[RTS_DATA_RTY_LMT]==0 && d[DATA_SHORT]==0 && d[DATA_BW]==0);
        t.retry_ctrl=1; limit=60; fill(&t,d); assert(d[RTS_DATA_RTY_LMT]==60);
    }
    /* Actual per-packet PHY bits, unrelated high bits and page-counter bits. */
    a.physt=1; a.drvinfo_sz=32;
    for(i=1;i<=5;i++) for(coding=0;coding<4;coding++) {
        phy[0]=0xb0|i; phy[7]=0x9f | (coding<<5);
        rx_query_moinfo(&a,phy);
        assert(a.phy_coding_valid && a.ldpc==(coding&1) && a.stbc==((coding>>1)&1));
        memset(rt,0,sizeof(rt)); emit(&a,&m,rt);
        assert(rt[0]==0x30 && rt[1]==((coding&1)<<4 | ((coding>>1)&1)<<5));
    }
    /* A missing/short/unknown report must not retain the previous packet. */
    for(i=0;i<4;i++) {
        a.physt=i!=0; a.drvinfo_sz=i==1?7:32; phy[0]=i==2?0:15;
        a.phy_coding_valid=a.ldpc=a.stbc=1;
        rx_query_moinfo(&a,phy);
        assert(!a.phy_coding_valid && !a.ldpc && !a.stbc);
        memset(rt,0,sizeof(rt)); emit(&a,&m,rt); assert(rt[0]==0 && rt[1]==0);
    }
    /* Existing monitor metadata remains a supported independent source. */
    m.u.snif_info.ldpc=m.u.snif_info.stbc=1;
    memset(rt,0,sizeof(rt)); emit(&a,&m,rt); assert(rt[0]==0x30 && rt[1]==0x30);
    a.drvinfo_sz=40; memset(phy+32,0x5a,12); rx_query_moinfo(&a,phy);
    assert(a.moif[0]==0x5a && a.moif[7]==0x5a);
    a.drvinfo_sz=48; rx_query_moinfo(&a,phy); assert(a.moif[11]==0x5a);
    return 0;
}
"""
# Compile the same override with Realtek's real descriptor macros as well.
wire = r"""
#include <assert.h>
#include <stdint.h>
#include <string.h>
typedef uint8_t u8;
static uint32_t read_le(const u8 *p) {
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static void write_bits(u8 *p,int start,int width,uint32_t value) {
    uint32_t mask=((1U<<width)-1U)<<start;
    uint32_t v=(read_le(p)&~mask)|((value<<start)&mask);
    p[0]=v; p[1]=v>>8; p[2]=v>>16; p[3]=v>>24;
}
#define HALMAC_8822E_SUPPORT 1
#define SET_BITS_TO_LE_4BYTE(p,s,w,v) write_bits(p,s,w,v)
#include "hal/halmac/halmac_tx_desc_nic.h"
#include "hal/halmac/halmac_tx_desc_chip.h"
struct pkt_attrib { int raw_inject,monitor_inject,seqnum,retry_ctrl,rate,sgi,bwmode,ldpc,stbc; };
static int rtw_monitor_retry_limit(void) { return 20; }
static int MRateToHwRate(int r) { return r; }
static int fill(struct pkt_attrib *pattrib,u8 *ptxdesc) { int DriverFixedRate=0;
""" + override + r"""
return DriverFixedRate;
}
int main(void) {
    u8 d[48]; struct pkt_attrib t={0}; int path,coding;
    for(path=0;path<2;path++) for(coding=0;coding<4;coding++) {
        memset(d,0,sizeof(d));
        t.monitor_inject=path==0; t.raw_inject=path==1;
        t.seqnum=4095; t.retry_ctrl=1; t.rate=19; t.sgi=1; t.bwmode=1;
        t.ldpc=coding&1; t.stbc=(coding>>1)&1;
        assert(fill(&t,d)==1);
        /* Descriptor DW4 retry limit/enable, DW5 PHY fields: independent offsets. */
        assert(((read_le(d+0x10)>>18)&63)==20);
        assert(((read_le(d+0x10)>>17)&1)==1);
        assert(((read_le(d+0x14)>>4)&1)==1);
        assert(((read_le(d+0x14)>>5)&3)==1);
        assert(((read_le(d+0x14)>>7)&1)==(unsigned)(coding&1));
        assert(((read_le(d+0x14)>>8)&3)==(unsigned)((coding>>1)&1));
        t.retry_ctrl=0; t.sgi=0; t.bwmode=0; t.ldpc=0; t.stbc=0;
        fill(&t,d);
        assert(((read_le(d+0x10)>>18)&63)==0);
        assert((read_le(d+0x14)&0x3f0)==0);
    }
    return 0;
}
"""
with tempfile.TemporaryDirectory() as directory:
    src = pathlib.Path(directory)/'monitor-phy.c'
    binary = pathlib.Path(directory)/'monitor-phy-test'
    for code in (fixture, wire):
        src.write_text(code)
        subprocess.run(['cc','-std=c99','-Wall','-Wextra','-Werror','-I',str(root),str(src),'-o',str(binary)],check=True)
        subprocess.run([str(binary)],check=True)
print('legacy/DATA injection descriptors, NOACK/retries, RX coding and radiotap validity PASS')
