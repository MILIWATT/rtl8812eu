#define _RTW_SELF_DIAG_C_
#include <drv_types.h>
#include <hal_data.h>
#ifdef RTW_HALMAC
#include "../../hal/hal_halmac.h"
#endif

const char* _disconnect_code_to_reason(enum disconnect_type disconnect_code)
{
	char *disconnect_reason = "";

	switch (disconnect_code) {
	case DISCONNECTION_NOT_YET_OCCUR:
		disconnect_reason = "Disconnection has not yet occurred.";
		break;
	case DISCONNECTION_BY_SYSTEM_DUE_TO_HIGH_LAYER_COMMAND:
		disconnect_reason = "System requests to disconnect by command.";
		break;
	case DISCONNECTION_BY_SYSTEM_DUE_TO_NET_DEVICE_DOWN:
		disconnect_reason = "System makes net_device down to cause disconnection.";
		break;
	case DISCONNECTION_BY_SYSTEM_DUE_TO_SYSTEM_IN_SUSPEND:
		disconnect_reason = "System enters suspend state to cause disconnection.";
		break;
	case DISCONNECTION_BY_DRIVER_DUE_TO_CONNECTION_EXIST:
		disconnect_reason = "The connection is exist so it triggers disconnection when joinbss.";
		break;
	case DISCONNECTION_BY_DRIVER_DUE_TO_EACH_IFACE_CHBW_NOT_SYNC:
		disconnect_reason = "Multiple interfaces do not synchronize channel and bandwidth when joinbss.";
		break;
	case DISCONNECTION_BY_DRIVER_DUE_TO_DFS_DETECTION:
		disconnect_reason = "DFS Slave mechanism detects radar to cause disconnection.";
		break;
	case DISCONNECTION_BY_DRIVER_DUE_TO_IOCTL_DBG_PORT:
		disconnect_reason = "Using ioctl dbg port command to accomplish disconnection.";
		break;
	case DISCONNECTION_BY_DRIVER_DUE_TO_AP_BEACON_CHANGED:
		disconnect_reason = "AP's beacon content was changed to cause disconnection.";
		break;
	case DISCONNECTION_BY_DRIVER_DUE_TO_KEEPALIVE_TIMEOUT:
		disconnect_reason = "Maybe AP disappears to trigger station keepalive timeout and connection break.";
		break;
	case DISCONNECTION_BY_DRIVER_DUE_TO_LAYER2_ROAMING_TERMINATE:
		disconnect_reason = "Layer2 roaming terminated to trigger disconnection.";
		break;
	case DISCONNECTION_BY_DRIVER_DUE_TO_JOINBSS_TIMEOUT:
		disconnect_reason = "Station joinbss process is timeout to trigger disconnection.";
		break;
	case DISCONNECTION_BY_FW_DUE_TO_FW_DECISION_IN_WOW_RESUME:
		disconnect_reason = "FW decides to disconnect in WOW resume flow.";
		break;
	case DISCONNECTION_BY_AP_DUE_TO_RECEIVE_DISASSOC_IN_WOW_RESUME:
		disconnect_reason = "Receiving Disassociation frame from AP to cause disconnection in WOW resume flow.";
		break;
	case DISCONNECTION_BY_AP_DUE_TO_RECEIVE_DEAUTH_IN_WOW_RESUME:
		disconnect_reason = "Receiving Deauth frame from AP to cause disconnection in WOW resume flow.";
		break;
	case DISCONNECTION_BY_AP_DUE_TO_RECEIVE_DEAUTH:
		disconnect_reason = "Receiving Deauth frame from AP to cause disconnection.";
		break;
	case DISCONNECTION_BY_AP_DUE_TO_RECEIVE_DISASSOC:
		disconnect_reason = "Receiving Disassociation frame from AP to cause disconnection.";
		break;
	case DISCONNECTION_BY_DRIVER_DUE_TO_RECEIVE_CSA_NON_DFS:
		disconnect_reason = "Disconnection due to AP switched to unsupported channel.";
		break;
	case DISCONNECTION_BY_DRIVER_DUE_TO_RECEIVE_CSA_DFS:
		disconnect_reason = "Disconnection due to AP switched to unsupported DFS channel.";
		break;
	case DISCONNECTION_BY_DRIVER_DUE_TO_SELECT_SCANNING_QUEUE_FAIL:
		disconnect_reason = "Disconnection due to Select scanning queue fail";
		break;
	default:
		disconnect_reason = "Unspecified";
		break;
	}

	return disconnect_reason;
}

void rtw_get_disconnect_info(void *sel, _adapter *padapter)
{
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	const char *disconnect_reason;
	int i = 0;
	u32 total_dis_cnt = 0;

	if (pmlmeinfo) {

		disconnect_reason = _disconnect_code_to_reason(pmlmeinfo->disconnect_code);

		RTW_PRINT_SEL(sel, "occurred disconnection time = %d\n", pmlmeinfo->disconnect_occurred_time);
		RTW_PRINT_SEL(sel, "disconnect code = %d\n", pmlmeinfo->disconnect_code);
		RTW_PRINT_SEL(sel, "last disconnect reason: %s\n", disconnect_reason);
		RTW_PRINT_SEL(sel, "beacon code: %d\n", pmlmeinfo->illegal_beacon_code);
		if (pmlmeinfo->disconnect_code == DISCONNECTION_BY_DRIVER_DUE_TO_AP_BEACON_CHANGED) {
			if (pmlmeinfo->illegal_beacon_code & SSID_CHANGED)
				RTW_PRINT_SEL(sel, "illegal beacon reason: The SSID of beacon is changed.\n");
			if (pmlmeinfo->illegal_beacon_code & SSID_LENGTH_CHANGED)
				RTW_PRINT_SEL(sel, "illegal beacon reason: The SSID length of beacon is changed.\n");
			if (pmlmeinfo->illegal_beacon_code & BEACON_CHANNEL_CHANGED)
				RTW_PRINT_SEL(sel, "illegal beacon reason: The Beacon channel of beacon is changed.\n");
			if (pmlmeinfo->illegal_beacon_code & ENCRYPT_PROTOCOL_CHANGED)
				RTW_PRINT_SEL(sel, "illegal beacon reason: The Encrypt protocol of beacon is changed.\n");
			if (pmlmeinfo->illegal_beacon_code & PAIRWISE_CIPHER_CHANGED)
				RTW_PRINT_SEL(sel, "illegal beacon reason: The Pairwise cipheris of beacon changed.\n");
			if (pmlmeinfo->illegal_beacon_code & GROUP_CIPHER_CHANGED)
				RTW_PRINT_SEL(sel, "illegal beacon reason: The Group cipher of beacon is changed.\n");
			if (pmlmeinfo->illegal_beacon_code & IS_8021X_CHANGED)
				RTW_PRINT_SEL(sel, "illegal beacon reason: The 802.1x of beacon is changed.\n");
		} else {
			RTW_PRINT_SEL(sel, "illegal beacon reason: NA.\n");
		}

		RTW_PRINT_SEL(sel, "Wi-Fi reason code: %d\n",
			(pmlmeinfo->disconnect_code) ? pmlmeinfo->wifi_reason_code : 0);

		RTW_PRINT_SEL(sel, "\ndisconnect reason count:\n");
		for (i = 1; i < DISCONNECTION_INFO_MAX; i++) {
			if (pmlmeinfo->disconnect_cnt[i]) {
				total_dis_cnt = total_dis_cnt + pmlmeinfo->disconnect_cnt[i];
				RTW_PRINT_SEL(sel, "%s: %d times\n",
					      _disconnect_code_to_reason(i),
					      pmlmeinfo->disconnect_cnt[i]);
			}
		}

		if (total_dis_cnt != pmlmeinfo->indicate_disconnect_cnt)
			RTW_PRINT_SEL(sel, "\nDisconnect Count Mismatch!! ");
		RTW_PRINT_SEL(sel, "total disconnect count(%d), indicate disconnect count(%d)\n",
			      total_dis_cnt, pmlmeinfo->indicate_disconnect_cnt);
	}
}

void rtw_reset_disconnect_info(_adapter *padapter)
{
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);

	if (!pmlmeinfo)
		return;

	pmlmeinfo->disconnect_occurred_time = 0;
	pmlmeinfo->disconnect_code = DISCONNECTION_NOT_YET_OCCUR;
	pmlmeinfo->illegal_beacon_code = 0;
}


#ifdef CONFIG_SELF_DIAG_INFO
enum diag_cmd_type {
	DIAG_CMD_UNKNOW,
	DIAG_CMD_BB, /* 1 */
	DIAG_CMD_RF, /* 2 */
	DIAG_CMD_MAC, /* 3 */
	DIAG_CMD_MAX
};

static void
_cmd_set(_adapter *padapter, char *phydm_msg, char *cmd_string,
	 enum diag_cmd_type type)
{
	char buf[25] = { 0 };
	u32 cmd_len = 0;
	struct dm_struct *phydm;

	phydm = adapter_to_phydm(padapter);
	cmd_len = strlen(cmd_string);

	if (cmd_len > 23) { /*23 + 1(space character) + 1(end character) = 25*/
		RTW_ERR("%s, cmd_len(%d) > 23, please check this!!\n", __func__,
			cmd_len);
	}
	/* Adding a whitespace at the end of the command string
	 * is a workaround for submodules string handling.
	 */
	snprintf(buf, 25, "%s ", cmd_string);

	_rtw_memset(phydm_msg, 0, DIAG_MSG_LEN);

	switch (type) {
	case DIAG_CMD_BB:
	case DIAG_CMD_RF:
		phydm_cmd(phydm, buf, cmd_len, 1, phydm_msg, DIAG_MSG_LEN);
		break;
	case DIAG_CMD_MAC:
		rtw_halmac_cmd(adapter_to_dvobj(padapter), buf, cmd_len,
			       phydm_msg, DIAG_MSG_LEN);
		break;
	default:
		RTW_ERR("%s, UNKNOW TYPE(%d)\n", __func__, type);
		break;
	}
}

/**
 * rtw_get_self_diag_info - Perform self-diagnosis and output diagnostic information.
 * @sel: Output selection, used for directing output to a specific stream.
 * @padapter: The adapter structure for the device.
 * @bbonce: Flag indicating whether to execute BB diagnostics once.
 * @times: Number of times to perform the self-diagnosis.
 * @interval: Time interval (in milliseconds) between consecutive diagnostics.
 * Return: 0 on success or a negative error code on failure.
 */
int rtw_get_self_diag_info(void *sel, _adapter *padapter, u8 bbonce, u8 times,
			   int interval)
{
	static systime last_time = 0;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	char *phydm_msg;
	_adapter *iface;
	int i;
#ifdef CONFIG_USB_HCI
	u32 trx_submit = 0;
	u32 trx_submit_fail = 0;
	u32 trx_complete = 0;
	u32 trx_complete_fail = 0;
	u32 trx_last_submit = 0;
	char trx_string[10] = { 0 };
#endif
	int count = 0;
	u8 bb_origin_en = dvobj->bb_self_diag_en;

	phydm_msg = rtw_zmalloc(DIAG_MSG_LEN);
	if (NULL == phydm_msg)
		return -ENOMEM;

	if (bbonce) {
		if (!bb_origin_en)
			rtw_set_self_diag_info(padapter, "bb", '1');
		rtw_hal_dm_watchdog(padapter);
		rtw_msleep_os(100);
		rtw_hal_dm_watchdog(padapter);
	}

	if (padapter->registrypriv.ips_mode)
		rtw_pm_set_ips(padapter, IPS_NONE);
	if (padapter->registrypriv.power_mgnt)
		rtw_pm_set_lps(padapter, PS_MODE_ACTIVE);

	for (count = 0; count < times; count++) {
		/* MAC */
		_RTW_PRINT_SEL(
			sel,
			"\n==================================[MAC info]==================================\n");
		_cmd_set(padapter, phydm_msg, "self_diag", DIAG_CMD_MAC);
		_RTW_PRINT_SEL(sel, "%s\n", phydm_msg);

		/* BB */
		_RTW_PRINT_SEL(
			sel,
			"=================================[BB info]==================================\n");
		if (dvobj->bb_self_diag_en) {
			_cmd_set(padapter, phydm_msg, "auto_dbg query 0",
				 DIAG_CMD_BB);
			_RTW_PRINT_SEL(sel, "%s\n", phydm_msg);

			_cmd_set(padapter, phydm_msg, "auto_dbg query 2",
				 DIAG_CMD_BB);
			_RTW_PRINT_SEL(sel, "%s\n", phydm_msg);

			_cmd_set(padapter, phydm_msg, "auto_dbg query 3",
				 DIAG_CMD_BB);
			_RTW_PRINT_SEL(sel, "%s\n", phydm_msg);

		} else {
			_RTW_PRINT_SEL(
				sel,
				"\nPlease use \"echo bb 1 > /proc/net/xxxxx/wlanx/self_diag_info\" to enable bb self diag\n\n");
		}

		/* RF */
		_RTW_PRINT_SEL(
			sel,
			"==================================[RF info]==================================\n");
		_cmd_set(padapter, phydm_msg, "rf dz_dbg rfkinfo", DIAG_CMD_RF);
		_RTW_PRINT_SEL(sel, "%s\n", phydm_msg);

		_cmd_set(padapter, phydm_msg, "rf dz_dbg rtinfo", DIAG_CMD_RF);
		_RTW_PRINT_SEL(sel, "%s\n", phydm_msg);

		_RTW_PRINT_SEL(
			sel,
			"==================================[driver info]==================================\n");

		if (last_time != 0) {
			_RTW_PRINT_SEL(
				sel,
				"get passing time from last get self_diag_info (%d ms)\n\n",
				rtw_get_passing_time_ms(last_time));
		}
		last_time = rtw_get_current_time();

		_RTW_PRINT_SEL(sel, "driver warn on count = %u \n",
			       ATOMIC_READ(&rtw_warn_on_cnt));

#ifdef CONFIG_USB_HCI
		_RTW_PRINT_SEL(
			sel,
			"\n---------------------------------------------------------------------------------\n");
		_RTW_PRINT_SEL(sel, "[ USB Bulk info ]\n\n");
		_RTW_PRINT_SEL(sel, "%s%17s, %s\n", "Driver RX:", "Last count",
			       "Current count");
		_RTW_PRINT_SEL(sel, " BULK_ID, Status, %s, %s, %s, %s, %s\n",
			       "read_port", "read_port", "read_port_complete",
			       "read_port_fail", "read_port_complete_fail");
		for (i = 0; i < RX_STATS_MAX_NUM; i++) {
			trx_submit = dvobj->trx_stats.rx_submit_cnt[i];
			trx_submit_fail = dvobj->trx_stats.rx_submit_fail[i];
			trx_complete = dvobj->trx_stats.rx_complete_cnt[i];
			trx_complete_fail =
				dvobj->trx_stats.rx_complete_fail[i];
			trx_last_submit =
				dvobj->trx_stats.last_rx_submit_cnt[i];
			strcpy(trx_string, "-------");

			if (trx_last_submit != 0) {
				if (trx_last_submit == trx_submit)
					strcpy(trx_string, "No RX");
				else
					strcpy(trx_string, "Have RX");
			}

			if (trx_submit) {
				_RTW_PRINT_SEL(sel,
					       "(%7d,%7s,%10u,%10u,%19u,%15u,%24u)\n", i,
					       trx_string, trx_last_submit,
					       trx_submit, trx_complete,
					       trx_submit_fail,
					       trx_complete_fail);
			}
			dvobj->trx_stats.last_rx_submit_cnt[i] = trx_submit;
		}
		_RTW_PRINT_SEL(sel, "\n%s%18s, %s\n",
			       "Driver TX:", "Last count", "Current count");
		_RTW_PRINT_SEL(sel, " BULK_ID, Status, %s, %s, %s, %s, %s\n",
			       "write_port", "write_port",
			       "write_port_complete", "write_port_fail",
			       "write_port_complete_fail");

		for (i = 0; i < TX_STATS_MAX_NUM; i++) {
			trx_submit = dvobj->trx_stats.tx_submit_cnt[i];
			trx_submit_fail = dvobj->trx_stats.tx_submit_fail[i];
			trx_complete = dvobj->trx_stats.tx_complete_cnt[i];
			trx_complete_fail =
				dvobj->trx_stats.tx_complete_fail[i];
			trx_last_submit =
				dvobj->trx_stats.last_tx_submit_cnt[i];
			strcpy(trx_string, "-------");

			if (trx_last_submit != 0) {
				if (trx_last_submit == trx_submit)
					strcpy(trx_string, "No TX");
				else
					strcpy(trx_string, "Have TX");
			}

			if (trx_submit) {
				_RTW_PRINT_SEL(sel,
					       "(%7d,%7s,%11u,%11u,%20u,%16u,%25u)\n", i,
					       trx_string, trx_last_submit,
					       trx_submit, trx_complete,
					       trx_submit_fail,
					       trx_complete_fail);
			}
			dvobj->trx_stats.last_tx_submit_cnt[i] = trx_submit;
		}
		_RTW_PRINT_SEL(
			sel,
			"---------------------------------------------------------------------------------\n");
#endif
		_RTW_PRINT_SEL(
			sel,
			"\n---------------------------------------------------------------------------------\n");
		_RTW_PRINT_SEL(sel, "[ Disconnect info ]\n");
		for (i = 0; i < dvobj->iface_nums; i++) {
			iface = dvobj->padapters[i];
			if (iface) {
				_RTW_PRINT_SEL(sel, "\n[%s]\n", ADPT_ARG(iface));
				rtw_get_disconnect_info(sel, iface);
			}
		}
		_RTW_PRINT_SEL(
			sel,
			"---------------------------------------------------------------------------------\n");

		rtw_msleep_os(interval);
	}

	if (padapter->registrypriv.ips_mode)
		rtw_pm_set_ips(padapter, padapter->registrypriv.ips_mode);
	if (padapter->registrypriv.power_mgnt)
		rtw_pm_set_lps(padapter, padapter->registrypriv.power_mgnt);

	rtw_mfree(phydm_msg, DIAG_MSG_LEN);
	phydm_msg = NULL;
	if (bbonce) {
		if (!bb_origin_en)
			rtw_set_self_diag_info(padapter, "bb", '0');
	}
	return 0;
}

void rtw_set_self_diag_info(_adapter *padapter, char *type, char val)
{
	char *phydm_msg;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	RTW_INFO("rtw_set_self_diag_info type : '%s' val : '%c'\n", type, val);

	if (strcmp(type, "bb") == 0) {
		char buf[25] = { 0 };

		phydm_msg = rtw_zmalloc(DIAG_MSG_LEN);
		if (NULL == phydm_msg)
			return;

		/* BB */
		snprintf(buf, 25, "%s %c", "auto_dbg type 0", val);
		_cmd_set(padapter, phydm_msg, buf, DIAG_CMD_BB);

		snprintf(buf, 25, "%s %c", "auto_dbg type 2", val);
		_cmd_set(padapter, phydm_msg, buf, DIAG_CMD_BB);

		snprintf(buf, 25, "%s %c", "auto_dbg type 3", val);
		_cmd_set(padapter, phydm_msg, buf, DIAG_CMD_BB);

		rtw_mfree(phydm_msg, DIAG_MSG_LEN);
		phydm_msg = NULL;

		if (val == '1')
			dvobj->bb_self_diag_en = 1;
		else
			dvobj->bb_self_diag_en = 0;

	} else if (strcmp(type, "reset_drv_counter") == 0) {
		if (val == '1') {
			ATOMIC_SET(&rtw_warn_on_cnt, 0);
#ifdef CONFIG_USB_HCI
			_rtw_memset(&(dvobj->trx_stats), 0,
				    sizeof(struct trx_stats));
#endif
		}
		RTW_INFO("driver_warn_on_cnt : %u\n",
			 ATOMIC_READ(&rtw_warn_on_cnt));
	}
}
#endif
