
#pragma once
#include <stdio.h>
#include <functional>

class Esp3232NowManagerAbstract
{
public:
    using SendCallback = std::function<void(const uint8_t *macadrr, bool &status)>;
    using ReceivevCallback = std::function<void(const uint8_t *macadrr, const uint8_t *data, int len)>;
    /**
     * @brief  Initialize ESP-NOW subsystem.
     * @note   Assumes that Wi-Fi (STA or AP) has already been initialized and started.
     * @return true if ESP-NOW initialized successfully, false otherwise.
     */
    virtual bool init() = 0;

    /**
     * @brief  Add a peer to the ESP-NOW peer list.
     * @param  peer_addr: MAC address of the peer (6 bytes).
     * @param  channel:   Wi-Fi channel to communicate on (default 0).
     * @param  encrypt:   Whether to enable ESP-NOW encryption (default false).
     * @return true if peer added (or already exists), false on error.
     * @note   Must be called before sendData() to that peer.
     */
    virtual bool addPeer(const uint8_t *peer_addr,
                         uint8_t channel = 0,
                         bool encrypt = false) = 0;

    /**
     * @brief  Remove a peer from the ESP-NOW peer list.
     * @param  peer_addr: MAC address of the peer to remove.
     * @return true if peer removed successfully, false otherwise.
     */
    virtual bool removePeer(const uint8_t *peer_addr) = 0;

    /**
     * @brief  Send a raw byte buffer to a registered peer.
     * @param  peer_addr: MAC address of the destination peer.
     * @param  data:      Pointer to the data buffer.
     * @param  len:       Number of bytes to send.
     * @return true if the send request was accepted, false on error.
     * @note   Actual delivery success or failure reported via onSend callback.
     */
    virtual bool sendData(const uint8_t *peer_addr, const uint8_t *data, size_t len) = 0;

    /**
     * @brief  Register a user callback for incoming ESP-NOW data.
     * @param  cb: function to call when data arrives.
     *             Signature: void cb(const uint8_t* mac_addr,
     *                                const uint8_t* data,
     *                                int len)
     */
    virtual void setSendCallbcak(SendCallback callback) = 0;
    virtual void setReceiveCallbcak(ReceivevCallback callback) = 0;

protected:
    bool mInitialized {false};
    SendCallback mSendCallback{nullptr};
    ReceivevCallback mReceivevCallback{nullptr};
};
