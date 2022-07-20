#pragma once
#include "Definitions.h"

enum class Colors
{
    White,
    Blue,
    Orange,
    Green,
    Red,
    Black,
};

class LedBar
{
public:
    void startup();
    void update();

    void setColorFade(const int &, const int &, const int &);
    void setColorWipe(const int &, const int &, const int &);
    void setColorWipe(const Colors &);

private:
    struct Rgb
    {
        int r = 0;
        int g = 0;
        int b = 0;
    } m_Color, m_Temp;
    int m_Action = 0;
    Colors m_PrevColor = Colors::Black;
};

extern LedBar ledbar;