#include <pspkernel.h>
#include <psputility.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <vlf.h>

#include "net.h"
#include "ntp.h"
#include "utils.h"

PSP_MODULE_INFO("NTPSP", PSP_MODULE_USER, VERSION_MAJOR, VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(0);

extern unsigned char logo_png_start[];
extern unsigned int logo_png_size;

char g_err_string[64];
static char g_message_text[128];
static pspTime g_psp_time_ntp = { 0 };
static VlfText g_title_text;
static VlfPicture g_logo_pic, g_title_pic;
static bool g_running = true;

static void vlfSetTitle(char *fmt, ...) {
    va_list list;
    char text[64];	
    
    va_start(list, fmt);
    vsprintf(text, fmt, list);
    va_end(list);
    
    if (g_title_text != NULL)
        vlfGuiRemoveText(g_title_text);
        
    if (g_title_pic != NULL)
        vlfGuiRemovePicture(g_title_pic);
        
    g_title_text = vlfGuiAddText(0, 0, text);
    g_title_pic = vlfGuiAddPictureResource("ps3scan_plugin.rco", "tex_infobar_icon", 4, -2);
    vlfGuiSetTitleBar(g_title_text, g_title_pic, 1, 0);
}

int menuSelection(int selection) {
    int ret = 0;

    switch (selection) {
        case 0:
            int button_res = vlfGuiMessageDialog(
                "This action requires an active internet connection and will alter your system clock. Do you wish to continue?",
                VLF_MD_TYPE_NORMAL | VLF_MD_BUTTONS_YESNO
            );

            if (button_res == 1) {
                if (R_FAILED(ret = netInit())) {
                    snprintf(g_message_text, 128, "Failed to initialize net modules\n%s", g_err_string);
                }
                else {
                    int ret = vlfGuiNetConfDialog();
                    if (ret != 0) {
                        snprintf(g_message_text, 128, "Failed to establish connection\n 0x%08x\n", ret);
                    }
                    else {
                        if (R_FAILED(ret = ntpGetTime(&g_psp_time_ntp))) {
                            snprintf(g_message_text, 128, "Failed to sync time with NTP server\n%s", g_err_string);
                        }
                        else {
                            int time_format = getSystemParamDateTimeFormat();

                            if (time_format == PSP_SYSTEMPARAM_TIME_FORMAT_12HR) {
                                int hour = ((g_psp_time_ntp.hour % 12) == 0)? 12 : g_psp_time_ntp.hour % 12;

                                snprintf(g_message_text, 128, "Time received from NTP server\n%04d-%02d-%02d %02d:%02d:%02d %s",
                                    g_psp_time_ntp.year, g_psp_time_ntp.month, g_psp_time_ntp.day, hour,
                                    g_psp_time_ntp.minutes, g_psp_time_ntp.seconds, (hour / 12)? "AM" : "PM"
                                );
                            }
                            else {
                                snprintf(g_message_text, 128, "Time received from NTP server\n%04d-%02d-%02d %02d:%02d:%02d",
                                    g_psp_time_ntp.year, g_psp_time_ntp.month, g_psp_time_ntp.day, g_psp_time_ntp.hour,
                                    g_psp_time_ntp.minutes, g_psp_time_ntp.seconds
                                );
                            }
                        }
                    }
                }
                
                vlfGuiMessageDialog(g_message_text, VLF_MD_BUTTONS_NONE);
                netExit();
            }

            break;

        case 1:
            g_running = false;
            break;
    }

    return VLF_EV_RET_NOTHING;
}

int app_main(int argc, char *argv[]) {
    const int num_menu_items = 2;
    char *item_labels[] = { "Sync Clock", "Exit" };
    
    vlfGuiSystemSetup(1, 1, 1);
    vlfSetTitle("NTPSP v%d.%d", VERSION_MAJOR, VERSION_MINOR);
    g_logo_pic = vlfGuiAddPicture(logo_png_start, logo_png_size, 45, 36);
    vlfGuiLateralMenu(num_menu_items, item_labels, 0, menuSelection, 120);
    
    while (g_running) {
        vlfGuiDrawFrame();
    }

    sceKernelExitGame();
    return 0;
}
