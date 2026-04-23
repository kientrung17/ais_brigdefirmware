#ifndef WiFiManagerAbstract_
#define WiFiManagerAbstract_
// FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
// ESP-IDF
#include "esp_event.h"
#include "esp_err.h"
#include "esp_wifi.h"
// STL
#include <string>

class WiFiManagerAbstract
{
public:
    enum class ModeWifi
    {
        MODE_STA,
        MODE_AP,
        MODE_AP_STA
    };

    struct STAConfig
    {
        std::string ssid;
        std::string password;
    };

    struct APConfig
    {
        std::string ssid;
        std::string password;
        uint8_t channel;
        uint8_t maxConnect;
        APConfig() : channel(1), maxConnect(4) {}
    };

    // Chỉ khởi tạo 1 hàm init duy nhất
    virtual bool initWifi(ModeWifi mode, const STAConfig &staCfg = STAConfig{}, const APConfig &apCfg = APConfig{}) = 0;
    virtual bool changeWifiMode(ModeWifi newMode, const STAConfig &staCfg = STAConfig{}, const APConfig &apCfg = APConfig{}) = 0;
    virtual bool reconnectWifi() = 0;
    virtual std::string getSsidWifiSTA() = 0;
    virtual std::string getPassWifiSTA() = 0;
    virtual bool isWifiConnected() const = 0;
    virtual ModeWifi getModeWifi() const { return mModeWifi; }

protected:
    ModeWifi mModeWifi{ModeWifi::MODE_STA};
    STAConfig mStaConfig;
    APConfig mApConfig;
};
#endif