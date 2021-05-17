#include "bkKeyboard.h"
#include "bkEmu.h"
#include "ps2Input.h"
#include "bkEnvironment.h"
#include "defines.h"

#define TTY_VECTOR      060
#define TTY_VECTOR2     0274

extern bkEnvironment Environment;

char keyMap1[] = {
	// A    B    C    D    E    F    G    H    I    J
	  'f', 'i', 's', 'w', 'u', 'a', 'p', 'r', '{', 'o',

	// K    L    M    N    O    P    Q    R    S    T
	  'l', 'd', 'x', 't', '}', 'z', 'j', 'k', 'y', 'e',

	// U    V    W    X    Y    Z
	  'g', 'm', 'c', '~', 'n', 'q'
};
char keyMap2[] = {
	// a    b    c    d    e    f    g    h    i    j
      'F', 'I', 'S', 'W', 'U', 'A', 'P', 'R', '[', 'O',

	// k    l    m    n    o    p    q    r    s    t
	  'L', 'D', 'X', 'T', ']', 'Z', 'J', 'K', 'Y', 'E',

	// u    v    w    x    y    z
	  'G', 'M', 'C', '^', 'N', 'Q'
};

int tty_finish(d_word info)
{
	service(( info & 0200 ) ? TTY_VECTOR2 : TTY_VECTOR);
	//tty_pending_int = 0;
	return OK;
}

uint8_t convertSymbol(uint8_t symbol, bool returnItself)
{
	if (symbol >= 'A' && symbol <= 'Z')
	{
		return keyMap1[symbol - 'A'];
	}

	if (symbol >= 'a' && symbol <= 'z')
	{
		return keyMap2[symbol - 'a'];
	}

	switch (symbol)
	{
	case '"': // Э
		return '|';
	case '\'': // э
		return '\\';
	case '>': // Ю
		return '`';
	case '.': // ю
		return '@';
	case '{': // Х
		return 'h';
	case '[': // х
		return 'H';
	case '}': // Ъ
		return '\x7F';
	case ']': // ъ
		return '_';
	case ':': // Ж
		return 'v';
	case ';': // ж
		return 'V';
	case '<': // Б
		return 'b';
	case ',': // б
		return 'B';
	case '?':
		return ',';
	case '/':
		return '.';
	}

	return returnItself ? symbol : '\0';
}

bool OnKey(uint32_t scanCode, bool isKeyUp)
{
	if (isKeyUp)
	{
		return false;
	}

	uint8_t symbol = '\0';

	if (ModifierKeyState & (ModifierKeys::LeftAlt | ModifierKeys::RightAlt))
	{
		// АР2

		switch (scanCode)
		{
		case KEY_COMMA:
			// Inverse on / off
			symbol = 156;
			break;
		case KEY_DIV:
			// Underscore on / off
			symbol = 159;
			break;
		case KEY_ESC: // СБР
			// Extended memory on / off
			symbol = 140;
			break;
		default:
			symbol = Ps2_ConvertScancode(scanCode);
			symbol = convertSymbol(symbol, false);
			if (symbol != '\0')
			{
				symbol += 89;
			}
			break;
		}
	}
	else
	{
		switch (scanCode)
		{
		case KEY_INSERT: // |=>
			symbol = 23;
			break;
		case KEY_ESC: // СБР
			symbol = 12;
			break;
		case KEY_LEFTARROW:
			symbol = 0x08;
			break;
		case KEY_RIGHTARROW:
			symbol = 0x19;
			break;
		case KEY_UPARROW:
			symbol = 0x1A;
			break;
		case KEY_DOWNARROW:
			symbol = 0x1B;
			break;
		case KEY_BACKSPACE:
			symbol = 0x18;
			break;
		case KEY_TAB:
			symbol = 20;
			break;
		case KEY_ENTER:
		case KEY_KP_ENTER:
			symbol = 0x0A;
			break;
		case KEY_LEFTCONTROL: // РУС
		case KEY_L_GUI:
			symbol = 0x0E;
			break;
		case KEY_RIGHTCONTROL: // ЛАТ
		case KEY_R_GUI:
			symbol = 0x0F;
			break;
		case KEY_ALT: // АР2
			symbol = 0x0F;
			break;
		default:
			symbol = Ps2_ConvertScancode(scanCode);
			if (Environment.ReadByte(0x0023) == 0x80)
			{
				// РУС
				symbol = convertSymbol(symbol, true);
			}

			break;
		}

		if (symbol == '\0')
		{
			return false;
		}
	}

	Environment.WriteByte(0177660, Environment.ReadByte(0177660) | 0x80);
	Environment.WriteByte(0177716, Environment.ReadByte(0177716) | 0x04);
	if (isKeyUp)
	{
		Environment.WriteByte(0177716, Environment.ReadByte(0177716) & ~0x80);
	}
	else
	{
		Environment.WriteByte(0177716, Environment.ReadByte(0177716) | 0x80);
	}
	Environment.WriteByte(0177662, symbol & 0x7F);

	ev_register(TTY_PRI, tty_finish, 0, symbol);

	return true;
}
