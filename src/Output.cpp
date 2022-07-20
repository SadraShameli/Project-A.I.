#include "Output.h"

Output::Output()
{
    for (Pin &_Pin : pin)
    {
        gpio_set_direction(_Pin.tag, GPIO_MODE_OUTPUT);
        gpio_set_level(_Pin.tag, 0);
        vTaskDelay(250 / portTICK_PERIOD_MS);
        gpio_set_level(_Pin.tag, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelay(250 / portTICK_PERIOD_MS);
    for (Pin &_Pin : pin)
        gpio_set_level(_Pin.tag, 0);
}

void Output::toggle(const enum Outputs &pinTag, bool targetState)
{
    for (Pin &_Pin : pin)
    {
        if ((Outputs)_Pin.tag == pinTag)
        {
            gpio_set_level(_Pin.tag, targetState);
            _Pin.interval = 999999;
            return;
        }
    }
}

void Output::blink(const enum Outputs &pinTag, const int &blinkTime, bool continuousBlinking)
{
    for (Pin &_Pin : pin)
    {
        if ((Outputs)_Pin.tag == pinTag)
        {
            _Pin.state = true;
            _Pin.interval = blinkTime;
            _Pin.continuous = continuousBlinking;
            _Pin.updateTime = 0;
            update();

            return;
        }
    }
}

void Output::setContinuity(const enum Outputs &pinTag, bool continuousBlinking)
{
    for (Pin &_Pin : pin)
    {
        if ((Outputs)_Pin.tag == pinTag)
        {
            _Pin.continuous = continuousBlinking;
            return;
        }
    }
}

void Output::update()
{
    clock_t currentTime = clock();
    for (Pin &_Pin : pin)
    {
        if ((currentTime - _Pin.updateTime) > _Pin.interval)
        {
            _Pin.updateTime = currentTime;
            if (_Pin.continuous)
            {
                _Pin.state = !_Pin.state;
                gpio_set_level(_Pin.tag, _Pin.state);
            }
            else
            {
                if (_Pin.state)
                {
                    gpio_set_level(_Pin.tag, 1);
                    _Pin.state = false;
                }
                else
                    gpio_set_level(_Pin.tag, 0);
            }
        }
    }
}

Output output;