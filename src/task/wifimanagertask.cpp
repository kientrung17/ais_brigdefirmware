#include "task/wifimanagertask.h"
#include "loggermanager.h"
#include "common/common.h"


WifiManagerTask::WifiManagerTask(WiFiManagerAbstract *wifimanager, std::string nameTask, int numElementQueueSet)
    : TaskAbstract(nameTask, numElementQueueSet)
{
    mWiFiManager = wifimanager;

    mSntpServer = SntpManager();
}

WifiManagerTask::~WifiManagerTask()
{
}

void WifiManagerTask::onTimer100HzProcess()
{
    mCounter100Hz++;

    // đồng bộ thời gian hệ thống với thời gian từ server sntp
    if (mStateSntpCurrent != StateSntp::Synced && mWifiStaInited && mWiFiManager->isWifiConnected() == true)
    {
        processInitSntpStateMachine(mStateSntpCurrent);
    }
    // Đồng bộ thời gian lại sau mỗi 1h
    if (mCounter100Hz % DIV_COUNTER_1_H == 0)
    {
        mStateSntpCurrent = StateSntp::Idle;
    }

    // div timer
    if (mCounter100Hz % DIV_COUNTER_10_S == 0)
    {
        // check disconneted
        if (mWiFiManager->isWifiConnected() == false && mWifiStaInited)
        {
            LOG_DEBUG("MonitorSystemTask", "Reconnect to wifi: ssid: %s, pass: %s",
                      mWiFiManager->getSsidWifiSTA().c_str(), mWiFiManager->getPassWifiSTA().c_str());
            mWiFiManager->reconnectWifi();
        }
    }
    if (mCounter100Hz % DIV_COUNTER_10Hz == 0)
    {
        if (mIsWifiStaConnected != mWiFiManager->isWifiConnected() && mWifiStaInited)
        {
            mIsWifiStaConnected = mWiFiManager->isWifiConnected();
            if (mIsWifiStaConnected == true)
            {
                LOG_INFO("WifiManagerTask", "Wifi sta connected");
                xEventGroupSetBits(gEventGroupNetworkState, BIT_WIFI_CONNECTED);
            }
            else
            {
                LOG_INFO("WifiManagerTask", "Wifi sta disconnected");
                xEventGroupClearBits(gEventGroupNetworkState, BIT_WIFI_CONNECTED);
            }
        }
    }
}

void WifiManagerTask::onQueueSetMessageProcess(OSBase::QueueHandle queue_sem)
{
    if (queue_sem == gSemInputBtnConfigFromRelayTaskToWifiTask)
    {
        mOSBase->semTake(gSemInputBtnConfigFromRelayTaskToWifiTask);
        LOG_DEBUG("WifiManagerTask", "[Queue Sem] Received sem gSemInputBtnConfigFromRelayTaskToWifiTask");
        //todo: bo comment sau khi hoàn thành chức năng đọc nút cấu hình
        // change mode wifi to AP + STA
        // changeModeWifiToAPSta();
    }

}

// init config wifi infor
void WifiManagerTask::onInitProcess()
{
    LOG_INFO("WifiManagerTask", "Init task");
    mStaConfig.ssid = gConfigSystem.getConfigSystem().ssidWifiSta;
    mStaConfig.password = gConfigSystem.getConfigSystem().passWifiSta;
    mApConfig.ssid = SSID_WIFI_AP;
    mApConfig.password = PASS_WIFI_AP;
    LOG_INFO("WifiManagerTask", "Init mode MODE_STA");
    mWiFiManager->initWifi(WiFiManagerAbstract::ModeWifi::MODE_AP_STA, mStaConfig, mApConfig);
    mWifiStaInited = true;
}

// check mode to init (Normal(wifi run Sta) or Config(wifi run Ap + STA))
void WifiManagerTask::changeModeWifiToAPSta()
{
    // return if wifi not init
    if (mWiFiManager->getModeWifi() == WiFiManagerAbstract::ModeWifi::MODE_AP_STA)
    {
        return;
    }
    LOG_INFO("WifiManagerTask", "Init mode MODE_AP_STA => Device on mode Config");
    mWiFiManager->changeWifiMode(WiFiManagerAbstract::ModeWifi::MODE_AP_STA, mStaConfig, mApConfig);
}

void WifiManagerTask::processInitSntpStateMachine(StateSntp stateSntp)
{
    switch (stateSntp)
    {
    case StateSntp::Idle:
    {
        // state reset
        LOG_INFO("WifiManagerTask", "SntpStateMachine: State idle");
        mCountCheckSntpSync = 0;
        mStateSntpCurrent = StateSntp::Init;
        processInitSntpStateMachine(mStateSntpCurrent);
        break;
    }
    case StateSntp::Init:
    {
        if (mServerType == ServerType::Ntp)
        {
            mSntpServer.beginUpdateSync(SERVER_SNTSP_NTP);
            LOG_INFO("WifiManagerTask", "SntpStateMachine: State Init: server Ntp");
        }
        else if (mServerType == ServerType::Google)
        {
            mSntpServer.beginUpdateSync(SERVER_SNTSP_GOOGLE);
            LOG_INFO("WifiManagerTask", "SntpStateMachine: State Init: server google");
        }
        else
        {
            mSntpServer.beginUpdateSync(SERVER_SNTP_CLOULDFARE);
            LOG_INFO("WifiManagerTask", "SntpStateMachine: State Init: server clouldfare");
        }
        // change state
        mStateSntpCurrent = StateSntp::WaitSync;
        processInitSntpStateMachine(mStateSntpCurrent);
        break;
    }
    case StateSntp::WaitSync:
    {
        if (mSntpServer.checkSntpSynced() == true)
        {
            LOG_INFO("WifiManagerTask", "SntpStateMachine: State WaitSync: server sysned");
            // change state
            mStateSntpCurrent = StateSntp::Synced;
            processInitSntpStateMachine(mStateSntpCurrent);
        }
        else
        {
            mCountCheckSntpSync++;
            // 10s không đồng bộ được thời gian thì chuyển server
            if (mCountCheckSntpSync == COUNTER_10_S)
            {
                LOG_INFO("WifiManagerTask", "SntpStateMachine: State WaitSync: sysnc faild => change server");
                // change state ChangeServer to start re begin new server
                mStateSntpCurrent = StateSntp::ChangeServer;
                processInitSntpStateMachine(mStateSntpCurrent);
            }
        }
        break;
    }
    case StateSntp::ChangeServer:
    {
        LOG_INFO("WifiManagerTask", "SntpStateMachine: State ChangeServer");
        // change server
        if (mServerType == ServerType::Ntp)
        {
            mServerType = ServerType::Google;
        }
        else if (mServerType == ServerType::Google)
        {
            mServerType = ServerType::Clouldfare;
        }
        else
        {
            mServerType = ServerType::Ntp;
        }
        // change state
        mStateSntpCurrent = StateSntp::Idle;
        processInitSntpStateMachine(mStateSntpCurrent);
        break;
    }
    case StateSntp::Synced:
    {
        // update status sntp system
        gIsSntpSynced = true;
        break;
    }
    }
}
