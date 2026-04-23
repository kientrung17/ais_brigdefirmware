#ifndef UDP_LOGGER_H
#define UDP_LOGGER_H

#include "logger.h"
#include <string>
#include <sys/socket.h>
/*
Example:
Logger* logger = new UdpLogger("192.168.1.100", 9999);

LOG_INFO(logger, "APP", "Khởi động hệ thống: phiên bản %s", "v1.2.3");
LOG_ERROR(logger, "SENSOR", "Lỗi đọc cảm biến tại mA = %.2f", 13.67);
*/
class UdpLogger : public Logger {
public:
    UdpLogger(const std::string& ip, uint16_t port);
    ~UdpLogger();

    void LogDebugEx(const char* tag, const char* file, int line, const char* func, const char* fmt, ...) override;
    void LogInfoEx(const char* tag, const char* file, int line, const char* func, const char* fmt, ...) override;
    void LogErrorEx(const char* tag, const char* file, int line, const char* func, const char* fmt, ...) override;
    void LogCheckError(int err, const char *file, int line, const char *func) override;

private:
    int sock;
    struct sockaddr_in dest_addr;

    std::string get_timestamp();
    void send_formatted(const std::string& level, const std::string& tag,
                        const char* file, int line, const char* func, const char* msg);
};

#endif
