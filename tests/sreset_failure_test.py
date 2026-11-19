#!/usr/bin/env python3
"""Compile the production reset function against small, observable kernel mocks."""
import argparse
from pathlib import Path
import subprocess
import tempfile

PREFIX = r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef unsigned char u8;
typedef int _irqL;
typedef int systime;
#define DBG_CONFIG_ERROR_RESET 1
#define CONFIG_IPS 1
#define _FALSE 0
#define _TRUE 1
#define _FAIL 0
#define _SUCCESS 1
#define WIFI_STATUS_SUCCESS 0
#define WIFI_SRESET_LEAVE_FAIL (1 << 7)
#define rf_off 0
#define IPS_NORMAL 0
#define IPS_CTRL_LEAVE_SRESET 1
#define RTW_CMDF_DIRECTLY 1
#define ATOMIC_READ(p) (*(p))
#define RTW_INFO(...) ((void)0)
#define RTW_ERR(...) ((void)0)
#define FUNC_ADPT_FMT "%s"
#define FUNC_ADPT_ARG(a) "mock"
struct completion { int done; };
struct sreset_priv {
 int Wifi_Error_Status, user_cmd_running, silent_reset_inprogress;
 int self_dect_fw, rx_cnt;
 struct completion sreset_done;
};
typedef struct { struct sreset_priv srestpriv; } HAL_DATA_TYPE;
struct pwrctrl_priv {
 int lock, change_rfpwrstate, rf_pwrstate, bips_processing, ips_mode_req;
};
struct mlme_priv { int unused; };
struct xmit_priv { int unused; };
struct cmd_priv { int cmdthd_running; };
struct debug_priv { int dbg_sreset_fail_cnt, dbg_sreset_cnt; };
struct dvobj_priv { struct debug_priv drv_dbg; };
typedef struct {
 HAL_DATA_TYPE hal;
 struct pwrctrl_priv power;
 struct mlme_priv mlmepriv;
 struct xmit_priv xmitpriv;
 struct cmd_priv cmdpriv;
 struct dvobj_priv *dvobj;
} _adapter;
#define GET_HAL_DATA(a) (&(a)->hal)
#define adapter_to_pwrctl(a) (&(a)->power)
static int hw_ready, leave_result, stopped, started, entered;
static int rtw_is_hw_init_completed(_adapter *a) { return hw_ready; }
static int rtw_get_current_time(void) { return 0; }
#define rtw_get_passing_time_ms(start) 0
static void init_completion(struct completion *c) { c->done = 0; }
static void complete_all(struct completion *c) { c->done++; }
static void _enter_pwrlock(int *lock) { assert(!*lock); ++*lock; }
static void _exit_pwrlock(int *lock) { assert(*lock == 1); --*lock; }
static void rtw_mi_sreset_adapter_hdl(_adapter *a, int start) {
 assert(a->power.lock == 1);
 if (start) started++; else stopped++;
}
static int rtw_ips_ctrl_wk_cmd(_adapter *a, int ctrl, int value, int flags) {
 return _SUCCESS;
}
static void _ips_enter(_adapter *a) { entered++; a->power.rf_pwrstate = rf_off; }
static int _ips_leave(_adapter *a) { return leave_result; }
'''

SUFFIX = r'''
int main(void) {
 _adapter a;
 struct dvobj_priv d;
 memset(&a,0,sizeof(a)); memset(&d,0,sizeof(d));
 a.dvobj=&d; a.power.rf_pwrstate=1; a.power.ips_mode_req=7;
 hw_ready=1; leave_result=_FAIL;
 sreset_reset(&a);
 assert(stopped==1 && started==0 && entered==1);
 assert(d.drv_dbg.dbg_sreset_fail_cnt==1 && d.drv_dbg.dbg_sreset_cnt==0);
 assert(a.hal.srestpriv.Wifi_Error_Status==WIFI_SRESET_LEAVE_FAIL);
 assert(a.hal.srestpriv.silent_reset_inprogress==0 && a.power.lock==0);
 assert(a.power.ips_mode_req==7 && a.hal.srestpriv.sreset_done.done==1);

 memset(&a,0,sizeof(a)); memset(&d,0,sizeof(d));
 a.dvobj=&d; a.power.rf_pwrstate=1; a.power.ips_mode_req=7;
 stopped=started=entered=0; leave_result=_SUCCESS;
 sreset_reset(&a);
 assert(stopped==1 && started==1 && entered==1);
 assert(d.drv_dbg.dbg_sreset_fail_cnt==0 && d.drv_dbg.dbg_sreset_cnt==1);
 assert(a.power.ips_mode_req==7 && a.power.lock==0);
 assert(a.hal.srestpriv.sreset_done.done==1);

 a.hal.srestpriv.user_cmd_running=1;
 stopped=started=entered=0;
 sreset_reset(&a);
 assert(stopped==0 && started==0 && entered==0 && a.power.lock==0);

 hw_ready=0; a.hal.srestpriv.Wifi_Error_Status=WIFI_SRESET_LEAVE_FAIL;
 sreset_reset(&a);
 assert(a.hal.srestpriv.Wifi_Error_Status==WIFI_SRESET_LEAVE_FAIL);
 puts("production reset: failure, success, completion, locks and command guard PASS");
 return 0;
}
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--compiler', nargs='+', default=['cc'])
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    text = (root / 'core/rtw_sreset.c').read_text()
    start = text.index('void sreset_reset(_adapter *padapter)')
    opening = text.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        if text[end] == '{':
            depth += 1
        elif text[end] == '}':
            depth -= 1
        end += 1
    production = text[start:end]
    with tempfile.TemporaryDirectory(prefix='iris-sreset-') as temp:
        source = Path(temp) / 'reset-test.c'
        binary = Path(temp) / 'reset-test.exe'
        source.write_text(PREFIX + production + SUFFIX)
        subprocess.run(args.compiler + ['-std=c99', '-Wall', '-Wextra', '-Werror',
                       '-Wno-unused-variable', '-Wno-unused-parameter', str(source),
                       '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    main()
