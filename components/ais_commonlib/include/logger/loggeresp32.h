#ifndef ESP_LOGGER_H
#define ESP_LOGGER_H

#include "logger.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdarg.h>
#include "esp_err.h"
class EspLogger : public Logger
{
public:
    void LogDebugEx(const char *tag, const char *file, int line, const char *func, const char *fmt, ...) override
    {
        char msg[256];
        va_list args;
        va_start(args, fmt);
        vsnprintf(msg, sizeof(msg), fmt, args);
        va_end(args);

        char fullTag[64];
        snprintf(fullTag, sizeof(fullTag), "[debug]%s", tag);
        ESP_LOG_LEVEL_LOCAL(ESP_LOG_INFO, fullTag, "[%s:%d:%s] %s", file, line, func, msg);
    }

    void LogInfoEx(const char *tag, const char *file, int line, const char *func, const char *fmt, ...) override
    {
        char msg[256];
        va_list args;
        va_start(args, fmt);
        vsnprintf(msg, sizeof(msg), fmt, args);
        va_end(args);

        char fullTag[64];
        snprintf(fullTag, sizeof(fullTag), "[info]%s", tag);
        ESP_LOG_LEVEL_LOCAL(ESP_LOG_INFO, fullTag, "[%s:%d:%s] %s", file, line, func, msg);
    }

    void LogErrorEx(const char *tag, const char *file, int line, const char *func, const char *fmt, ...) override
    {
        char msg[256];
        va_list args;
        va_start(args, fmt);
        vsnprintf(msg, sizeof(msg), fmt, args);
        va_end(args);

        char fullTag[64];
        snprintf(fullTag, sizeof(fullTag), "[error]%s", tag);
        ESP_LOG_LEVEL_LOCAL(ESP_LOG_ERROR, fullTag, "[%s:%d:%s] %s", file, line, func, msg);
    }

    void LogCheckError(int err, const char *file, int line, const char *func) override
    {
        if (err)
        {
            ESP_LOG_LEVEL_LOCAL(ESP_LOG_ERROR, "ESP_CHECK",
                                "[%s:%d:%s] Error code: 0x%x (%s)",
                                file, line, func, err, esp_err_to_name(err));
        }
    }
};
#endif
