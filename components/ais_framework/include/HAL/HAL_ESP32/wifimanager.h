#ifndef WiFiManagerESP32_
#define WiFiManagerESP32_

// FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
// ESP-IDF
#include "esp_event.h"
#include "esp_err.h"
#include "esp_wifi.h"
// STL
#include <string>
#include "wifimanagerabstract.h"

class WiFiManagerESP32 : public WiFiManagerAbstract
{
public:
    WiFiManagerESP32();
    ~WiFiManagerESP32();

    // Chỉ khởi tạo 1 hàm init duy nhất
    virtual bool initWifi(ModeWifi mode, const STAConfig &staCfg = STAConfig{}, const APConfig &apCfg = APConfig{}) override;

    virtual bool reconnectWifi() override;
    virtual std::string getSsidWifiSTA() override;
    virtual std::string getPassWifiSTA() override;
    virtual bool changeWifiMode(ModeWifi newMode, const STAConfig &staCfg = STAConfig{}, const APConfig &apCfg = APConfig{}) override;
    bool isWifiConnected() const override;

private:
    static void eventHandlerWifi(void *arg,
                                 esp_event_base_t event_base,
                                 int32_t event_id,
                                 void *event_data);

private:
    EventGroupHandle_t mWifiEventGroup{};
    bool mWifiInited{false};
};
#endif
