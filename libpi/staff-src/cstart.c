#include "rpi.h"

void _cstart() {
    extern int __bss_start__, __bss_end__;
	void notmain();

    custom_loader();

    int* bss = &__bss_start__;
    int* bss_end = &__bss_end__;
 
    while( bss < bss_end )
        *bss++ = 0;

    notmain(); 
	clean_reboot();
}


