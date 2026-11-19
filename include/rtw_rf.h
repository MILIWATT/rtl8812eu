/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#ifndef	__RTW_RF_H_
#define __RTW_RF_H_

#define NumRates	(13)
#define	B_MODE_RATE_NUM	(4)
#define	G_MODE_RATE_NUM	(8)
#define	G_MODE_BASIC_RATE_NUM	(3)
/* slot time for 11g */
#define SHORT_SLOT_TIME					9
#define NON_SHORT_SLOT_TIME				20
#define SLOT_TIME_10M					0xD
#define SLOT_TIME_5M					0x15

typedef enum band_type {
	BAND_ON_2_4G = 0,
	BAND_ON_5G = 1,
#if CONFIG_IEEE80211_BAND_6GHZ
	BAND_ON_6G = 2,
#endif
	BAND_MAX,
	BAND_ON_24G = BAND_ON_2_4G,
} BAND_TYPE, *PBAND_TYPE;

typedef enum _CAPABILITY {
	cESS			= 0x0001,
	cIBSS			= 0x0002,
	cPollable		= 0x0004,
	cPollReq			= 0x0008,
	cPrivacy		= 0x0010,
	cShortPreamble	= 0x0020,
	cPBCC			= 0x0040,
	cChannelAgility	= 0x0080,
	cSpectrumMgnt	= 0x0100,
	cQos			= 0x0200,	/* For HCCA, use with CF-Pollable and CF-PollReq */
	cShortSlotTime	= 0x0400,
	cAPSD			= 0x0800,
	cRM				= 0x1000,	/* RRM (Radio Request Measurement) */
	cDSSS_OFDM	= 0x2000,
	cDelayedBA		= 0x4000,
	cImmediateBA	= 0x8000,
} CAPABILITY, *PCAPABILITY;

enum	_REG_PREAMBLE_MODE {
	PREAMBLE_LONG	= 1,
	PREAMBLE_AUTO	= 2,
	PREAMBLE_SHORT	= 3,
};

#define rf_path_char(path) (((path) >= RF_PATH_MAX) ? 'X' : 'A' + (path))

enum phl_band_idx {
	HW_BAND_0,
	HW_BAND_MAX
};

#define RTW_RLINK_MAX (HW_BAND_MAX)
#define RTW_RLINK_PRIMARY (0)

/* Bandwidth Offset */
#define HAL_PRIME_CHNL_OFFSET_DONT_CARE	0
#define HAL_PRIME_CHNL_OFFSET_LOWER	1
#define HAL_PRIME_CHNL_OFFSET_UPPER	2

enum chan_offset {
	CHAN_OFFSET_NO_EXT	= HAL_PRIME_CHNL_OFFSET_DONT_CARE,	/*SCN - no secondary channel*/
	CHAN_OFFSET_UPPER	= HAL_PRIME_CHNL_OFFSET_LOWER,		/*SCA - secondary channel above*/
	CHAN_OFFSET_LOWER	= HAL_PRIME_CHNL_OFFSET_UPPER,		/*SCB - secondary channel below*/
};

#ifdef CONFIG_NARROWBAND_SUPPORTING
enum nb_config {
	RTW_NB_CONFIG_NONE		= 0,
	RTW_NB_CONFIG_WIDTH_5	= 5,
	RTW_NB_CONFIG_WIDTH_10	= 6,
};
#endif

extern const char *const _band_str[];
#define band_str(band) (((band) >= BAND_MAX) ? _band_str[BAND_MAX] : _band_str[(band)])

extern const u8 _band_to_band_cap[];
#define band_to_band_cap(band) (((band) >= BAND_MAX) ? _band_to_band_cap[BAND_MAX] : _band_to_band_cap[(band)])


extern const char *const _ch_width_str[];
#define ch_width_str(bw) (((bw) < CHANNEL_WIDTH_MAX) ? _ch_width_str[(bw)] : "CHANNEL_WIDTH_MAX")

extern const u8 _ch_width_to_bw_cap[];
#define ch_width_to_bw_cap(bw) (((bw) < CHANNEL_WIDTH_MAX) ? _ch_width_to_bw_cap[(bw)] : 0)

int get_supported_op_class(_adapter *padapter, u8 *op_set, int len);

typedef enum _VHT_DATA_SC {
	VHT_DATA_SC_DONOT_CARE = 0,
	VHT_DATA_SC_20_UPPER_OF_80MHZ = 1,
	VHT_DATA_SC_20_LOWER_OF_80MHZ = 2,
	VHT_DATA_SC_20_UPPERST_OF_80MHZ = 3,
	VHT_DATA_SC_20_LOWEST_OF_80MHZ = 4,
	VHT_DATA_SC_20_RECV1 = 5,
	VHT_DATA_SC_20_RECV2 = 6,
	VHT_DATA_SC_20_RECV3 = 7,
	VHT_DATA_SC_20_RECV4 = 8,
	VHT_DATA_SC_40_UPPER_OF_80MHZ = 9,
	VHT_DATA_SC_40_LOWER_OF_80MHZ = 10,
} VHT_DATA_SC, *PVHT_DATA_SC_E;

typedef enum _PROTECTION_MODE {
	PROTECTION_MODE_AUTO = 0,
	PROTECTION_MODE_FORCE_ENABLE = 1,
	PROTECTION_MODE_FORCE_DISABLE = 2,
} PROTECTION_MODE, *PPROTECTION_MODE;

#define RF_TYPE_VALID(rf_type) (rf_type < RF_TYPE_MAX)

extern const u8 _rf_type_to_rf_tx_cnt[];
#define rf_type_to_rf_tx_cnt(rf_type) (RF_TYPE_VALID(rf_type) ? _rf_type_to_rf_tx_cnt[rf_type] : 0)

extern const u8 _rf_type_to_rf_rx_cnt[];
#define rf_type_to_rf_rx_cnt(rf_type) (RF_TYPE_VALID(rf_type) ? _rf_type_to_rf_rx_cnt[rf_type] : 0)

extern const char *const _rf_type_to_rfpath_str[];
#define rf_type_to_rfpath_str(rf_type) (RF_TYPE_VALID(rf_type) ? _rf_type_to_rfpath_str[rf_type] : "UNKNOWN")

void rf_type_to_default_trx_bmp(enum rf_type rf, enum bb_path *tx, enum bb_path *rx);

enum rf_type trx_num_to_rf_type(u8 tx_num, u8 rx_num);
enum rf_type trx_bmp_to_rf_type(u8 tx_bmp, u8 rx_bmp);
bool rf_type_is_a_in_b(enum rf_type a, enum rf_type b);
u8 rtw_restrict_trx_path_bmp_by_trx_num_lmt(u8 trx_path_bmp, u8 tx_num_lmt, u8 rx_num_lmt, u8 *tx_num, u8 *rx_num);
u8 rtw_restrict_trx_path_bmp_by_rftype(u8 trx_path_bmp, enum rf_type type, u8 *tx_num, u8 *rx_num);
void tx_path_nss_set_default(enum bb_path txpath_nss[], u8 txpath_num_nss[], u8 txpath);
void tx_path_nss_set_full_tx(enum bb_path txpath_nss[], u8 txpath_num_nss[], u8 txpath);

struct rf_ctl_t;

#define BB_GAIN_2G 0
#if CONFIG_IEEE80211_BAND_5GHZ
#define BB_GAIN_5GLB1 1
#define BB_GAIN_5GLB2 2
#define BB_GAIN_5GMB1 3
#define BB_GAIN_5GMB2 4
#define BB_GAIN_5GHB 5
#endif

#if CONFIG_IEEE80211_BAND_5GHZ
#define BB_GAIN_NUM 6
#else
#define BB_GAIN_NUM 1
#endif

int rtw_ch_to_bb_gain_sel(int ch);
void rtw_rf_set_tx_gain_offset(_adapter *adapter, u8 path, s8 offset);
void rtw_rf_apply_tx_gain_offset(_adapter *adapter, u8 ch);

#endif /* _RTL8711_RF_H_ */
