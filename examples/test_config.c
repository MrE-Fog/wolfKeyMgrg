/* test_config.c
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

#include "examples/test_config.h"

static EtsClientCtx* gEtsClient = NULL;

/* ETS Client Helpers */
int ets_client_connect(const char* urlStr)
{
    int ret = 0;
    static char urlStrCopy[HTTP_MAX_URI];
    static HttpUrl url;

    /* setup key manager connection */
    if (gEtsClient == NULL) {
        wolfEtsClientInit();

        gEtsClient = wolfEtsClientNew();
        if (gEtsClient) {
            wolfEtsClientAddCA(gEtsClient, ETS_TEST_CLIENT_CA);
            wolfEtsClientSetKey(gEtsClient,
                ETS_TEST_CLIENT_KEY, ETS_TEST_CLIENT_PASS,
                ETS_TEST_CLIENT_CERT, WOLFSSL_FILETYPE_PEM);

            if (urlStr) {
                strncpy(urlStrCopy, urlStr, (HTTP_MAX_URI - 1));
                memset(&url, 0, sizeof(url));
                wolfHttpUrlDecode(&url, urlStrCopy);
            }

            ret = wolfEtsClientConnect(gEtsClient, url.domain, url.port,
                ETS_TEST_TIMEOUT_MS);
            if (ret != 0) {
                printf("Error connecting to ETS server %s! %d\n", urlStr, ret);
                ets_client_cleanup();
            }
        }
        else {
            ret = WOLFKM_BAD_MEMORY;
        }
    }
    return ret;
}

int ets_client_get(const char* urlStr, EtsKey* key, int keyType)
{
    int ret;
    const char* keyStr = wolfEtsKeyGetTypeStr(keyType);
    if (keyStr == NULL) {
        return WOLFKM_NOT_COMPILED_IN;
    }

    ret = ets_client_connect(urlStr);
    if (ret == 0 && key != NULL) {
        /* Get and set a static ephemeral for each supported key type */
        ret = wolfEtsClientGet(gEtsClient, key, keyType, NULL, NULL,
            ETS_TEST_TIMEOUT_MS);

        /* negative means error */
        if (ret < 0) {
            printf("Error getting ETS %s static ephemeral key! %d\n", keyStr, ret);
            ets_client_cleanup();
        }
        /* positive return means new key returned */
        else if (ret > 0) {
            printf("Got ETS %s static ephemeral key (%d bytes)\n", keyStr, key->responseSz);
            wolfEtsKeyPrint(key);
        }
        /* zero means, same key is used - key has not changed */
        else {
            printf("ETS %s Key Cached (valid for %lu sec)\n",
                keyStr, key->expires - wolfGetCurrentTimeT());
        }
    }
    return ret;
}

int ets_client_get_all(const char* urlStr, ets_client_key_cb cb,
    void* cbCtx)
{
    int ret = WOLFKM_NOT_COMPILED_IN;
#ifdef HAVE_ECC
    static EtsKey keyEcc;
#endif
#ifndef NO_DH
    static EtsKey keyDh;
#endif
#ifdef HAVE_CURVE25519
    static EtsKey keyX25519;
#endif
#ifdef HAVE_CURVE448
    static EtsKey keyX448;
#endif

    /* Get static ephemeral for each supported key type */
#ifdef HAVE_ECC
    if (ret == 0 || ret == WOLFKM_NOT_COMPILED_IN) {
        ret = ets_client_get(urlStr, &keyEcc, ETS_KEY_TYPE_SECP256R1);
        if (ret >= 0 && cb != NULL) {
            ret = cb(&keyEcc, cbCtx);
        }
    }
#endif
#ifndef NO_DH
    if (ret == 0 || ret == WOLFKM_NOT_COMPILED_IN) {
        ret = ets_client_get(urlStr, &keyDh, ETS_KEY_TYPE_FFDHE_2048);
        if (ret >= 0 && cb != NULL) {
            ret = cb(&keyDh, cbCtx);
        }
    }
#endif
#ifdef HAVE_CURVE25519
    if (ret == 0 || ret == WOLFKM_NOT_COMPILED_IN) {
        ret = ets_client_get(urlStr, &keyX25519, ETS_KEY_TYPE_X25519);
        if (ret >= 0 && cb != NULL) {
            ret = cb(&keyX25519, cbCtx);
        }
    }
#endif
#ifdef HAVE_CURVE448
    if (ret == 0 || ret == WOLFKM_NOT_COMPILED_IN) {
        ret = ets_client_get(urlStr, &keyX448, ETS_KEY_TYPE_X448);
        if (ret >= 0 && cb != NULL) {
            ret = cb(&keyX448, cbCtx);
        }
    }
#endif
    if (ret > 0 || ret == WOLFKM_NOT_COMPILED_IN) {
        ret = 0; /* success */
    }

    return ret;
}

int ets_client_find(const char* urlStr, EtsKey* key, int namedGroup,
    const byte* pub, word32 pubSz)
{
    int ret;

    if (key == NULL)
        return BAD_FUNC_ARG;

    ret = ets_client_connect(urlStr);
    if (ret == 0) {
        char fpStr[ETS_MAX_FINGERPRINT_STR];
        word32 fpStrSz = (word32)sizeof(fpStr);

        ret = wolfEtsCalcTlsFingerprint((EtsKeyType)namedGroup, pub, pubSz,
            fpStr, &fpStrSz);
        if (ret == 0) {
            ret = wolfEtsClientFind(gEtsClient, key, namedGroup, fpStr,
                NULL, ETS_TEST_TIMEOUT_MS);
        }
        if (ret < 0) {
            printf("Error finding ETS static ephemeral key! %d\n", ret);
            ets_client_cleanup();
        }
        else {
            printf("Found ETS static ephemeral key (%d bytes)\n",
                key->responseSz);
            wolfEtsKeyPrint(key);
        }
    }
    return ret;
}

void ets_client_cleanup(void)
{
    if (gEtsClient) {
        wolfEtsClientFree(gEtsClient);
        gEtsClient = NULL;

        wolfEtsClientCleanup();
    }
}
