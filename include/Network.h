#pragma once
#include "Definitions.h"
#include "esp_http_client.h"

class Network
{
public:
  void configureDevice(const char *);
  bool retrieveConfig();

  void responseToFailsafe(esp_http_client_handle_t, const int &);
  void errorToFailsafe(const char *, const int &);

public:
  std::string DeviceID, AuthKey, DeviceName;

private:
  std::string m_Address;
};

extern Network network;

const char PreConfig[] = R"({
  "data": {
    "name": "MPS M2",
    "authenticationKey": "056e090f-0887-4496-fc68-08d9faa5ee8d",
    "id": "b857460b-0375-4143-c247-08da29bde386",
    "workstationId": "f4f72095-7ccd-4c51-037a-08d9ae9323b5"
  },
  "error": null,
  "statusCode": "Succeed"
})";