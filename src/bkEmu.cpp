// BK-0010 page mapping
// ====================
// 0000..3FFF RAM (16K)
//   0020 Screen mode 0 - 512x256, FF - 256x256
//   0021 Screen inversion 0 - off, FF - on
// 4000..7FFF Video RAM (16K)
// 8000..9FFF ROM Monitor and drivers (8K)
// A000..FF7F ROM Basic (24448 bytes)
// FF80..FFFF I/O ports
//   FFB0 Keyboard status
//   FFB2 Keyboard key code
//   FFB3 Scroll register
//   FFC6 System Timer counter start value
//   FFC8 System Timer counter
//   FFCA System Timer control

#include "settings.h"
#include "VideoController.h"
#include "bkEnvironment.h"
#include "bkEmu.h"
#include "bkKeyboard.h"
#include "basic.h"
#include "monitor.h"
#include "ps2Input.h"
#include "defines.h"

pdp_regs pdp;
extern "C" void timing(pdp_regs* p);
flag_t bkmodel = 0;
flag_t io_stop_happened;
unsigned short last_branch;

const int TICK_RATE = 3000000; // CPU clock speed

static VideoController Screen;
bkEnvironment Environment;
static fabgl::PS2Controller* InputController;

static void startKeyboard()
{
	InputController = new fabgl::PS2Controller();
	InputController->begin(PS2Preset::KeyboardPort0);
	Ps2_Initialize(InputController);
}

void EmulatorTaskMain(void *unused)
{
	Environment.Initialize();
	Screen.Initialize(&Environment);

	startKeyboard();

	Screen.Start(RESOLUTION);

	bk_reset();

	// Loop
	while (true)
	{
		vTaskDelay(1); // important to avoid task watchdog timeouts

		bk_loop();
	}
}

int32_t bk_loop()
{
	pdp_regs* p = &pdp;
	register int result; 
	int result2 = OK; 
	int rtt = 0; 
	int32_t returnValue = 0;

	uint32_t frames = Screen.Frames + 1;
	//Uint32 last_screen_update = SDL_GetTicks();
	//double timing_delta = ticks - SDL_GetTicks() * (TICK_RATE/1000.0);

	while (Screen.Frames < frames)
	{
		result = ll_word(p, p->regs[PC], &p->ir);
		p->regs[PC] += 2;
		if (result == OK)
		{
			result = (itab[p->ir >> 6].func)(p);
			timing(p);
		}

		if (result != OK)
		{
			switch (result)
			{
			case BUS_ERROR: 
				ticks += 64;
				break;
			case ODD_ADDRESS:
				result2 = service((d_word) 04);
				break;
			case CPU_ILLEGAL: 
				result2 = service((d_word) 010);
				break;
			case CPU_BPT: 
				result2 = service((d_word) 014);
				break;
			case CPU_EMT: 
				result2 = service((d_word) 030);
				break;
			case CPU_TRAP: 
				result2 = service((d_word) 034);
				break;
			case CPU_IOT: 
				result2 = service((d_word) 020);
				break;
			case CPU_WAIT:
				in_wait_instr = 1;
				result2 = OK;
				break;
			case CPU_RTT:
				rtt = 1;
				result2 = OK;
				break;
			case CPU_HALT:
				io_stop_happened = 4;
				result2 = service((d_word) 004);
				break;
			default:
				// Unexpected return
				//flag = 0;
				result2 = OK;
				break;
			}
			if (result2 != OK)
			{
				// Double trap
				ll_word(p, 0177716, &p->regs[PC]);
				p->regs[PC] &= 0177400;
			}
		}

		// Keyboard input
		int32_t scanCode = Ps2_GetScancode();
		if (scanCode > 0)
		{
			if ((scanCode & 0xFF00) == 0xF000)
			{
				// key up

				scanCode = ((scanCode & 0xFF0000) >> 8 | (scanCode & 0xFF));
				if (!OnKey(scanCode, true))
				{
					returnValue = scanCode;
				}
			}
			else
			{
				// key down
				OnKey(scanCode, false);
			}
		}

		if ((p->psw & 020) && (rtt == 0))
		{ 
			if (service((d_word) 014) != OK)
			{
				// Double trap
				ll_word(p, 0177716, &p->regs[PC]);
				p->regs[PC] &= 0177400;
				p->regs[SP] = 01000; 
			}
		}
		rtt = 0;
		p->total++;

		//if (nflag)
		//	sound_flush();

	//		if (bkmodel && ticks >= ticks_timer) {
	//			scr_sync();
	//			if (timer_intr_enabled) {
	//				ev_register(TIMER_PRI, service, 0, 0100);
	//			}
	//			ticks_timer += half_frame_delay;
	//		}

		int priority = (p->psw >> 5) & 7;
		if (pending_interrupts && priority != 7)
		{
			ev_fire(priority);
		}
	}

	return returnValue;
}

void bk_reset()
{
	for (int x = 0; x < 8; ++x)
	{
		pdp.regs[x] = 0;
	}

	pdp.ir = 0;
	//pdp.psw = 0200;

	pdp.regs[PC] = 0x8000;
	//ll_word(&pdp, 0177716, &pdp.regs[PC]);
//	pdp.regs[PC] &= 0177400;

	for (uint32_t* ram = (uint32_t*)Environment.GetPointer(0); ram < (uint32_t*)Environment.GetPointer(0x3FFC); ram++)
	{
		*ram = 0x00FF00FF;
	}
}

extern "C" int ll_byte(pdp_regs* p, c_addr addr, d_byte* byte)
{
	*byte = Environment.ReadByte(addr);
	return OK;
}

extern "C" int ll_word(pdp_regs* p, c_addr addr, d_word* word)
{
	*word = Environment.ReadWord(addr);
	return OK;
}

extern "C" int sl_byte(pdp_regs* p, c_addr addr, d_byte byte)
{
	Environment.WriteByte(addr, byte);
	return OK;
}

extern "C" int sl_word(pdp_regs* p, c_addr addr, d_word word)
{
	return Environment.WriteWord(addr, word);
}

extern "C" void q_reset()
{
}
