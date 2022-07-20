#include "Main.h"

static void network_client_task(void *arg)
{
  constexpr int refreshInterval = 30000;

  config.getSSID(wifi.SSID);
  config.getPasswd(wifi.Passwd);
  config.getAuthKey(network.AuthKey);
  config.getDeviceID(network.DeviceID);
  config.getDeviceName(network.DeviceName);
  wifi.startStation();

  for (;;)
  {
    if (display.getMenu() == Menus::Orders && orders.retrieveOrders())
    {
      ledbar.setColorWipe(Colors::White);
      vTaskDelay(refreshInterval);
      continue;
    }
    vTaskDelay(Tasks::Network_Interval);
  }
}

static void network_host_task(void *arg)
{
  wifi.startAP();

  for (;;)
  {
    ledbar.setColorWipe(Colors::Orange);
    display.setMenu(Menus::Config);

    xTaskNotifyWait(0, 0, &Tasks::Notification, portMAX_DELAY);
    if (Tasks::Notification & Tasks::ConfigSet)
    {
      ledbar.setColorWipe(Colors::Blue);
      display.setMenu(Menus::Config);
      wifi.startStation();

      for (;;)
      {
        if (network.retrieveConfig())
        {
          config.commit();
          esp_restart();
        }
        vTaskDelay(250);
      }
    }
    vTaskDelay(250);
  }
}

static void gui_task(void *arg)
{
  display.startup();

  for (;;)
  {
    if (display.getMenu() == Menus::Orders && orders.getOrderQuantity())
    {
      std::optional<int> runningOrder = orders.getFirstOrder(States::Running);
      if (runningOrder.has_value())
      {
        orders.selectedOrder = *runningOrder;
        orders.retrieveOperations(*runningOrder);
        display.setMenu(Menus::Operations);
      }
      else
      {
        display.printOrders(0, orders.getOrderID(display.cursorLine), orders.getOrderTitle(display.cursorLine));
        display.printOrders(1, orders.getOrderID(display.cursorLine + 1), orders.getOrderTitle(display.cursorLine + 1));
        display.printOrders(2, orders.getOrderID(display.cursorLine + 2), orders.getOrderTitle(display.cursorLine + 2));
        display.printOrders(3, orders.getOrderID(display.cursorLine + 3), orders.getOrderTitle(display.cursorLine + 3));
        display.printCursor((orders.selectedOrder % 4));

        ledbar.setColorWipe(Colors::White);
        output.toggle(Output::LedB, true);
        output.toggle(Output::LedY, false);
        output.toggle(Output::LedG, false);
      }
    }

    else if (display.getMenu() == Menus::Operations)
    {
      std::optional<int> runningOperation = orders.getFirstOperation(States::Running, orders.selectedOrder);
      std::optional<int> pausedOperation = orders.getFirstOperation(States::Paused, orders.selectedOrder);

      if (runningOperation.has_value())
      {
        orders.setSelectedOperation(orders.selectedOrder, *runningOperation);
        display.setMenu(Menus::Actions);
      }
      else
      {
        if (pausedOperation.has_value())
        {
          orders.setSelectedOperation(orders.selectedOrder, *pausedOperation);
          display.setMenu(Menus::Actions);
        }
        else
        {
          std::optional<int> selectedOperation = orders.getSelectedOperation(orders.selectedOrder);
          if (selectedOperation.has_value())
          {
            display.printOperations(0, orders.getOperationTitle(orders.selectedOrder, display.cursorLine));
            display.printOperations(1, orders.getOperationTitle(orders.selectedOrder, display.cursorLine + 1));
            display.printOperations(2, orders.getOperationTitle(orders.selectedOrder, display.cursorLine + 2));
            display.printOperations(3, orders.getOperationTitle(orders.selectedOrder, display.cursorLine + 3));
            display.printCursor(*selectedOperation % 4);

            ledbar.setColorWipe(Colors::White);
            output.toggle(Output::LedB, true);
            output.toggle(Output::LedY, true);
            output.toggle(Output::LedG, false);
          }
        }
      }
    }

    else if (display.getMenu() == Menus::Actions)
    {
      display.printOrders(0, orders.getOrderID(orders.selectedOrder), orders.getOrderTitle(orders.selectedOrder));
      std::optional<int> selectedOperation = orders.getSelectedOperation(orders.selectedOrder);
      if (selectedOperation.has_value())
      {
        display.printOperations(1, orders.getOperationTitle(orders.selectedOrder, *selectedOperation));
        std::optional<States> operationState = orders.getOperationState(orders.selectedOrder, *selectedOperation);
        if (operationState.has_value())
        {
          switch (*operationState)
          {
          case States::Running:
            ledbar.setColorWipe(Colors::Blue);
            output.toggle(Output::LedB, true);
            output.toggle(Output::LedY, true);
            output.toggle(Output::LedG, true);
            display.printActions("ACTIVE");
            break;

          case States::Stopped:
            ledbar.setColorWipe(Colors::Green);
            output.toggle(Output::LedB, true);
            output.toggle(Output::LedY, false);
            output.toggle(Output::LedG, true);
            display.printActions("STOPPED");
            break;

          case States::Paused:
            ledbar.setColorWipe(Colors::Orange);
            output.toggle(Output::LedB, true);
            output.toggle(Output::LedY, false);
            output.toggle(Output::LedG, true);
            display.printActions("PAUSED");
            break;

          case States::Default:
            ledbar.setColorWipe(Colors::White);
            output.toggle(Output::LedB, true);
            output.toggle(Output::LedY, true);
            output.toggle(Output::LedG, true);
            display.printActions("Default");
            break;

          default:
            display.printActions("STATE UNKNOWN");
            break;
          }
          display.printCursor(3);
        }
      }
    }

    else if (display.getMenu() == Menus::Orders && orders.getOrderQuantity() == 0)
    {
      display.print(0, 1, "Downloading Orders");
      display.print(0, 2, "from the Backend");
      display.printLogo(0);
    }

    if (config.getConfigMode())
    {
      if (display.getMenu() == Menus::Config && config.getConfigMode() && !(Tasks::Notification & Tasks::ConfigSet))
        display.printConfig("SSID:", WiFiCredentials::SSID, "Host IP:", wifi.getIPAP());
      else if (display.getMenu() == Menus::Config && config.getConfigMode() && (Tasks::Notification & Tasks::ConfigSet) && !wifi.isConnected())
        display.printConfig("Connecting to:", wifi.SSID, "", "");
      else if (display.getMenu() == Menus::Config && config.getConfigMode() && (Tasks::Notification & Tasks::ConfigSet) && wifi.isConnected())
        display.printConfig("Connected to ", wifi.SSID, "Retrieving data from", "back end.");
      else if ((display.getMenu() == Menus::ConfigClients))
        display.printClients(wifi.getIPClients(0), wifi.getMacClients(0), wifi.getIPClients(1), wifi.getMacClients(1));
    }

    vTaskDelay(Tasks::Gui_Interval);
  }
}

static void input_task(void *arg)
{
  for (;;)
  {
    input.update();

    if (input.getPinState(Input::Refresh))
    {
      if (!config.getConfigMode())
      {
        if (clock() < 10000)
        {
          display.setMenu(Menus::Config);
          display.print(0, 1, "Press Yellow button");
          display.print(0, 2, "to reset the unit, ");
          display.print(0, 3, "Green to continue  ");

          for (;;)
          {
            input.update();
            if (input.getPinState(Input::Green))
            {
              display.setMenu(Menus::Orders);
              break;
            }

            else if (input.getPinState(Input::Yellow))
            {
              config.reset();
              esp_restart();
            }
            vTaskDelay(Tasks::Input_Interval);
          }
        }
        else
        {
          switch (display.getMenu())
          {
          case Menus::Orders:
            orders.retrieveOrders();
            break;
          case Menus::Operations:
            orders.retrieveOperations(orders.selectedOrder);
            break;
          case Menus::Actions:
            orders.retrieveOperations(orders.selectedOrder);
            break;
          }
        }
      }
      else
        esp_restart();
    }

    else if (input.getPinState(Input::Up))
    {
      if (display.getMenu() == Menus::Orders)
      {
        if (orders.selectedOrder > 0)
        {
          --orders.selectedOrder;
          if (orders.selectedOrder % 4 == 3)
            display.cursorLine += orders.selectedOrder;
        }
      }

      else if (display.getMenu() == Menus::Operations)
      {
        std::optional<int> selectedOperation = orders.getSelectedOperation(orders.selectedOrder);
        if (selectedOperation.has_value())
        {
          if (*selectedOperation > 0)
          {
            orders.decrementOperation(orders.selectedOrder);
            if (*selectedOperation - 1 % 4 == 3)
              display.cursorLine += *selectedOperation - 1;
          }
        }
      }
    }

    else if (input.getPinState(Input::Down))
    {
      if (display.getMenu() == Menus::Orders)
      {
        if (orders.selectedOrder + 1 < orders.getOrderQuantity())
        {
          ++orders.selectedOrder;
          if (orders.selectedOrder % 4 == 0)
            display.cursorLine += orders.selectedOrder;
        }
      }

      else if (display.getMenu() == Menus::Operations)
      {
        std::optional<int> selectedOperation = orders.getSelectedOperation(orders.selectedOrder);
        if (selectedOperation.has_value())
        {
          if (*selectedOperation + 1 < orders.getOperationQuantity(orders.selectedOrder))
          {
            orders.incrementOperation(orders.selectedOrder);
            if (*selectedOperation + 1 % 4 == 0)
              display.cursorLine += *selectedOperation + 1;
          }
        }
        else
          display.cursorLine = 0;
      }
    }

    else if (input.getPinState(Input::Blue))
    {
      switch (display.getMenu())
      {
      case Menus::Orders:
        if (orders.retrieveOperations(orders.selectedOrder))
          display.setMenu(Menus::Operations);
        break;

      case Menus::Operations:
      {
        std::optional<int> selectedOperation = orders.getSelectedOperation(orders.selectedOrder);
        if (selectedOperation.has_value())
        {
          std::optional<States> operationState = orders.getOperationState(orders.selectedOrder, *selectedOperation);
          if (operationState.has_value())
          {
            if (*operationState != States::Running)
            {
              if (orders.startOperation(orders.selectedOrder, *selectedOperation))
                display.setMenu(Menus::Actions);
            }
          }
        }
        break;
      }
      case Menus::Actions:
      {
        std::optional<int> selectedOperation = orders.getSelectedOperation(orders.selectedOrder);
        if (selectedOperation.has_value())
        {
          std::optional<States> operationState = orders.getOperationState(orders.selectedOrder, *selectedOperation);
          if (operationState.has_value())
          {
            if (*operationState != States::Running)
            {
              orders.startOperation(orders.selectedOrder, *selectedOperation);
            }
          }
        }
        break;
      }
      case Menus::Config:
        if (!(Tasks::Notification & Tasks::ConfigSet))
          display.setMenu(Menus::ConfigClients);
        break;
      case Menus::Failsafe:
        if (!config.getConfigMode())
          display.setMenu(Menus::Orders);
        break;
      }
    }
    else if (input.getPinState(Input::Yellow))
    {
      switch (display.getMenu())
      {
      case Menus::Operations:
        display.setMenu(Menus::Orders);
        break;
      case Menus::Actions:
      {
        std::optional<int> selectedOperation = orders.getSelectedOperation(orders.selectedOrder);
        if (selectedOperation.has_value())
        {
          std::optional<States> operationState = orders.getOperationState(orders.selectedOrder, *selectedOperation);
          if (operationState.has_value())
          {
            if (*operationState == States::Running)
              orders.pauseOperation(orders.selectedOrder, *selectedOperation);
          }
        }
        break;
      }
      case Menus::ConfigClients:
        display.setMenu(Menus::Config);
        break;
      }
    }

    else if (input.getPinState(Input::Green))
    {
      if (display.getMenu() == Menus::Actions)
      {
        std::optional<int> selectedOperation = orders.getSelectedOperation(orders.selectedOrder);
        if (selectedOperation.has_value())
        {
          std::optional<States> operationState = orders.getOperationState(orders.selectedOrder, *selectedOperation);
          if (operationState.has_value())
          {
            if (operationState == States::Running || operationState == States::Paused || operationState == States::Default)
            {
              if (orders.stopOperation(orders.selectedOrder, *selectedOperation))
              {
                vTaskDelay(1000);
                if (*selectedOperation + 1 < orders.getOperationQuantity(orders.selectedOrder))
                {
                  orders.setSelectedOperation(orders.selectedOrder, *selectedOperation + 1);
                  display.setMenu(Menus::Operations);
                }
                else
                {
                  orders.retrieveOrders();
                  if (orders.selectedOrder + 1 < orders.getOrderQuantity())
                    ++orders.selectedOrder;
                  display.setMenu(Menus::Orders);
                }
              }
            }
          }
        }
      }
    }
    vTaskDelay(Tasks::Input_Interval);
  }
}

static void output_task(void *arg)
{
  for (;;)
  {
    output.update();
    vTaskDelay(Tasks::Output_Interval);
  }
}

static void ledBar_task(void *arg)
{
  ledbar.startup();
  for (;;)
  {
    ledbar.update();
    vTaskDelay(Tasks::LedBar_Interval);
  }
}

static void failsafe_task(void *arg)
{
  char queueBuffer[128] = {};
  Tasks::FailsafeQueue = xQueueCreate(8, sizeof(queueBuffer));

  while (xQueueReceive(Tasks::FailsafeQueue, queueBuffer, portMAX_DELAY))
  {
    vTaskSuspend(Tasks::Gui);
    printf("Failsafe: %s\n", queueBuffer);

    output.blink(Output::LedB, Tasks::Failsafe_Interval);
    output.blink(Output::LedY, Tasks::Failsafe_Interval);
    output.blink(Output::LedG, Tasks::Failsafe_Interval);
    ledbar.setColorWipe(Colors::Red);
    display.printString(queueBuffer);

    vTaskDelay(Tasks::Failsafe_Interval);
    vTaskResume(Tasks::Gui);
  }
}

void setup()
{
  xTaskCreate(&failsafe_task, "failsafe", 4096, nullptr, 1, &Tasks::Failsafe);
  xTaskCreate(&input_task, "input", 8192, nullptr, configMAX_PRIORITIES - 1, &Tasks::Input);
  xTaskCreate(&output_task, "output", 2048, nullptr, 1, &Tasks::Output);
  xTaskCreate(&ledBar_task, "ledBar", 2048, nullptr, 4, &Tasks::LedBar);

  if (!config.getConfigMode())
    xTaskCreate(&network_client_task, "network", 8192, nullptr, 18, &Tasks::Network);
  else
    xTaskCreate(&network_host_task, "network", 8192, nullptr, 18, &Tasks::Network);
  xTaskCreate(&gui_task, "gui", 8192, nullptr, configMAX_PRIORITIES - 1, &Tasks::Gui);
  vTaskDelete(nullptr);
}

void loop() {}