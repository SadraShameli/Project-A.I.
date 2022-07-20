#include "Configuration.h"
#include "nvs_flash.h"
#include "nvs.h"

static nvs_handle_t s_NvsHandle = 0;

Configuration::Configuration()
{
    nvs_flash_init();
    nvs_open("configData", NVS_READWRITE, &s_NvsHandle);

    size_t required_size = 0;
    nvs_get_blob(s_NvsHandle, "configData", nullptr, &required_size);
    nvs_get_blob(s_NvsHandle, "configData", (void *)&m_ConfigData, &required_size);

    calculateMask();
}

void Configuration::commit()
{
    nvs_set_blob(s_NvsHandle, "configData", (const void *)&m_ConfigData, sizeof(m_ConfigData));
    nvs_commit(s_NvsHandle);
}

void Configuration::reset()
{
    m_ConfigData = {};
    m_ConfigData.configMode = true;
    commit();
}

void Configuration::getSSID(std::string &str)
{
    str.reserve(m_SSID_Length);
    decryptText(m_ConfigData.ssid, str);
}
void Configuration::getDeviceName(std::string &str)
{
    str.reserve(m_Name_Length);
    decryptText(m_ConfigData.deviceName, str);
}
void Configuration::getPasswd(std::string &str)
{
    str.reserve(m_Password_Length);
    decryptText(m_ConfigData.passwd, str);
}
void Configuration::getAuthKey(std::string &str)
{
    str.reserve(m_Data_Length);
    decryptText(m_ConfigData.authKey, str);
}
void Configuration::getDeviceID(std::string &str)
{
    str.reserve(m_Data_Length);
    decryptText(m_ConfigData.deviceID, str);
}
void Configuration::getID(std::string &str)
{
    str.reserve(m_Data_Length);
    decryptText(m_ConfigData.id, str);
}
bool Configuration::getConfigMode()
{
    return m_ConfigData.configMode;
}

void Configuration::setSSID(const std::string &str)
{
    if (str.length() <= m_SSID_Length)
        encryptText(m_ConfigData.ssid, str);
}
void Configuration::setPasswd(const std::string &str)
{
    if (str.length() <= m_Password_Length)
        encryptText(m_ConfigData.passwd, str);
}
void Configuration::setDeviceName(const std::string &str)
{
    if (str.length() <= m_Name_Length)
        encryptText(m_ConfigData.deviceName, str);
}
void Configuration::setAuthKey(const std::string &str)
{
    if (str.length() <= m_Data_Length)
        encryptText(m_ConfigData.authKey, str);
}
void Configuration::setDeviceID(const std::string &str)
{
    if (str.length() <= m_Data_Length)
        encryptText(m_ConfigData.deviceID, str);
}
void Configuration::setID(const std::string &str)
{
    if (str.length() <= m_Data_Length)
        encryptText(m_ConfigData.id, str);
}
void Configuration::setConfigMode(bool var)
{
    m_ConfigData.configMode = var;
}

Configuration config;