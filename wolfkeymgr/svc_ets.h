/* svc_ets.h
 *
 * Copyright (C) 2006-2021 wolfSSL Inc.
 *
 * This file is part of wolf Key Manager.
 *
 * wolfKeyMgr is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * wolfKeyMgr is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

#ifndef WOLFKM_SVC_ETS_H
#define WOLFKM_SVC_ETS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EtsSvcConfig {
    EtsKeyType     keyTypeDef; /* default key type */
    word32          renewSec;
    word32          maxUseCount;
} EtsSvcConfig;


WOLFKM_LOCAL SvcInfo* wolfEtsSvc_Init(const EtsSvcConfig* config);
WOLFKM_LOCAL int wolfEtsSvc_Start(SvcInfo* svc, struct event_base* mainBase, const char* listenPort);
WOLFKM_LOCAL void wolfEtsSvc_Cleanup(SvcInfo* svc);

WOLFKM_LOCAL int wolfEtsSvc_DoRequest(SvcConn* conn);
WOLFKM_LOCAL int wolfEtsSvc_HandleTimeout(SvcConn* conn);
WOLFKM_LOCAL int wolfEtsSvc_DoNotify(SvcConn* conn);
WOLFKM_LOCAL void wolfEtsSvc_ConnClose(SvcConn* conn);

WOLFKM_LOCAL int wolfEtsSvc_SetVaultFile(SvcInfo* svc, const char* vaultFile);

#ifdef __cplusplus
}
#endif

#endif /* WOLFKM_SVC_ETS_H */
