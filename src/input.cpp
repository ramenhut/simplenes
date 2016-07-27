
#include "input.h"

namespace nes {

controller::controller()
{
    memset(buttons, 0, 8 * sizeof(bool));
    index = 0;
    strobe = 0;
}

void controller::apply_strobe()
{
    if (strobe & 0x1)
    {
        index = 0;
    }
}

uint8 controller::read()
{
    uint8 result = 0;

    if (index < 8)
    {
        result = buttons[index];
    }

    index++;

    apply_strobe();

    return result;
}

void controller::write(uint8 value)
{
    strobe = value;

    apply_strobe();
}

void controller::set_button(uint8 index, bool state)
{
    if (index < 8)
    {
        buttons[index] = state;
    }
}

bool controller::get_button(uint8 index)
{
    if (index < 8)
    {
        return buttons[index];
    }
}

} // namespace nes