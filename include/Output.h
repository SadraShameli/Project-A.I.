#pragma once
#include "Definitions.h"
#include "driver/gpio.h"

class Output
{
public:
    enum Outputs
    {
        LedB = 12,
        LedY = 18,
        LedG = 19
    };
    static constexpr int Count = 3;

public:
    Output();
    void update();
    void toggle(const enum Outputs &, bool);
    void blink(const enum Outputs &, const int &, bool = false);
    void setContinuity(const enum Outputs &, bool);

private:
    struct Pin
    {
        gpio_num_t tag;
        clock_t updateTime;
        int interval;
        bool continuous;
        bool state = false;
        Pin(const Outputs &_tag) : tag((gpio_num_t)_tag) {}
    };
    Pin pin[Count]{Outputs::LedB, Outputs::LedY, Outputs::LedG};
};

extern Output output;