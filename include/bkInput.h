#ifndef BKINPUT_H
#define BKINPUT_H

#include <stdint.h>

extern uint8_t port0177660;
extern uint8_t port0177662;
extern uint16_t port0177716;

bool OnKey(uint32_t scanCode, bool isKeyUp);

#endif  // DRAW4_H
