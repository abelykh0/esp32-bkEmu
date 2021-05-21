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

	bool isCapsLock = KeyboardIsCapsLockOn();
	if (isCapsLock)
	{
		switch (symbol)
		{
		case '"': // Э
			symbol = '\'';
			break;
		case '\'': // э
			symbol = '"';
			break;
		case '>': // Ю
			symbol = '.';
			break;
		case '.': // ю
			symbol = '>';
			break;
		case '{': // Х
			symbol = '[';
			break;
		case '[': // х
			symbol = '{'; 
			break;
		case ':': // Ж
			symbol = ';';
			break;
		case ';': // ж
			symbol = ':';
			break;
		case '<': // Б
			symbol = ',';
			break;
		case ',': // б
			symbol = '<';
			break;
		case '}': // Ъ
			symbol = ']';
			break;
		case ']': // ъ
			symbol = '}';
			break;
		}
	}

	switch (symbol)
	{
	case '@':
		return '"';
	case '^':
		return ':';
	case '?':
		return ',';
	case '/':
		return '.';

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
	case ':': // Ж
		return 'v';
	case ';': // ж
		return 'V';
	case '<': // Б
		return 'b';
	case ',': // б
		return 'B';
	case '}': // Ъ
		return '\x7F';
	case ']': // ъ
		return '_';
	}

	return returnItself ? symbol : '\0';
}

bool OnKey(fabgl::VirtualKeyItem* virtualKey)
{
	if (virtualKey == nullptr || !virtualKey->down)
	{
		Environment.WriteByte(0177716, Environment.ReadByte(0177716) | 0x80);
		return false;
	}
	
	uint8_t symbol = '\0';

	if (ModifierKeyState & (ModifierKeys::LeftAlt | ModifierKeys::RightAlt))
	{
		// АР2

		switch (virtualKey->vk)
		{
		case VirtualKey::VK_COMMA:
			// Inverse on / off
			symbol = 156;
			break;
		case VirtualKey::VK_SLASH:
			// Underscore on / off
			symbol = 159;
			break;
		case VirtualKey::VK_ESCAPE: // СБР
			// Extended memory on / off
			symbol = 140;
			break;
		case VirtualKey::VK_RALT: // АР2
		case VirtualKey::VK_LALT:
			break;
		default:
			symbol = virtualKey->ASCII;
			if (symbol > 90)
			{
				symbol -= 32;
			}

			symbol = convertSymbol(symbol, false);
			if (symbol != '\0')
			{
				symbol += 96;
			}
			break;
		}
	}
	else
	{
		switch (virtualKey->vk)
		{
		case VirtualKey::VK_INSERT: // |=>
			symbol = 23;
			break;
		case VirtualKey::VK_ESCAPE: // СБР
			symbol = 12;
			break;
		case VirtualKey::VK_LEFT:
			symbol = 0x08;
			break;
		case VirtualKey::VK_RIGHT:
			symbol = 0x19;
			break;
		case VirtualKey::VK_UP:
			symbol = 0x1A;
			break;
		case VirtualKey::VK_DOWN:
			symbol = 0x1B;
			break;
		case VirtualKey::VK_BACKSPACE:
			symbol = 0x18;
			break;
		case VirtualKey::VK_TAB:
			symbol = 20;
			break;
		case VirtualKey::VK_RETURN:
		case VirtualKey::VK_KP_ENTER:
			symbol = 0x0A;
			break;
		case VirtualKey::VK_LCTRL: // РУС
		case VirtualKey::VK_LGUI:
			if ((PrevModifierKeyState & (ModifierKeys::LeftControl | ModifierKeys::LeftWindows)) == 0)
			{
				symbol = 0x0E;
			}
			break;
		case VirtualKey::VK_RCTRL: // ЛАТ
		case VirtualKey::VK_RGUI:
			if ((PrevModifierKeyState & (ModifierKeys::RightControl | ModifierKeys::RightWindows)) == 0)
			{
				symbol = 0x0F;
			}
			break;
		case VirtualKey::VK_RALT: // АР2
		case VirtualKey::VK_LALT:
			break;
		default:
			symbol = virtualKey->ASCII;
			if (Environment.ReadByte(0x0023) == 0x80)
			{
				// РУС
				symbol = convertSymbol(symbol, true);
			}

			break;
		}
	}

	if (symbol == '\0')
	{
		return false;
	}

	Environment.WriteByte(0177660, Environment.ReadByte(0177660) | 0x80);
	Environment.WriteByte(0177716, Environment.ReadByte(0177716) & ~0x80);
	Environment.WriteByte(0177662, symbol & 0x7F);

	ev_register(TTY_PRI, tty_finish, 0, symbol);

	return true;
}
