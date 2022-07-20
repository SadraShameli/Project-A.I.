#pragma once
#include <cstring>
#include <string>
#include <string_view>
#include <optional>
#include <vector>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

namespace Tasks
{
    extern TaskHandle_t Failsafe;
    extern TaskHandle_t Gui;
    extern TaskHandle_t Input;
    extern TaskHandle_t Output;
    extern TaskHandle_t LedBar;
    extern TaskHandle_t Network;
    extern QueueHandle_t FailsafeQueue;

    constexpr TickType_t Network_Interval = 5000;
    constexpr TickType_t Failsafe_Interval = 2000;
    constexpr TickType_t Input_Interval = 50;
    constexpr TickType_t Output_Interval = 50;
    constexpr TickType_t LedBar_Interval = 50;
    constexpr TickType_t Gui_Interval = 50;

    enum Notifications
    {
        ConfigSet = 0x01,
    };
    extern uint32_t Notification;
}

namespace Menus
{
    enum Menus
    {
        Orders,
        Operations,
        Actions,
        Config,
        ConfigClients,
        Failsafe
    };
    constexpr int Count = 5;
}

namespace WiFiCredentials
{
    const char SSID[] = "BSP - HMI Unit";
    const char PASS[] = "";
    constexpr int ConnectionLimit = 10;
    extern const char ServerCA[] asm("_binary_src_certification_serverca_cer_start");
}

class Timer
{
private:
    int64_t start;

public:
    Timer() : start(esp_timer_get_time()) {}

    ~Timer()
    {
        int64_t end = esp_timer_get_time();
        printf("%lld\n", (end - start) / 1000);
    }
};