// WebServerEsp32.cpp
#include "common/webservermanager.h"
#include "common/common.h"
#include "common/storeflashmanager.h"
#include "common/webserversource.h"
#include "message/controlrelaymessage.h"
#include "task/powermanagertask.h"
#include "esp_log.h"
#include <cstdio>
#include <cstdlib>

static const char *TAG = "WebServerEsp32";

WebServerEsp32::WebServerEsp32() = default;

WebServerEsp32::~WebServerEsp32()
{
    stop();
}

bool WebServerEsp32::start()
{
    if (mServer)
    {
        ESP_LOGW(TAG, "Server already started");
        return true;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8192;
    esp_err_t ret = httpd_start(&mServer, &config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start server: %s", esp_err_to_name(ret));
        return false;
    }

    registerHandlers();
    ESP_LOGI(TAG, "Web server started");
    return true;
}

void WebServerEsp32::stop()
{
    if (mServer)
    {
        httpd_stop(mServer);
        mServer = nullptr;
        ESP_LOGI(TAG, "Web server stopped");
    }
}

void WebServerEsp32::registerHandlers()
{
    httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = handleRoot,
        .user_ctx = nullptr};
    httpd_register_uri_handler(mServer, &root_uri);

    httpd_uri_t relay_get_uri = {
        .uri = "/api/relay",
        .method = HTTP_GET,
        .handler = handleRelayGet,
        .user_ctx = nullptr};
    httpd_register_uri_handler(mServer, &relay_get_uri);

    httpd_uri_t relay_post_uri = {
        .uri = "/api/relay",
        .method = HTTP_POST,
        .handler = handleRelayPost,
        .user_ctx = nullptr};
    httpd_register_uri_handler(mServer, &relay_post_uri);

    httpd_uri_t power_get_uri = {
        .uri = "/api/power",
        .method = HTTP_GET,
        .handler = handlePowerGet,
        .user_ctx = nullptr};
    httpd_register_uri_handler(mServer, &power_get_uri);
}

esp_err_t WebServerEsp32::handleRoot(httpd_req_t *req)
{
    httpd_resp_send(req, SourceWebserver::html_config, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t WebServerEsp32::handleRelayGet(httpd_req_t *req)
{
    char response[512];
    size_t offset = 0;
    offset += snprintf(response + offset, sizeof(response) - offset, "{\"relays\":[");
    for (int i = 0; i < MessageCommon::MAX_NUM_RELAY; ++i)
    {
        int status = -1;
        if (gGpioRelay[i])
        {
            status = static_cast<int>(gGpioRelay[i]->Hal_Gpio_ReadPin());
        }
        else
        {
            ControlRelayMessage relayMsg;
            if (StoreFlashManager::getInstance()->readRelayInforFromFlash(&relayMsg, static_cast<uint8_t>(i)))
            {
                status = static_cast<int>(relayMsg.getControlRelayMessage().status_control);
            }
        }
        offset += snprintf(response + offset, sizeof(response) - offset,
                           "%s{\"channel\":%d,\"status\":%d}",
                           (i == 0) ? "" : ",", i, status);
    }
    offset += snprintf(response + offset, sizeof(response) - offset, "]}");

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
}

esp_err_t WebServerEsp32::handleRelayPost(httpd_req_t *req)
{
    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive POST data");
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    char channelStr[8] = {0};
    char statusStr[8] = {0};
    if (httpd_query_key_value(buf, "channel", channelStr, sizeof(channelStr)) != ESP_OK ||
        httpd_query_key_value(buf, "status", statusStr, sizeof(statusStr)) != ESP_OK)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing channel/status");
        return ESP_FAIL;
    }

    long channel = std::strtol(channelStr, nullptr, 10);
    long status = std::strtol(statusStr, nullptr, 10);
    if (channel < 0 || channel >= MessageCommon::MAX_NUM_RELAY || (status != 0 && status != 1))
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid channel/status");
        return ESP_FAIL;
    }

    if (gGpioRelay[channel])
    {
        auto pinState = status ? HalGpioAbstract::GpioPinState::GPIO_PIN_SET
                               : HalGpioAbstract::GpioPinState::GPIO_PIN_RESET;
        gGpioRelay[channel]->Hal_Gpio_WritePin(pinState);
    }

    AquaCtrl_ControlRelayData relayData = AquaCtrl_ControlRelayData_init_default;
    relayData.channel = static_cast<uint32_t>(channel);
    relayData.status_control = static_cast<uint32_t>(status);
    ControlRelayMessage relayMsg(relayData);
    StoreFlashManager::getInstance()->saveRelayInforToFlash(relayMsg, static_cast<uint8_t>(channel));

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, "{\"ok\":true}");
}

esp_err_t WebServerEsp32::handlePowerGet(httpd_req_t *req)
{
    if (!gPowerManagerTask)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Power monitor unavailable");
        return ESP_FAIL;
    }

    const auto &latest = gPowerManagerTask->latest();
    char response[256];
    snprintf(response, sizeof(response),
             "{\"ampe\":[%.2f,%.2f],\"voltage\":[%.2f,%.2f],\"lost_phase\":%d,\"lost_electric\":%d}",
             static_cast<double>(latest.ampe[0]),
             static_cast<double>(latest.ampe[1]),
             static_cast<double>(latest.voltage[0]),
             static_cast<double>(latest.voltage[1]),
             gPowerManagerTask->isLostPhase() ? 1 : 0,
             gPowerManagerTask->isLostElectric() ? 1 : 0);

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
}
