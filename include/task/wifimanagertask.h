#pragma once
#include "taskabstract.h"
#include <string>
#include "wifimanagerabstract.h"
#include "webservermanagerabstract.h"
#include "sntpmanager.h"
#include "halgpioabstract.h"

//đồng bộ thời gin hệ thống với thời gian ở server sntp
//Giải thích ở README
class WifiManagerTask : public TaskAbstract
{
public:
    const std::string SERVER_SNTSP_NTP = "pool.ntp.org";
    const std::string SERVER_SNTSP_GOOGLE = "time.google.com";
    const std::string SERVER_SNTP_CLOULDFARE = "time.cloudflare.com";

    static constexpr uint16_t DIV_COUNTER_10_S = 1000; // 10s
    static constexpr uint16_t DIV_COUNTER_10Hz = 10;
    static constexpr uint16_t COUNTER_10_S = 1000;
    static constexpr uint32_t DIV_COUNTER_1_H = 360000;
    // wifi
    const std::string SSID_WIFI_AP = "Aqua_Control";
    const std::string PASS_WIFI_AP = "";
    WifiManagerTask(WiFiManagerAbstract *wifimanager, WebServerAbstract* webserver, std::string nameTask, int numElementQueueSet);
    ~WifiManagerTask();

    virtual void onTimer100HzProcess() override;
    virtual void onQueueSetMessageProcess(OSBase::QueueHandle queue_sem) override;
    virtual void onInitProcess() override;

    enum ServerType
    {
        Ntp = 0,
        Google,
        Clouldfare
    };

    enum StateSntp
    {
        Idle = 0,
        Init,
        WaitSync,
        ChangeServer,
        Synced
    };

private:
    void changeModeWifiToAPSta();
    //Target state machine: Sync and change server if not sysnc before init 10s
    void processInitSntpStateMachine(StateSntp stateSntpCurrent);

private:
    uint32_t mCounter100Hz{0};
    WiFiManagerAbstract *mWiFiManager{nullptr};
    WebServerAbstract *mWebServer;
    //init wifi
    WiFiManagerAbstract::STAConfig mStaConfig; 
    WiFiManagerAbstract::APConfig mApConfig;
    bool mWifiStaInited {false};

    bool mIsWifiStaConnected{false};
    SntpManager mSntpServer;

    //Sntp
    StateSntp mStateSntpCurrent {StateSntp::Idle};
    ServerType mServerType {ServerType::Ntp};
    uint16_t mCountCheckSntpSync {0};
};