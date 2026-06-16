#ifndef LORA_BRIDGE_TASK_H_
#define LORA_BRIDGE_TASK_H_

#include "TaskAbstract.h"
#include "sx127x.h"

#define ID_LORA_BRIDGE_TASK 8

class LoraBridgeTask : public TaskAbstract {
public:
    LoraBridgeTask(const std::string& taskName, uint8_t maxElementQueueSet);
    virtual ~LoraBridgeTask();

protected:
    void onInitProcess() override;
    void onTimer100HzProcess() override;
    void onQueueSetMessageProcess(OSBase::QueueHandle queue_sem) override {}

private:
    SX127x lora;
    uint8_t mBroadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    void processLoraRx();
};

#endif /* LORA_BRIDGE_TASK_H_ */
