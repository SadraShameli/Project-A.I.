#pragma once
#include "Definitions.h"

class Configuration
{
public:
    Configuration();
    void commit();
    void reset();

    void getSSID(std::string &);
    void getPasswd(std::string &);
    void getDeviceName(std::string &);
    void getAuthKey(std::string &);
    void getDeviceID(std::string &);
    void getID(std::string &);
    bool getConfigMode();

    void setSSID(const std::string &);
    void setPasswd(const std::string &);
    void setDeviceName(const std::string &);
    void setAuthKey(const std::string &);
    void setDeviceID(const std::string &);
    void setID(const std::string &);
    void setConfigMode(bool);

private:
    void calculateMask();
    void encryptText(uint32_t *, const std::string &);
    void decryptText(uint32_t *, std::string &);

private:
    static constexpr int m_SSID_Length = 32, m_Password_Length = 64;
    static constexpr int m_Name_Length = 12, m_Data_Length = 100;
    uint32_t m_EncryptionMask = 0;

    struct configData
    {
        uint32_t ssid[m_SSID_Length] = {};
        uint32_t passwd[m_Password_Length] = {};
        uint32_t deviceName[m_Name_Length] = {};
        uint32_t authKey[m_Data_Length] = {};
        uint32_t deviceID[m_Data_Length] = {};
        uint32_t id[m_Data_Length] = {};
        bool configMode = true;
    } m_ConfigData;
};

extern Configuration config;