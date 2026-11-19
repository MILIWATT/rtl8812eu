/******************************************************************************
 *
 * Copyright(c) 2007 - 2021 Realtek Corporation.
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
#ifndef __RTW_SELF_DIAG_H__
#define __RTW_SELF_DIAG_H__

void rtw_get_disconnect_info(void *sel, _adapter *padapter);
void rtw_reset_disconnect_info(_adapter *padapter);

#ifdef CONFIG_SELF_DIAG_INFO
#define DIAG_MSG_LEN	80*24*4
void rtw_set_self_diag_info(_adapter *padapter, char* type, char val);
int  rtw_get_self_diag_info(void *sel, _adapter *padapter, u8 bbonce, u8 times,
			    int interval);
#endif
#endif
