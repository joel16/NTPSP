#include <pspsdk.h>

PSP_MODULE_INFO("audio_driver", PSP_MODULE_KERNEL, 1, 3);
PSP_NO_CREATE_MAIN_THREAD();

int sceRtcSetCurrentTick(u64 *tick);

int pspRtcSetCurrentTick(u64 *tick) {
	u32 k1 = pspSdkSetK1(0);
	int ret = sceRtcSetCurrentTick(tick);
	pspSdkSetK1(k1);
	return ret;
}

int module_start(SceSize args, void *argp) {
	return 0;
}

int module_stop(void) {
	return 0;
}
