#include <pspsdk.h>
#include <pspkernel.h>
#include <stdio.h>

#include <vlf.h>

extern unsigned char intraFont_prx_start[], iop_prx_start[], rtc_driver_prx_start[], vlf_prx_start[];
extern unsigned int intraFont_prx_size, iop_prx_size, rtc_driver_prx_size, vlf_prx_size;

extern int app_main(int argc, char *argv[]);

int SetupCallbacks(void) {
    int CallbackThread(SceSize args, void *argp) {
        int exit_callback(int arg1, int arg2, void *common) {
            sceKernelExitGame();
            return 0;
        }
        
        int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
        sceKernelRegisterExitCallback(cbid);
        sceKernelSleepThreadCB();
        return 0;
    }
    
    int thid = sceKernelCreateThread("NTPSP_callback_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0)
        sceKernelStartThread(thid, 0, 0);
        
    return thid;
}

void LoadStartModuleBuffer(const char *path, const void *buf, int size, SceSize args, void *argp) {
    SceUID mod, out;
    
    sceIoRemove(path);
    out = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT, 0777);
    sceIoWrite(out, buf, size);
    sceIoClose(out);
    
    mod = sceKernelLoadModule(path, 0, NULL);
    mod = sceKernelStartModule(mod, args, argp, NULL, NULL);
    sceIoRemove(path);
}

int start_thread(SceSize args, void *argp) {
    char *path = (char *)argp;
    int last_trail = -1;
    
    for(int i = 0; path[i]; i++) {
        if (path[i] == '/')
            last_trail = i;
    }
    
    if (last_trail >= 0)
        path[last_trail] = 0;
    
    sceIoChdir(path);
    path[last_trail] = '/';

    LoadStartModuleBuffer("intraFont.prx", intraFont_prx_start, intraFont_prx_size, args, argp);
    LoadStartModuleBuffer("iop.prx", iop_prx_start, iop_prx_size, args, argp);
    LoadStartModuleBuffer("rtc_driver.prx", rtc_driver_prx_start, rtc_driver_prx_size, args, argp);
    LoadStartModuleBuffer("vlf.prx", vlf_prx_start, vlf_prx_size, args, argp);
    
    vlfGuiInit(15000, app_main);
    return sceKernelExitDeleteThread(0);
}

int module_start(SceSize args, void *argp) {
    SetupCallbacks();

    SceUID thread = sceKernelCreateThread("NTPSP_start_thread", start_thread, 0x10, 0x4000, 0, NULL);
    if (thread < 0)
        return thread;
        
    sceKernelStartThread(thread, args, argp);
    return 0;
}
