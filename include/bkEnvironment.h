#ifndef __BKENVIRONMENT_INCLUDED__
#define __BKENVIRONMENT_INCLUDED__

// Class for the BK-0010 enviroment (ROM, RAM, I/O)

// 0000..3FFF RAM (16K)
//   0020 (000040) Screen mode 0 - 512x256, FF - 256x256
//   0021 (000041) Screen inversion 0 - off, FF - on

// 4000..7FFF Video RAM (16K)

// 8000..9FFF ROM Monitor and drivers (8K)

// A000..FF7F ROM Basic (24,448 bytes)

// FF80..FFFF I/O ports
//   FFB0 (177660) Keyboard status
//        bit 6 : interrupt enable
//        bit 7 : status, 1 new key code available
//   FFB2 (177662) Keyboard key code
//        bit 0..6 : key code
//   FFB4 (177664) Scroll register
//        bit 0..7 : shift
//        bit 9 : extended memory 0 - on, 1 - off
//   FFC6 System Timer counter start value
//   FFC8 System Timer counter
//   FFCA System Timer control
//   FFCE (177716) Control external devices 
//        bit 6 : beeper


#include <stdint.h>
#include "VideoController.h"

class bkEnvironment
{
private:
    uint8_t* _ram;
    uint8_t* _videoRam;
    uint8_t* _romMonitor;
    uint8_t* _romBasic;
    uint8_t* _ports;

public:
	VideoController* Screen;
    bkEnvironment(VideoController* screen);

    void Initialize();

    uint8_t ReadByte(uint16_t address);
	uint16_t ReadWord(uint16_t address);
	void WriteByte(uint16_t address, uint8_t data);
	void WriteWord(uint16_t address, uint16_t data);
};

#endif

