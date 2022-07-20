#pragma once
#include "Definitions.h"

class Wifi
{
public:
    bool isConnected();
    void startStation();
    void startAP();

    std::string getIPAP();
    std::optional<std::string> getIPClients(const int &);
    std::optional<std::string> getMacClients(const int &);

public:
    std::string SSID, Passwd, IPAddress = "0.0.0.0";
};

extern Wifi wifi;