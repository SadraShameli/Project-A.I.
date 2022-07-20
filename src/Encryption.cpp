#include "Configuration.h"
#include "esp_mac.h"

void Configuration::calculateMask()
{
    uint32_t maskArray[10] = {};
    uint32_t macArray[10] = {};
    uint32_t mask = 1564230594;
    uint32_t mac = 0;
    esp_efuse_mac_get_default((uint8_t *)(&mac));

    for (int i = 10 - 1; i >= 0; i--)
    {
        macArray[i] = mac % 10;
        mac /= 10;
    }
    for (int i = 10 - 1; i >= 0; i--)
    {
        maskArray[i] = mask % 10;
        mask /= 10;
    }
    for (int i = 0; i <= 10 - 1; i += 2)
    {
        m_EncryptionMask *= 10;
        m_EncryptionMask += macArray[i];
    }
    for (int i = 9; i >= 1; i -= 2)
    {
        m_EncryptionMask *= 10;
        m_EncryptionMask += maskArray[i];
    }
}

void Configuration::encryptText(uint32_t *var, const std::string &str)
{
    size_t len = str.length();
    for (int i = 0; i < len; ++i)
        var[i] = str[i] ^ m_EncryptionMask;
    var[len] = 0;
}

void Configuration::decryptText(uint32_t *var, std::string &str)
{
    while (*var)
        str.push_back(*(var++) ^ m_EncryptionMask);
}