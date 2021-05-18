#include "bkEnvironment.h"
#include "settings.h"
#include "monitor.h"
#include "basic.h"
#include "defines.h"

#define BEEPER_PIN 25

// From arduino
#define OUTPUT            0x02
#define LOW               0x0
#define HIGH              0x1
extern "C" void pinMode(uint8_t pin, uint8_t mode);
extern "C" void digitalWrite(uint8_t pin, uint8_t val);

static uint8_t ram[0x4000];
static uint8_t videoRam[0x4000];
static uint8_t ports[0x80];


void bkEnvironment::Initialize()
{
    this->_ram = ram;
    this->_videoRam = videoRam;
    this->_romMonitor = (uint8_t*)monitor;
    this->_romBasic = (uint8_t*)basic;
    this->_ports = ports;

#ifdef BEEPER
    pinMode(BEEPER_PIN, OUTPUT);
    digitalWrite(BEEPER_PIN, LOW);
#endif
}

uint8_t bkEnvironment::ReadByte(uint16_t addr)
{
    uint8_t res;
    switch (addr)
    {
        case 0x0000 ... 0x3fff:
            res = this->_ram[addr];
            break;
        case 0x4000 ... 0x7FFF:
            res = this->_videoRam[addr - 0x4000];
            break;
        case 0x8000 ... 0x9FFF:
            res = this->_romMonitor[addr - 0x8000];
            break;
        case 0xA000 ... 0xFF7F:
            res = this->_romBasic[addr - 0xA000];
            break;
        case 0xFF80 ... 0xFFFF:
            res = this->_ports[addr - 0xFF80];
            break;
    }

    return res;
}

uint16_t bkEnvironment::ReadWord(uint16_t addr)
{
    if (addr & 0x1)
    {
        uint8_t byte1 = this->ReadByte(addr);
        uint8_t byte2 = this->ReadByte(addr + 1);
        return (byte2 << 8) | byte1;
    }

    uint16_t res;
    switch (addr)
    {
        case 0x0000 ... 0x3fff:
            res = ((uint16_t*)this->_ram)[addr >> 1];
            break;
        case 0x4000 ... 0x7FFF:
            res = ((uint16_t*)this->_videoRam)[(addr - 0x4000) >> 1];
            break;
        case 0x8000 ... 0x9FFF:
            res = ((uint16_t*)this->_romMonitor)[(addr - 0x8000) >> 1];
            break;
        case 0xA000 ... 0xFF7F:
            res = ((uint16_t*)this->_romBasic)[(addr - 0xA000) >> 1];
            break;
        case 0xFF80 ... 0xFFFF:
            res = ((uint16_t*)this->_ports)[(addr - 0xFF80) >> 1];
            break;
    }

    return res;
}

void bkEnvironment::WriteByte(uint16_t addr, uint8_t data)
{
    switch (addr)
    {
        case 0x0000 ... 0x3fff:
            this->_ram[addr] = data;
            break;
        case 0x4000 ... 0x7FFF:
            this->_videoRam[addr - 0x4000] = data;
            break;
        case 0x8000 ... 0xFF7F:
            // Can't write to ROM
            break;
        case 0xFF80 ... 0xFFFF:
#ifdef BEEPER
            if (addr == 0xFFCE)
            {
                uint8_t sound = (data & 0x40);
                if ((this->_ports[addr - 0xFF80] & 0x40) != sound)
                {
                    digitalWrite(BEEPER_PIN, sound >> 6); 
                }
            }
#endif
            this->_ports[addr - 0xFF80] = data;
            break;
    }
}

int bkEnvironment::WriteWord(uint16_t addr, uint16_t data)
{
	if (addr & 0x1)
	{
        this->WriteByte(addr, (uint8_t)data);
        this->WriteByte(addr + 1, (uint8_t)(data >> 8));
        return ODD_ADDRESS;
	}

    switch (addr)
    {
        case 0x0000 ... 0x3fff:
            ((uint16_t*)this->_ram)[addr >> 1] = data;
            break;
        case 0x4000 ... 0x7FFF:
            ((uint16_t*)this->_videoRam)[(addr - 0x4000) >> 1] = data;
            break;
        case 0x8000 ... 0xFF7F:
            // Can't write to ROM
            break;
        case 0xFF80 ... 0xFFFF:
            this->WriteByte(addr, (uint8_t)data);
            this->WriteByte(addr + 1, (uint8_t)(data >> 8));
            break;
    }

    return OK;
}

uint8_t* bkEnvironment::GetPointer(uint16_t addr)
{
    uint8_t* res;
    switch (addr)
    {
        case 0x0000 ... 0x3fff:
            res = &this->_ram[addr];
            break;
        case 0x4000 ... 0x7FFF:
            res = &this->_videoRam[addr - 0x4000];
            break;
        case 0x8000 ... 0x9FFF:
            res = &this->_romMonitor[addr - 0x8000];
            break;
        case 0xA000 ... 0xFF7F:
            res = &this->_romBasic[addr - 0xA000];
            break;
        case 0xFF80 ... 0xFFFF:
            res = &this->_ports[addr - 0xFF80];
            break;
    }

    return res;
}