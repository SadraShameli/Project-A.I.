#include "Display.h"
#include "LedBar.h"
#include "Network.h"
#include "SPI.h"
#include "Wire.h"
#include "LiquidCrystal_I2C.h"

static constexpr int screen_saver = 15;
static constexpr int logo_time = 500;
static uint8_t cursorChar[8] = {0x00, 0x01, 0x05, 0x0D, 0x1F, 0x0C, 0x04, 0x00};
static LiquidCrystal_I2C lcd(0x27, 20, 4);

void Display::startup()
{
    lcd.init();
    lcd.createChar(0, cursorChar);
    lcd.backlight();
}

void Display::printOrders(const int &line, const std::optional<std::string_view> &str0, const std::optional<std::string_view> &str1)
{
    if (str0.has_value() && str1.has_value())
    {
        printLength(0, line, str0->data(), 8);
        lcd.print(' ');
        printLength(9, line, str1->data(), 10);
    }
    else
        wipe(0, line, 19);
}

void Display::printOperations(const int &line, const std::optional<std::string_view> &str0)
{
    if (str0.has_value())
        printLength(0, line, str0->data(), 19);
    else
        wipe(0, line, 19);
}

void Display::printActions(const char *str0)
{
    wipe(0, 2, 20);
    wipe(0, 3, 7);
    printLength(7, 3, str0, 12);
}

void Display::printConfig(const std::string_view &str0, const std::string_view &str1, const std::string_view &str2, const std::string_view &str3)
{
    printLength(0, 0, str0.data(), 20);
    printLength(0, 1, str1.data(), 20);
    printLength(0, 2, str2.data(), 20);
    printLength(0, 3, str3.data(), 20);
}

void Display::printClients(const std::optional<std::string_view> &str0, const std::optional<std::string_view> &str1, const std::optional<std::string_view> &str2, const std::optional<std::string_view> &str3)
{
    if (str0.has_value())
    {
        print(0, 0, "1. ");
        printLength(3, 0, str0->data(), 15);
    }
    else
        wipe(0, 0, 20);
    if (str1.has_value())
    {
        print(0, 1, "   ");
        printLength(3, 1, str1->data(), 17);
    }
    else
        print(0, 1, "No Station Connected");
    if (str2.has_value())
    {
        print(0, 2, "2. ");
        printLength(3, 2, str2->data(), 15);
    }
    else
        wipe(0, 2, 20);
    if (str3.has_value())
    {
        print(0, 3, "   ");
        printLength(3, 3, str3->data(), 17);
    }
    else
        print(0, 3, "No Station Connected");
}

void Display::printString(const char *str)
{
    size_t len = strlen(str);
    if (len <= 20)
        printLength(0, 0, str, 20);
    else if (len <= 40)
    {
        printLength(0, 0, str, 20);
        printLength(0, 1, str + 20, 20);
    }
    else if (len <= 60)
    {
        printLength(0, 0, str, 20);
        printLength(0, 1, str + 20, 20);
        printLength(0, 2, str + 40, 20);
    }
    else
    {
        printLength(0, 0, str, 20);
        printLength(0, 1, str + 20, 20);
        printLength(0, 2, str + 40, 20);
        printLength(0, 3, str + 60, 20);
    }
}

void Display::print(const int &x, const int &y, const char *str)
{
    lcd.setCursor(x, y);
    while (*str)
        lcd.print(*(str++));
}

void Display::printLength(const int &x, const int &y, const char *str, int n)
{
    lcd.setCursor(x, y);
    while (*str && n--)
        lcd.print(*(str++));
    for (n; n > 0; --n)
        lcd.print(' ');
}

void Display::wipe(const int &x, const int &y, int len)
{
    lcd.setCursor(x, y);
    while (len--)
        lcd.print(' ');
}

void Display::clear()
{
    lcd.clear();
}

void Display::printLogo(const int &line)
{
    lcd.setCursor(0, 0);
    for (int i = 0; i < 4; ++i)
        lcd.print(char(255));
    lcd.setCursor(16, 0);
    for (int i = 0; i < 4; ++i)
        lcd.print(char(255));
    lcd.setCursor((20 - network.DeviceName.length()) / 2, line);
    lcd.print(network.DeviceName.c_str());
}

void Display::printCursor(const int &line)
{
    lcd.setCursor(19, prevCursorLine);
    lcd.print(' ');
    lcd.setCursor(19, line);
    lcd.write(0);
    prevCursorLine = line;
}

void Display::saver()
{
    time = clock();
    if ((time - prevTime) > screen_saver * 60000)
    {
        prevTime = time;
        lcd.clear();
        lcd.noBacklight();
        ledbar.setColorFade(0, 0, 0);
        vTaskSuspend(Tasks::Gui);
        vTaskSuspend(Tasks::LedBar);
    }
}

void Display::resetSaver()
{
    prevTime = clock();
    lcd.backlight();
    vTaskResume(Tasks::Gui);
    vTaskResume(Tasks::LedBar);
}

const Menus::Menus &Display::getMenu()
{
    return menuPage;
}

void Display::setMenu(const Menus::Menus &menu)
{
    menuPage = menu;
}

Display display;