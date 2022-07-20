#include "Wifi.h"
#include "Configuration.h"
#include "HTTPServer.h"
#include "esp_wifi.h"

static int failedAttempt = 0;
static bool connected = false;
static esp_netif_t *wifi_sta = nullptr, *wifi_ap = nullptr;

static void sta_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_START)
            esp_wifi_connect();
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            connected = false;
            if (failedAttempt < WiFiCredentials::ConnectionLimit)
                failedAttempt++;
            else
            {
                xQueueSend(Tasks::FailsafeQueue, "Can't connect to the WiFi", portMAX_DELAY);
                wifi.IPAddress = "0.0.0.0";
                failedAttempt = 0;
            }
            esp_wifi_connect();
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t event = *(ip_event_got_ip_t *)event_data;
        char buff[16];
        sprintf(buff, IPSTR, IP2STR(&event.ip_info.ip));
        wifi.IPAddress = buff;

        failedAttempt = 0;
        connected = true;
    }
}

bool Wifi::isConnected()
{
    return connected;
}

void Wifi::startStation()
{
    esp_netif_init();
    esp_event_loop_create_default();
    wifi_sta = esp_netif_create_default_wifi_sta();
    esp_netif_set_hostname(wifi_sta, WiFiCredentials::SSID);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &sta_event_handler, nullptr, nullptr);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &sta_event_handler, nullptr, nullptr);

    wifi_config_t wifi_config = {};
    wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifi_config.sta.threshold.rssi = -127;
    wifi_config.sta.pmf_cfg.capable = true;

    strncpy((char *)wifi_config.sta.ssid, wifi.SSID.c_str(), wifi.SSID.length());
    if (!wifi.Passwd.empty())
    {
        strncpy((char *)wifi_config.sta.password, wifi.Passwd.c_str(), wifi.Passwd.length());
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    printf("Connecting to SSID: %s\n", wifi.SSID.c_str());
}

void Wifi::startAP()
{
    esp_netif_init();
    esp_event_loop_create_default();
    wifi_ap = esp_netif_create_default_wifi_ap();
    esp_netif_set_hostname(wifi_ap, WiFiCredentials::SSID);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);

    wifi_config_t wifi_config = {};
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    wifi_config.ap.max_connection = 2;
    wifi_config.ap.ftm_responder = false;

    strncpy((char *)wifi_config.ap.ssid, WiFiCredentials::SSID, strlen(WiFiCredentials::SSID));
    if (strlen(WiFiCredentials::PASS) >= 8)
    {
        strncpy((char *)wifi_config.ap.password, WiFiCredentials::PASS, strlen(WiFiCredentials::PASS));
        wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
        wifi_config.ap.pairwise_cipher = WIFI_CIPHER_TYPE_CCMP;
    }

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();

    esp_netif_ip_info_t ip;
    esp_netif_get_ip_info(wifi_ap, &ip);

    char buff[16];
    sprintf(buff, IPSTR, IP2STR(&ip.ip));

    printf("Access Point Started\n");
    printf("SSID: %s\n", WiFiCredentials::SSID);
    printf("Password: %s\n", WiFiCredentials::PASS);
    printf("IP: %s\n", buff);

    mountStorage();
    startServer("/data");
}

std::string Wifi::getIPAP()
{
    esp_netif_ip_info_t ip;
    esp_netif_get_ip_info(wifi_ap, &ip);

    char buff[16];
    sprintf(buff, IPSTR, IP2STR(&ip.ip));
    return buff;
}

std::optional<std::string> Wifi::getIPClients(const int &i)
{
    wifi_sta_list_t stationList;
    tcpip_adapter_sta_list_t adapter_sta_list;

    esp_wifi_ap_get_sta_list(&stationList);
    tcpip_adapter_get_sta_list(&stationList, &adapter_sta_list);

    if (i < adapter_sta_list.num)
    {
        tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
        char buff[18] = {};
        sprintf(buff, IPSTR, IP2STR(&station.ip));
        return buff;
    }
    return {};
}

std::optional<std::string> Wifi::getMacClients(const int &i)
{
    wifi_sta_list_t stationList;
    esp_wifi_ap_get_sta_list(&stationList);

    if (i < stationList.num)
    {
        wifi_sta_info_t station = stationList.sta[i];
        char buff[18] = {};
        sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X", station.mac[0], station.mac[1], station.mac[2], station.mac[3], station.mac[4], station.mac[5]);
        return buff;
    }
    return {};
}

Wifi wifi;