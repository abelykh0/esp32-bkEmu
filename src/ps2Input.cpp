#include <ctype.h>
#include <map>

#include "ps2Input.h"

using namespace fabgl;

static Keyboard* _keyboard;
static VirtualKeyItem _virtualKeyItem;
uint8_t ModifierKeyState = 0;

void Ps2_Initialize(PS2Controller* inputController)
{
	_keyboard = inputController->keyboard();
}

static void updateModifiers(VirtualKey virtualKey, bool keyDown)
{
    switch (virtualKey)
    {
        case VirtualKey::VK_LCTRL:
            if (keyDown)
            {
                ModifierKeyState |= ModifierKeys::LeftControl;
            }
            else
            {
                ModifierKeyState &= ~ModifierKeys::LeftControl; 
            }
            break;
        
        case VirtualKey::VK_LSHIFT:
            if (keyDown)
            {
                ModifierKeyState |= ModifierKeys::LeftShift;
            }
            else
            {
                ModifierKeyState &= ~ModifierKeys::LeftShift; 
            }
            break;
        
        case VirtualKey::VK_LALT:
            if (keyDown)
            {
                ModifierKeyState |= ModifierKeys::LeftAlt;
            }
            else
            {
                ModifierKeyState &= ~ModifierKeys::LeftAlt; 
            }
            break;
        
        case VirtualKey::VK_LGUI:
            if (keyDown)
            {
                ModifierKeyState |= ModifierKeys::LeftWindows;
            }
            else
            {
                ModifierKeyState &= ~ModifierKeys::LeftWindows; 
            }
            break;
        
        case VirtualKey::VK_RCTRL:
            if (keyDown)
            {
                ModifierKeyState |= ModifierKeys::RightControl;
            }
            else
            {
                ModifierKeyState &= ~ModifierKeys::RightControl; 
            }
            break;
        
        case VirtualKey::VK_RSHIFT:
            if (keyDown)
            {
                ModifierKeyState |= ModifierKeys::RightShift;
            }
            else
            {
                ModifierKeyState &= ~ModifierKeys::RightShift; 
            }
            break;
        
        case VirtualKey::VK_RALT:
            if (keyDown)
            {
                ModifierKeyState |= ModifierKeys::RightAlt;
            }
            else
            {
                ModifierKeyState &= ~ModifierKeys::RightAlt; 
            }
            break;
        
        case VirtualKey::VK_RGUI:
            if (keyDown)
            {
                ModifierKeyState |= ModifierKeys::RightWindows;
            }
            else
            {
                ModifierKeyState &= ~ModifierKeys::RightWindows; 
            }
            break;
        
        default:
            break;
    }
}

VirtualKeyItem* KeyboardGetNextVirtualKey()
{
	if (!_keyboard->getNextVirtualKey(&_virtualKeyItem, 0))
	{
		return nullptr;		
	}

	updateModifiers(_virtualKeyItem.vk, _virtualKeyItem.down);
	
	return &_virtualKeyItem;
}
