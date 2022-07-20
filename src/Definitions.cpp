#include "Definitions.h"

namespace Tasks
{
    TaskHandle_t Failsafe = nullptr;
    TaskHandle_t Gui = nullptr;
    TaskHandle_t Input = nullptr;
    TaskHandle_t Output = nullptr;
    TaskHandle_t LedBar = nullptr;
    TaskHandle_t Network = nullptr;

    QueueHandle_t FailsafeQueue = nullptr;
    uint32_t Notification = 0;
}