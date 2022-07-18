#include <pspkernel.h>
#include <psputility.h>

#include "utils.h"

int getSystemParamDateTimeFormat(void) {
    int value = 0;

    if (sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_TIME_FORMAT, &value) != PSP_SYSTEMPARAM_RETVAL_FAIL) {
        return value;
    }
    
    return PSP_SYSTEMPARAM_TIME_FORMAT_24HR; // Assume 24 HR format
}
