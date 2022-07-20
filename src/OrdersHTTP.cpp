#include "Orders.h"
#include "Network.h"
#include "Display.h"
#include "Wifi.h"
#include "ArduinoJson.h"

bool Orders::retrieveOrders()
{
    if (wifi.isConnected())
    {
        std::string url = m_OrderURL;
        url.append(network.DeviceID);

        int len;
        char *http_payload = performRetrieveHTTP(url.c_str(), len);
        if (http_payload != nullptr)
        {
            DynamicJsonDocument doc(len);
            DeserializationError error = deserializeJson(doc, http_payload, len);
            if (error != DeserializationError::Ok)
            {
                xQueueSend(Tasks::FailsafeQueue, error.c_str(), portMAX_DELAY);
                free(http_payload);
                return false;
            }

            JsonArray jsonItems = doc["data"]["liveOperationGroups"].as<JsonArray>();
            m_Orders.clear();
            m_Orders.reserve(jsonItems.size());

            for (const JsonObject &json : jsonItems)
            {
                const char *Title = json["productionOrderName"].as<const char *>();
                const char *OrderID = json["productionOrderId"].as<const char *>();
                const char *ID = json["id"].as<const char *>();
                const char *State = json["state"].as<const char *>();
                m_Orders.emplace_back(Title, OrderID, ID, State);
            }

            free(http_payload);
            return true;
        }
    }
    return false;
}

bool Orders::retrieveOperations(const size_t &order)
{
    if (wifi.isConnected() && order < m_Orders.size())
    {
        std::string url = m_OperationURL;
        url.append(m_Orders[order].id);

        int len;
        char *http_payload = performRetrieveHTTP(url.c_str(), len);
        if (http_payload != nullptr)
        {
            DynamicJsonDocument doc(len);
            DeserializationError error = deserializeJson(doc, http_payload, len);

            if (error != DeserializationError::Ok)
            {
                xQueueSend(Tasks::FailsafeQueue, error.c_str(), portMAX_DELAY);
                free(http_payload);
                return false;
            }

            JsonArray jsonItems = doc["data"]["liveOperations"].as<JsonArray>();
            m_Orders[order].operations.clear();
            m_Orders[order].operations.reserve(jsonItems.size());

            for (const JsonObject &json : jsonItems)
            {
                const char *Title = json["name"].as<const char *>();
                const char *ID = json["id"].as<const char *>();
                const char *State = json["state"].as<const char *>();
                m_Orders[order].operations.emplace_back(Title, ID, State);
            }

            free(http_payload);
            return true;
        }
    }
    return false;
}

bool Orders::startOperation(const size_t &order, const size_t &operation)
{
    if (order < m_Orders.size() && operation < m_Orders[order].operations.size())
    {
        StaticJsonDocument<256> doc;
        doc["Id"] = m_Orders[order].operations[operation].id;

        char json_payload[256];
        size_t json_len = serializeJson(doc, json_payload);
        json_payload[json_len] = 0;

        const char *url = "https://apidev.bluestarplanning.com/Planning/Commands/Planboard/StartLiveOperation";
        if (performOperationStateHTTP(url, json_payload, json_len))
        {
            m_Orders[order].operations[operation].state = States::Running;
            return true;
        }
    }
    return false;
}

bool Orders::stopOperation(const size_t &order, const size_t &operation)
{
    if (order < m_Orders.size() && operation < m_Orders[order].operations.size())
    {
        StaticJsonDocument<256> doc;
        doc["Id"] = m_Orders[order].operations[operation].id;

        char json_payload[256];
        size_t json_len = serializeJson(doc, json_payload);
        json_payload[json_len] = 0;

        const char *url = "https://apidev.bluestarplanning.com/Planning/Commands/Planboard/StopLiveOperation";
        if (performOperationStateHTTP(url, json_payload, json_len))
        {
            m_Orders[order].operations[operation].state = States::Stopped;
            return true;
        }
    }
    return false;
}

bool Orders::pauseOperation(const size_t &order, const size_t &operation)
{
    if (order < m_Orders.size() && operation < m_Orders[order].operations.size())
    {
        StaticJsonDocument<256> doc;
        doc["Id"] = m_Orders[order].operations[operation].id;

        char json_payload[256];
        size_t json_len = serializeJson(doc, json_payload);
        json_payload[json_len] = 0;

        const char *url = "https://apidev.bluestarplanning.com/Planning/Commands/Planboard/PauseLiveOperation";
        if (performOperationStateHTTP(url, json_payload, json_len))
        {
            m_Orders[order].operations[operation].state = States::Paused;
            return true;
        }
    }
    return false;
}

char *Orders::performRetrieveHTTP(const char *url, int &len)
{
    if (wifi.isConnected())
    {
        esp_http_client_config_t http_config = {
            .url = url,
            .cert_pem = WiFiCredentials::ServerCA,
            .method = HTTP_METHOD_GET,
        };
        esp_http_client_handle_t http_client = esp_http_client_init(&http_config);
        esp_http_client_set_header(http_client, "x-bluestar-apk", network.AuthKey.c_str());

        if (esp_http_client_open(http_client, 0) != ESP_OK)
        {
            xQueueSend(Tasks::FailsafeQueue, "HTTP Connection Failed", portMAX_DELAY);
            return nullptr;
        }

        if ((len = esp_http_client_fetch_headers(http_client)) == 0)
            esp_http_client_get_chunk_length(http_client, &len);

        char *http_payload = (char *)malloc(len + 1);
        if (http_payload == nullptr)
        {
            xQueueSend(Tasks::FailsafeQueue, "No Memory Available", portMAX_DELAY);
            esp_http_client_close(http_client);
            esp_http_client_cleanup(http_client);
            return nullptr;
        }
        if ((len = esp_http_client_read(http_client, http_payload, len)) <= 0)
        {
            xQueueSend(Tasks::FailsafeQueue, "HTTP Reading Failed", portMAX_DELAY);

            esp_http_client_close(http_client);
            esp_http_client_cleanup(http_client);
            free(http_payload);
            return nullptr;
        }
        http_payload[len] = 0;

        int http_status = esp_http_client_get_status_code(http_client);
        esp_http_client_close(http_client);
        esp_http_client_cleanup(http_client);

        if (http_status == HttpStatus_Ok)
            return http_payload;
        network.errorToFailsafe(http_payload, http_status);
        free(http_payload);
    }
    return nullptr;
}

bool Orders::performOperationStateHTTP(const char *url, const char *http_payload, const int &len)
{
    esp_http_client_config_t http_config = {
        .url = url,
        .cert_pem = WiFiCredentials::ServerCA,
        .method = HTTP_METHOD_POST,
    };
    esp_http_client_handle_t http_client = esp_http_client_init(&http_config);
    esp_http_client_set_header(http_client, "Content-Type", "application/json");
    esp_http_client_set_header(http_client, "x-bluestar-apk", network.AuthKey.c_str());

    if (esp_http_client_open(http_client, len) != ESP_OK)
        xQueueSend(Tasks::FailsafeQueue, "HTTP Connection Failed", portMAX_DELAY);
    else
    {
        if (esp_http_client_write(http_client, http_payload, len) < 0)
            xQueueSend(Tasks::FailsafeQueue, "HTTP Writing Failed", portMAX_DELAY);
        if (esp_http_client_fetch_headers(http_client) < 0)
            xQueueSend(Tasks::FailsafeQueue, "HTTP Fetching Headers Failed", portMAX_DELAY);
        else
        {
            int http_status = esp_http_client_get_status_code(http_client);
            if (http_status == HttpStatus_Ok)
            {
                esp_http_client_close(http_client);
                esp_http_client_cleanup(http_client);
                return true;
            }
            network.responseToFailsafe(http_client, http_status);
        }
        esp_http_client_close(http_client);
        esp_http_client_cleanup(http_client);
    }
    return false;
}