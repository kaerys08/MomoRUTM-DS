/*---------------------------------------------------------------------------------

	default ARM7 core

		Copyright (C) 2005 - 2010
		Michael Noland (joat)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.

	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.

	3.	This notice may not be removed or altered from any source
		distribution.

---------------------------------------------------------------------------------*/
#include <calico.h>
#include <nds.h>
#include <maxmod7.h>

void vBlank(void);

static Thread s_myServerThread;
alignas(8) static u8 s_myServerThreadStack[1024];

int vcounter=0;
u32 sTitleKaho_frame = 0;

void vBlank(void){
	++vcounter;
   if (vcounter==10){
	sTitleKaho_frame = 1;
   }
   if (vcounter==20){
	sTitleKaho_frame = 2;
   }
   if (vcounter==30){
	sTitleKaho_frame = 3;
   }
   if (vcounter==40){
	sTitleKaho_frame = 4;
   }
   if (vcounter==50){
	sTitleKaho_frame = 0;
      vcounter=0;
   }
}

//---------------------------------------------------------------------------------
static int myServerThreadMain(void* arg) {
//---------------------------------------------------------------------------------
	// Set up PXI mailbox, used to receive PXI command words
	Mailbox mb;
	u32 mb_slots[4];
	mailboxPrepare(&mb, mb_slots, sizeof(mb_slots)/4);
	pxiSetMailbox(PxiChannel_User0, &mb);

	// Main PXI message loop
	for (;;) {
		// Receive a message
		u32 msg = mailboxRecv(&mb);
		u32 retval = 0;

		switch (msg) {
			default: break;

			// Command 0: Read the firmware chip's JEDEC identifier
			case 0: {
				spiLock();
				nvramReadJedec(&retval);
				spiUnlock();

				break;
			}

			// Command 1: Read touch pressure values (only works on DS Phat/DS Lite)
			case 1: {
				unsigned z1 = 0, z2 = 0;

				if (!cdcIsTwlMode()) {
					spiLock();
					z1 = tscReadChannel12(TscChannel_Z1);
					z2 = tscReadChannel12(TscChannel_Z2);
					spiUnlock();
				}

				// Pack the two 12-bit values into the reply value
				retval = z1 | (z2 << 12);
				break;
			}

			case 2: {
				spiLock();
				retval = sTitleKaho_frame;
				spiUnlock();

				break;
			}
		}

		// Send a reply back to the ARM9
		pxiReply(PxiChannel_User0, retval);
	}

	return 0;
}

//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------

	// Read settings from NVRAM
	envReadNvramSettings();

	// Set up extended keypad server (X/Y/hinge)
	keypadStartExtServer();

	// Configure and enable VBlank interrupt
	lcdSetIrqMask(DISPSTAT_IE_ALL, DISPSTAT_IE_VBLANK);
	irqSet(IRQ_VBLANK, vBlank);
	irqEnable(IRQ_VBLANK);

	// Set up RTC
	rtcInit();
	rtcSyncTime();

	// Initialize power management
	pmInit();

	// Set up block device peripherals
	blkInit();

	// Set up touch screen driver
	touchInit();
	touchStartServer(80, MAIN_THREAD_PRIO);

	// Set up sound and mic driver
	soundStartServer(MAIN_THREAD_PRIO-0x10);
	micStartServer(MAIN_THREAD_PRIO-0x18);

	// Set up wireless manager
	wlmgrStartServer(MAIN_THREAD_PRIO-8);

	// Set up Maxmod
	mmInstall(MAIN_THREAD_PRIO+1);

	threadPrepare(&s_myServerThread, myServerThreadMain, NULL, &s_myServerThreadStack[sizeof(s_myServerThreadStack)], MAIN_THREAD_PRIO);
	threadStart(&s_myServerThread);

	// Keep the ARM7 mostly idle
	while (pmMainLoop()) {
		threadWaitForVBlank();
	}

	return 0;
}
