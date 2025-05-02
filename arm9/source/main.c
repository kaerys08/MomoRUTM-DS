#include <nds.h>
//#include <fat.h>
//#include <filesystem.h>
//#include <libntr.h>

#include <stdio.h>

int main(int argc, char** argv){
    //videoSetMode(MODE_0_2D);
    //vramSetBankA(VRAM_A_MAIN_BG_0x06000000);

    consoleDemoInit();

    /*if(!nitroFSInit(NULL)){
		iprintf("[ERROR] Nitro FileSystem failed\n");
	}

    if(ReadFile("nitro:/bg/8-1.NCLR")){
        iprintf("success\n");
    }*/

    /*u16* offset = paletteAllocateColors(&pal_state_main, 15);
    u16* offset2 = paletteAllocateColors(&pal_state_main, 17);
    u16* offset3 = paletteAllocateColors(&pal_state_main, 8);
    iprintf("offset: %04X %04X %04X\n", offset, offset2, offset3);*/

    while(pmMainLoop()){
        swiWaitForVBlank();

        /*scanKeys();
		if (keysDown()&KEY_START) break;*/
    }
    return 0;
}