#ifndef LOGGER_MANAGER_H
#define LOGGER_MANAGER_H

#include "logger.h"

#define LOG_INFO(tag, fmt, ...) \
    LoggerManager::GetInstance()->LogInfoEx(tag, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define LOG_ERROR(tag, fmt, ...) \
    LoggerManager::GetInstance()->LogErrorEx(tag, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define LOG_DEBUG(tag, fmt, ...) \
    LoggerManager::GetInstance()->LogDebugEx(tag, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define LOG_ERROR_CHECK(err) \
    LoggerManager::GetInstance()->LogCheckError(err, __FILE__, __LINE__, __func__)
    
#define LOG_ESP32_LOGGER
// #define LOG_UDP_LOGGER
#define LOG_UDP_IP "192.168.4.2"
#define LOG_UDP_PORT "8888"

//example
// LOG_INFO("NET", "Kết nối WiFi thành công: %s", ssid);
// LOG_ERROR("I2C", "Truyền dữ liệu lỗi: mã %d", err);

class LoggerManager {
    public:
        static Logger* GetInstance();  // trả về logger mặc định (EspLogger, UdpLogger...)
    
    private:
        static Logger* mInstance;
};

#endif