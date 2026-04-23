// EspNowManager.h

#pragma once
#include "esp32nowmanagerabstract.h"
#include <esp_now.h> // ESP-NOW API types

/**
 * @brief  A thin C++ wrapper around ESP-NOW functionality.
 *         Allows initializing ESP-NOW, managing peers, sending data,
 *         and registering a receive callback.
 */
class EspNowManager : public Esp3232NowManagerAbstract
{
public:
    /**
     * @brief  Construct a new EspNowManager.
     *         Does not initialize ESP-NOW; call init() explicitly.
     */
    EspNowManager();

    ~EspNowManager();

    virtual bool init() override;
    virtual bool addPeer(const uint8_t *peer_addr, uint8_t channel = 0, bool encrypt = false) override;
    virtual bool removePeer(const uint8_t *peer_addr);
    virtual bool sendData(const uint8_t *peer_addr, const uint8_t *data, size_t len) override;
    virtual void setSendCallbcak(SendCallback callback) override;
    virtual void setReceiveCallbcak(ReceivevCallback callback) override;
    
    static EspNowManager* getInatnce();
private:
    static EspNowManager *mInstance;
    /**
     * @brief  Internal callback invoked by ESP-NOW when a send completes.
     * @param  mac_addr:  MAC address of the destination peer.
     * @param  status:    ESP_NOW_SEND_SUCCESS or ESP_NOW_SEND_FAIL.
     */
    void onSend(const uint8_t *mac_addr, esp_now_send_status_t status);

    /**
     * @brief  Internal callback invoked by ESP-NOW when data is received.
     * @param  mac_addr:  MAC address of the sender.
     * @param  data:      Pointer to received data buffer.
     * @param  len:       Length of received data in bytes.
     */
    void onRecv(const esp_now_recv_info_t *info, const uint8_t *data, int data_len);
};
