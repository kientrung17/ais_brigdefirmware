#ifndef UDPABSTRACT_H
#define UDPABSTRACT_H

#include <stdint.h> // Để sử dụng uint8_t

class UdpAbstract
{
public:
    // Khởi tạo UDP
    virtual void init() = 0;

    // Gửi dữ liệu qua UDP
    virtual void send(const uint8_t *data, int length) = 0;

    // Đọc dữ liệu UDP, trả về độ dài dữ liệu đọc được
    //using
    //if(checkAvailable())
    // {
    //     uint8_t data[100];
    //     int lengthDataReceived = read(data, 100);
    // }
    virtual int read(uint8_t* data, uint16_t maxLength) = 0;

    // Kiểm tra xem có dữ liệu UDP mới hay không
    virtual bool checkAvailable() = 0;

    //set remote port
    //when has data from other device, it stored
    // and function updateRemoteFromRemoteOfDataReceive will update remote to remote reveived
    virtual void updateRemoteFromRemoteOfDataReceive() = 0;
    virtual void setRemoteConfig(const char *ip, uint16_t port) = 0;

protected:
    const char *mIP;      // Địa chỉ IP của remote
    uint16_t mLocalPort;  // Cổng local
    uint16_t mRemotePort; // Cổng remote
};

#endif // UDPABSTRACT_H
