#include "Input.h"

Input::Input()
{
    for (Pin &_Pin : pin)
    {
        gpio_set_direction(_Pin.tag, GPIO_MODE_INPUT);
        gpio_set_pull_mode(_Pin.tag, GPIO_PULLUP_ONLY);
    }
}

bool Input::getPinState(const enum Inputs &pinTag)
{
    for (Pin &_Pin : pin)
    {
        if ((Inputs)_Pin.tag == pinTag && _Pin.state)
        {
            _Pin.state = false;
            return true;
        }
    }
    return false;
}

void Input::update()
{
    for (Pin &_Pin : pin)
    {
        if (gpio_get_level(_Pin.tag) && _Pin.locked)
        {
            _Pin.state = false;
            _Pin.locked = false;
        }
        else if (!gpio_get_level(_Pin.tag) && !_Pin.locked)
        {
            _Pin.locked = true;
            _Pin.state = true;
        }
    }
}

Input input;