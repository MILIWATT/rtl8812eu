#!/usr/bin/env python3
"""Compile the actual cfg80211 getter against a minimal power-policy fixture."""
import pathlib
import subprocess
import tempfile

root = pathlib.Path(__file__).resolve().parents[1]
source = (root / 'os_dep/linux/ioctl_cfg80211.c').read_text()
start = source.index('static int cfg80211_rtw_get_txpower(')
end = source.index('\n#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31)) */', start)
getter = source[start:end]
fixture = r'''
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#define KERNEL_VERSION(a,b,c) (((a)<<16)|((b)<<8)|(c))
#define LINUX_VERSION_CODE KERNEL_VERSION(6,1,84)
#define UNSPECIFIED_MBM 32767
#define MBM_PDBM 100
#define RTW_INFO(...) ((void)0)
typedef int16_t s16;
typedef struct adapter { int unused; } _adapter;
struct dvobj_priv { int unused; };
struct wiphy { int unused; };
struct wireless_dev { int unused; };
struct rtw_wiphy_data { s16 txpwr_total_target_mbm, txpwr_total_lmt_mbm; };
static struct dvobj_priv dvobj;
static _adapter adapter;
static struct rtw_wiphy_data policy;
static s16 estimate;
static struct dvobj_priv *wiphy_to_dvobj(struct wiphy *w) { (void)w; return &dvobj; }
static void *wdev_to_ndev(struct wireless_dev *w) { return w; }
static void *rtw_netdev_priv(void *n) { (void)n; return &adapter; }
static s16 rtw_adapter_get_oper_txpwr_max_mbm(_adapter *a,int e) { (void)a;(void)e;return estimate; }
static s16 rtw_get_oper_txpwr_max_mbm(struct dvobj_priv *d,int e) { (void)d;(void)e;return estimate; }
static struct rtw_wiphy_data *rtw_wiphy_priv(struct wiphy *w) { (void)w;return &policy; }
'''
cases = r'''
int main(void) {
    struct wiphy w; struct wireless_dev dev; int dbm = -999;
    policy.txpwr_total_target_mbm=500; policy.txpwr_total_lmt_mbm=1000;
    estimate=UNSPECIFIED_MBM;
    assert(cfg80211_rtw_get_txpower(&w,&dev,&dbm)==0 && dbm==5);
    policy.txpwr_total_target_mbm=UNSPECIFIED_MBM;
    assert(cfg80211_rtw_get_txpower(&w,&dev,&dbm)==0 && dbm==10);
    policy.txpwr_total_lmt_mbm=UNSPECIFIED_MBM; dbm=-999;
    assert(cfg80211_rtw_get_txpower(&w,&dev,&dbm)==-ENODATA && dbm==-999);
    estimate=1700;
    assert(cfg80211_rtw_get_txpower(&w,&dev,&dbm)==0 && dbm==17);
    estimate=UNSPECIFIED_MBM; policy.txpwr_total_target_mbm=500;
    assert(cfg80211_rtw_get_txpower(&w,0,&dbm)==0 && dbm==5);
    return 0;
}
'''
with tempfile.TemporaryDirectory() as directory:
    src = pathlib.Path(directory) / 'power.c'
    binary = pathlib.Path(directory) / 'power-test'
    src.write_text(fixture + getter + cases)
    subprocess.run(['cc', '-std=c99', '-Wall', '-Wextra', '-Werror', str(src), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
print('cfg80211 power estimate / target / limit / unavailable / wiphy fallback PASS')
