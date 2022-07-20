#include "LedBar.h"
#include "Adafruit_NeoPixel.h"

static constexpr int ledPin = 15, ledCount = 20;
static Adafruit_NeoPixel ledStrip(ledCount, ledPin, NEO_RGB + NEO_KHZ800);

void LedBar::startup()
{
    ledStrip.begin();
    for (int i = 0; i < ledCount; ++i)
        ledStrip.setPixelColor(i, 0, 0, 0);
    ledStrip.show();
}

void LedBar::update()
{
    if (m_Action == 1)
    {
        while (m_Temp.r != m_Color.r || m_Temp.g != m_Color.g || m_Temp.b != m_Color.b)
        {
            if (m_Temp.r < m_Color.r)
                m_Temp.r += 1;
            if (m_Temp.r > m_Color.r)
                m_Temp.r -= 1;

            if (m_Temp.g < m_Color.g)
                m_Temp.g += 1;
            if (m_Temp.g > m_Color.g)
                m_Temp.g -= 1;

            if (m_Temp.b < m_Color.b)
                m_Temp.b += 1;
            if (m_Temp.b > m_Color.b)
                m_Temp.b -= 1;

            for (int i = 0; i < ledCount; ++i)
                ledStrip.setPixelColor(i, m_Temp.g, m_Temp.r, m_Temp.b);
            ledStrip.show();
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
        m_Action = 0;
    }
}

void LedBar::setColorWipe(const int &r, const int &g, const int &b)
{
    m_Color.r = r;
    m_Color.g = g;
    m_Color.b = b;
    m_Action = 1;
}

void LedBar::setColorWipe(const Colors &colors)
{
    if (m_PrevColor != colors)
    {
        switch (colors)
        {
        case Colors::White:
            m_Color.r = 255;
            m_Color.g = 255;
            m_Color.b = 255;
            break;

        case Colors::Blue:
            m_Color.r = 0;
            m_Color.g = 0;
            m_Color.b = 255;
            break;

        case Colors::Orange:
            m_Color.r = 255;
            m_Color.g = 50;
            m_Color.b = 0;
            break;

        case Colors::Green:
            m_Color.r = 0;
            m_Color.g = 255;
            m_Color.b = 0;
            break;

        case Colors::Red:
            m_Color.r = 255;
            m_Color.g = 0;
            m_Color.b = 0;
            break;

        default:
            m_Color.r = 0;
            m_Color.g = 0;
            m_Color.b = 0;
            break;
        }
        m_Action = 1;
        m_PrevColor = colors;
        update();
    }
}

LedBar ledbar;