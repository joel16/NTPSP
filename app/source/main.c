#include <pspkernel.h>
#include <stdarg.h>
#include <stdio.h>
#include <vlf.h>

#include "net.h"
#include "ntp.h"

PSP_MODULE_INFO("NTPSP", PSP_MODULE_USER, VERSION_MAJOR, VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(0);

extern unsigned char logo_png_start[];
extern unsigned int logo_png_size;

static pspTime psp_time_ntp = { 0 };
static VlfText title_text;
static VlfPicture logo, title_pic;

static void vlfSetTitle(char *fmt, ...) {
    va_list list;
    char text[64];	
    
    va_start(list, fmt);
    vsprintf(text, fmt, list);
    va_end(list);
    
    if (title_text != NULL)
        vlfGuiRemoveText(title_text);
        
    if (title_pic != NULL)
        vlfGuiRemovePicture(title_pic);
        
    title_text = vlfGuiAddText(0, 0, text);
    title_pic = vlfGuiAddPictureResource("ps3scan_plugin.rco", "tex_infobar_icon", 4, -2);
    vlfGuiSetTitleBar(title_text, title_pic, 1, 0);
}

int menuSelection(int selection) {
    switch (selection) {
        case 0:
            vlfGuiNetConfDialog();
            ntpGetTime(&psp_time_ntp);
            break;

        case 1:
            sceKernelExitGame();
            break;
    }

    return VLF_EV_RET_NOTHING;
}

int app_main(int argc, char *argv[]) {
    netInit();

    const int num_menu_items = 2;
    char *item_labels[] = { "Sync Clock", "Exit" };
    
    vlfGuiSystemSetup(1, 1, 1);
    vlfSetTitle("NTPSP v%d.%d", VERSION_MAJOR, VERSION_MINOR);
    logo = vlfGuiAddPicture(logo_png_start, logo_png_size, 45, 36); // png's are now supported
    vlfGuiLateralMenu(num_menu_items, item_labels, 0, menuSelection, 120);
    
    while (1) {
        vlfGuiDrawFrame();
    }
    
    netExit();
    return 0;
}
