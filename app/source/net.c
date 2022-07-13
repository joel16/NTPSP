#include <pspkernel.h>
#include <pspnet.h>
#include <pspnet_apctl.h>
#include <pspnet_inet.h>
#include <psputility.h>
#include <stdio.h>

#include "utils.h"

int netInit(void) {
    int ret = 0;

    sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
    sceUtilityLoadNetModule(PSP_NET_MODULE_INET);

    if (R_FAILED(ret = sceNetInit(128 * 1024, 42, 4 * 1024, 42, 4 * 1024))) {
        snprintf(g_err_string, 64, "sceNetInit() failed: 0x%08x\n", ret);
        return ret;
    }
    
    if (R_FAILED(ret = sceNetInetInit())) {
        snprintf(g_err_string, 64, "sceNetInetInit() failed: 0x%08x\n", ret);
        return ret;
    }
    
    if (R_FAILED(ret = sceNetApctlInit(0x8000, 48))) {
        snprintf(g_err_string, 64, "sceNetApctlInit() failed: 0x%08x\n", ret);
        return ret;
    }
    
    return 0;
}

void netExit(void) {
    sceNetApctlTerm();
    sceNetInetTerm();
    sceNetTerm();
    sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
    sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
}
