#include "Network.h"
#include "Configuration.h"
#include "wifi.h"
#include "ArduinoJson.h"

Network network;

void Network::configureDevice(const char *json_payload)
{
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, json_payload, strlen(json_payload));

    if (error != DeserializationError::Ok)
    {
        xQueueSend(Tasks::FailsafeQueue, error.c_str(), portMAX_DELAY);
        return;
    }

    wifi.SSID = doc["inputSSID"].as<const char *>();
    wifi.Passwd = doc["inputPassword"].as<const char *>();
    AuthKey = doc["inputToken"].as<const char *>();
    m_Address = doc["inputAddress"].as<const char *>();

    wifi.SSID.erase(wifi.SSID.find_last_not_of(" \n\r\t") + 1);
    wifi.Passwd.erase(wifi.Passwd.find_last_not_of(" \n\r\t") + 1);
    AuthKey.erase(AuthKey.find_last_not_of(" \n\r\t") + 1);
    m_Address.erase(m_Address.find_last_not_of(" \n\r\t") + 1);

    xTaskNotify(Tasks::Network, 0x01, eSetValueWithOverwrite);
}

bool Network::retrieveConfig()
{
    if (wifi.isConnected())
    {
        std::string url = m_Address;
        url.append("?setupCode=");
        url.append(AuthKey);
        url.append("&organizationName=Starlabel%20B.V.");

        esp_http_client_config_t http_config =
            {
                .url = url.c_str(),
                .cert_pem = WiFiCredentials::ServerCA,
                .method = HTTP_METHOD_GET,
            };
        esp_http_client_handle_t http_client = esp_http_client_init(&http_config);

        esp_err_t err = esp_http_client_open(http_client, 0);
        if ((err) != ESP_OK)
        {
            xQueueSend(Tasks::FailsafeQueue, "HTTP Connection Failed", portMAX_DELAY);
            esp_http_client_cleanup(http_client);
            return false;
        }

        int len;
        esp_http_client_fetch_headers(http_client);
        esp_http_client_get_chunk_length(http_client, &len);

        char http_payload[1024] = {};
        if ((len = esp_http_client_read(http_client, http_payload, len)) <= 0)
        {
            xQueueSend(Tasks::FailsafeQueue, "HTTP Reading Failed", portMAX_DELAY);
            esp_http_client_close(http_client);
            esp_http_client_cleanup(http_client);
            return false;
        }
        http_payload[len] = 0;

        int http_status = esp_http_client_get_status_code(http_client);
        esp_http_client_close(http_client);
        esp_http_client_cleanup(http_client);

        if (http_status != HttpStatus_Ok)
        {
            StaticJsonDocument<1024> doc;
            DeserializationError error = deserializeJson(doc, PreConfig);

            if (error != DeserializationError::Ok)
            {
                xQueueSend(Tasks::FailsafeQueue, error.c_str(), portMAX_DELAY);
                return false;
            }

            JsonObject data = doc["data"];
            config.setSSID(wifi.SSID);
            config.setPasswd(wifi.Passwd);
            config.setDeviceName(data["name"].as<const char *>());
            config.setAuthKey(data["authenticationKey"].as<const char *>());
            config.setDeviceID(data["workstationId"].as<const char *>());
            config.setID(data["id"].as<const char *>());
            config.setConfigMode(false);
            return true;
        }
        errorToFailsafe(http_payload, http_status);
    }
    return false;
}

void Network::responseToFailsafe(esp_http_client_handle_t http_client, const int &http_status)
{
    char http_payload[512] = {};
    int len = esp_http_client_read_response(http_client, http_payload, sizeof(http_payload));
    http_payload[len] = 0;

    errorToFailsafe(http_payload, http_status);
}

void Network::errorToFailsafe(const char *http_payload, const int &http_status)
{
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, http_payload, strlen(http_payload));

    if (error != DeserializationError::Ok)
    {
        xQueueSend(Tasks::FailsafeQueue, error.c_str(), portMAX_DELAY);
        return;
    }

    char errorMessage[256] = {};
    sprintf(errorMessage, "%s %d - %s", "HTTP Code:", http_status, doc["error"]["message"].as<const char *>());
    xQueueSendToBack(Tasks::FailsafeQueue, errorMessage, portMAX_DELAY);
}