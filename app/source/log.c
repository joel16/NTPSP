#include <pspiofilemgr.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

int debug(const char *format, ...) {
    SceUID log = 0;
    
    if (R_SUCCEEDED(log = sceIoOpen("debug.log", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777))) {
        va_list list;
        char string[256] = {0};
        
        va_start(list, format);
        vsprintf(string, format, list);
        va_end(list);
        
        printf("%s", string);
        
        int ret = 0;
        if (R_FAILED(ret = sceIoWrite(log, string, strlen(string))))
            return ret;
            
        sceIoClose(log);
    }
    
    return 0;
}
