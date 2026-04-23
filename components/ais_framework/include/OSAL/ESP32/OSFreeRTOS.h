/*
 * OSFreeRTOS.h
 *
 *  Created on: May 13, 2024
 *      Author: kienvd3
 */

#ifndef OSFREERTOS_H_
#define OSFREERTOS_H_

#ifdef STM32F407xx
#include "cmsis_os.h"
#endif
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "../Platform/OSBase.h"

class OSFreeRTOS : public OSBase{
public:
	OSFreeRTOS(void);

	typedef struct task_infor
	{
		TaskHandle_t handle;
		uint8_t idTask;
		uint16_t stackDepth;
		uint32_t lastUlRunTimeCounter;
	}task_infor_t;

    virtual void init (void);
    virtual void start (void); //Method explicitly starts multitasking in the operating system.
    virtual bool isStarted (void);	///< Method checks whether 'start' function was executed

    virtual bool taskCreate (char* taskName, TaskProc taskFunction, PRIORITY_TASK taskPriority, uint32_t stackSize, uint8_t taskId); //Method creates new thread of the operating system.

    virtual EvtHandle evtCreate ();	///< Method creates an event groups
	virtual EVTCODE evtPost (EvtHandle eh, EvtMask flags, bool setFlags);	///< Method sets or resets the event flags
	virtual EVTCODE evtWaitAny (EvtHandle eh, EvtMask &flags, bool bConsume=true);	///< Method performs waiting for any event from the group

	virtual SemHandle semCreate ();	///< Method creates new semaphore
	virtual SEMCODE semTake (SemHandle sh, int mSec = -1);	///< Method performs locking of the semaphore
	virtual SEMCODE semGive (SemHandle sh);	///< Method performs releasing of the semaphore

	virtual QueueHandle queueCreate(int queueLength, int queueItemSize); ///< Method creates new queue
	virtual QUEUECODE queueSend(QueueHandle qh, const void * item); ///< Method send an item to a queue.
	virtual QUEUECODE queueReceive(QueueHandle qh, void * buffer, int mSec = -1); ///< Method receive an item from a queue

	virtual QueueSetHandle queueSetCreate(int queueSetLength);
	virtual QUEUESETCODE queueSetAddQueue(QueueHandle qh, QueueSetHandle qsh);
	virtual QUEUESETCODE queueSetAddSemaphore(SemHandle sh, QueueSetHandle qsh);
	virtual QueueSetMember queueSetWaitAny(QueueSetHandle qsh);

    virtual void sleepMs (int mSec); //Method sleeps the thread execution for a specified time (in milliseconds).
    uint32_t getTimeSystem () override;

    uint8_t getAllTaskInfor(task_monitor_infor_t taskmonitorInfor[], uint8_t numElementTask, uint8_t* heapUsage, uint8_t *cpuUsageIdle) override;
    
    virtual QueueHandle setQueueHandleInited(void* queueHande) override;
	virtual SemHandle setSemHandleInited(void* semHandle) override;
    ~OSFreeRTOS(void){};
private:
    EventGroupHandle_t _evtgTab[MAX_EVT_GROUPS];  ///< Array of pointers to the structures of FreeRTOS event groups
    int _nEvtg {0};
    // SemHandle is int begin from 31(MAX_QUEUES+1) to 71 (MAX_QUEUES + MAX_SEMAPHORES)
    SemaphoreHandle_t _semTab[MAX_SEMAPHORES];   ///< Array with pointers to the structures of FreeRTOS semaphores
    int _nSem {0};									///< Number of created semaphores
    // QueueHandle is int begin from 1 to 30 (MAX_QUEUES)
    QueueHandle_t _queueTab[MAX_QUEUES];		///< Array with pointers to the structures of FreeRTOS queues
    int _nQueue {0};								///< Number of created queues
    QueueSetHandle_t _queueSetTab[MAX_QUEUESET];
    int _nQueueSet {0};

    task_infor_t mTaskInforArray[MAX_TASK_MONITOR] {};
    uint8_t mCountNumTask {0};
    //cpu
    bool mIsFirstCalCpu {true};
    uint32_t mLastTotalRuntime {0};
    uint32_t mLatsUlRunTimeCounterIdle {0};
};

#endif /* OSFREERTOS_H_ */
