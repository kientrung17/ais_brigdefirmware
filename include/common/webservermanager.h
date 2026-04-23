#pragma once

#include "esp_http_server.h"
#include "HAL/HAL_ABSTRACT/webservermanagerabstract.h"

class WebServerEsp32 : public WebServerAbstract
{
public:
    WebServerEsp32();
    ~WebServerEsp32();

    bool start() override;
    void stop() override;

private:
    httpd_handle_t mServer = nullptr;

    static esp_err_t handleRoot(httpd_req_t *req);
    static esp_err_t handleRelayGet(httpd_req_t *req);
    static esp_err_t handleRelayPost(httpd_req_t *req);
    static esp_err_t handlePowerGet(httpd_req_t *req);

    void registerHandlers();
};
