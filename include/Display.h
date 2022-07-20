#pragma once
#include "Definitions.h"

class Display
{
public:
    void startup();

    void printOrders(const int &, const std::optional<std::string_view> &, const std::optional<std::string_view> &);
    void printOperations(const int &, const std::optional<std::string_view> &);
    void printActions(const char *);
    void printConfig(const std::string_view &, const std::string_view &, const std::string_view &, const std::string_view &);
    void printClients(const std::optional<std::string_view> &, const std::optional<std::string_view> &, const std::optional<std::string_view> &, const std::optional<std::string_view> &);

    void printLogo(const int &);
    void printCursor(const int &);

    void printString(const char *);
    void printLength(const int &, const int &, const char *, int);
    void print(const int &, const int &, const char *);
    void wipe(const int &, const int &, int);
    void clear();

    void saver();
    void resetSaver();

    void setMenu(const Menus::Menus &);
    const Menus::Menus &getMenu();

public:
    size_t cursorLine = 0;

private:
    Menus::Menus menuPage = Menus::Orders;
    clock_t time = 0, prevTime = 0;
    size_t prevCursorLine = 0;
};

extern Display display;