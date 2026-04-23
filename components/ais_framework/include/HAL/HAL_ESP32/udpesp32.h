#ifndef UDPESP32_H
#define UDPESP32_H

#include <stdint.h>
#include "lwip/sockets.h"
#include "udpabstract.h"

class UdpEsp32 : public UdpAbstract
{
public:
    // Constructor: Cấu hình IP, cổng local, cổng remote và bộ đệm
    UdpEsp32(const char *ip, uint16_t localPort, uint16_t remotePort);
    UdpEsp32(uint16_t localPort);

    // Destructor: Đảm bảo đóng socket khi không sử dụng
    ~UdpEsp32();

    // Khởi tạo UDP
    void init() override;

    // Gửi dữ liệu
    void send(const uint8_t *data, int length) override;

    // Đọc dữ liệu UDP
    int read(uint8_t* data, uint16_t maxLength) override;

    // Kiểm tra xem có dữ liệu UDP mới hay không
    bool checkAvailable() override;

    //update remote
    void setRemoteConfig(const char *ip, uint16_t port) override;
    void updateRemoteFromRemoteOfDataReceive() override;
private:
    int sockfd;                    // Socket UDP
    struct sockaddr_in mRemoteAddr; // Địa chỉ remote
    struct sockaddr_in mLocalAddr;  // Địa chỉ local
    struct sockaddr_in mRemoteAdressReceivedData; //địa chỉ máy vừa gửi dữ liệu đên
};

#endif // UDPESP32_H
