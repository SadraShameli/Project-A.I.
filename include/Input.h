#pragma once
#include "Definitions.h"
#include "driver/gpio.h"

class Input
{
public:
    enum Inputs
    {
        Refresh = 32,
        Up = 33,
        Down = 25,
        Blue = 26,
        Yellow = 27,
        Green = 14
    };
    static constexpr int Count = 6;

public:
    Input();
    void update();
    bool getPinState(const enum Inputs &);

private:
    struct Pin
    {
        gpio_num_t tag;
        bool state;
        bool locked;
        Pin(const Inputs &_tag) : tag((gpio_num_t)_tag) {}
    };
    Pin pin[Count]{Inputs::Refresh, Inputs::Up, Inputs::Down, Inputs::Blue, Inputs::Yellow, Inputs::Green};
};

extern Input input;