/*
 * OSBase.h
 *
 *  Created on: May 13, 2024
 *      Author: kienvd3
 */

#ifndef OSBASE_H_
#define OSBASE_H_
#include <stdint.h>
#include <cstddef>
typedef void		(*TaskProc)(void*);

class OSBase {
public:
	typedef int SemHandle;
	typedef int EvtHandle;
	typedef int QueueHandle;
	typedef int QueueSetMember;
	typedef int QueueSetHandle;
	typedef unsigned int EvtMask;
	static const int MAX_SEMAPHORES = 50;   ///< Maximum amount of system semaphores
    static const int MAX_EVT_GROUPS = 30;   ///< Maximum amount of events group
	static const int EVT_IN_GROUP   = 24;   ///< There are 24 bit flag within an event group. because configUSE_16_BIT_TICKS is define to 0
	static const int MAX_QUEUES = 50;   ///< Maximum amount of events group
	static const int MAX_QUEUESET = 50;   ///< Maximum amount of events group

	static const int MAX_TASK_MONITOR = 20;
	typedef struct task_monitor_infor
	{
		uint8_t idTask;
		uint8_t cpuUsage;
		uint8_t stackUsage;
	}task_monitor_infor_t;

    enum SEMCODE
    {
        SEM_OK = 0,     ///< All correct
        SEM_FIXED,      ///< Ok, but setting the amount of semaphores was required (too many unlocks)
        SEM_TIMEOUT,    ///< Semaphore unlocking timeout
        SEM_OTHER       ///< Other errors
    };

    enum EVTCODE
    {
        EVT_OK = 0,     ///< Ok
        EVT_OTHER       ///< Other errors
    };

    enum QUEUECODE
	{
    	QUEUE_OK = 0,	///< Ok
		QUEUE_OTHER		///< Other errors
	};

    enum QUEUESETCODE
	{
    	QUEUESET_OK = 0,	///< Ok
		QUEUESET_OTHER		///< Other errors
	};

    enum PRIORITY_TASK
    {
		PRIORITY_LOW = 1,
		PRIORITY_BELOWNORMAL,
		PRIORITY_NORMAL,
		PRIORITY_ABOVENORMAL,
		PRIORITY_HIGH,
		PRIORITY_REALTIME,
    };

	/// Pure virtual method.
    virtual void init (void) = 0;

    /// Pure virtual method. Method explicitly starts multitasking in the operating system.
    virtual void start (void) = 0;

	/// Pure virtual method. Method returns bool value whether multitasking has been started
    virtual bool isStarted (void) = 0;

	/** \name Pure virtual method. Method creates new thread of the operating system. Thread is created as a immediately running.
	* \param 'taskName' - name of task
	* \param 'taskFunction' - argument of a handler function
	* \param 'taskPriority' - task priority (-3 -> +3) lowest number is idle priority, highest number is realtime priority
	* \param 'stackSize' - stack size requirements in bytes; 0 is default stack size
	* \return 'true' if task was created successfully, 'false' otherwise.
	*/
    virtual bool taskCreate (char* taskName, TaskProc taskFunction, PRIORITY_TASK taskPriority, uint32_t stackSize, uint8_t taskId) = 0;

	/** \name Pure virtual method. Method creates a group of binary events. The event flags creating this group are resetted.
	* \param 'errCode' - operating system specific error code. If NULL no error is returned. NOTE: this argument is used only for statistic purposes, do not use it to control the programs flow.
	* \return A handle to a group of binary events or '0' if error occured.
	*/
    virtual EvtHandle evtCreate () = 0;

	/** \name Pure virtual method. Method sets/resets an event occurrence flag.
	* \param 'eh' - a handle to the group of binary events
	* \param 'flags' - binary flags correcponding to event states
	* \param 'setFlags' - flags modificaion mode: 'true' - set flag mode, 'false' - reset flag mode
	* \return 'EVT_OK' - ok, 'EVT_OTHER' - other error
	*/
	virtual EVTCODE evtPost (EvtHandle eh, EvtMask flags, bool setFlags) = 0;

	/** \name Pure virtual method. Method waits (without timeouting) on an occurrence of any event from specified events group. Method resets flags of the events that have occurred.
	* \param 'eh' - a handle to the events group
	* \param 'flags' - returned flags corresponding to the occurred events
	* \param 'bConsume' - 'true' - the set event flag is resetted, 'false' - flag remains unchanged
	* \return 'EVT_OK' - ok, 'EVT_OTHER' - other error
	*/
	virtual EVTCODE evtWaitAny (EvtHandle eh, EvtMask &flags, bool bConsume=true) = 0;

	/** \name Pure virtual method. Method creates new binary semaphore (with value of 0 or 1).
	* Semaphore is initialized with value of 1 which means that protected resource is accessible.
	* Method returns handle to the semaphore or '0' is any error occured.
	*/
	virtual SemHandle semCreate () = 0;

	/** \name Pure virtual method. Method blocks access to the resource.
	* \param 'sh' - handle to a semaphore
	* \param 'mSec' - number of milliseconds to wait for the release of semaphore in use,  -1 mean wait forever
	* \return 'SEM_OK' - on success, 'SEM_TIMEOUT' - semaphore unlocking timeout, 'SEM_OTHER' - other error
	*/
	virtual SEMCODE semTake (SemHandle sh, int mSec = -1) = 0;

	/** \name Pure virtual method. Method unlocks the semaphore.
	* \param 'sh' - handle to a semaphore
	* \return 'SEM_OK' - on success, 'SEM_FIXED' - ok, 'SEM_OTHER' - other error
	*/
	virtual SEMCODE semGive (SemHandle sh) = 0;

	/** \name Pure virtual method. Method create new queue.
	* \param 'queueLength' - number of items in queue
	* \param 'queueItemSize' - size of 1 item queue (bytes)
	* Method returns handle to the queue or '0' is any error occured.
	*/
	virtual QueueHandle queueCreate(int queueLength, int queueItemSize) = 0;

	/** \name Pure virtual method. Method send an item to a queue.
	* \param 'qh' - handle to a queue
	* \param 'item' - pointer to item
	* \param 'timeWaitmSec' - timeout(ms) to wait while queue is full
	* \return 'QUEUE_OK' - on success, 'QUEUE_OTHER' - other error
	*/
	virtual QUEUECODE queueSend(QueueHandle qh, const void * item) = 0;

	/** \name Pure virtual method. Method receive an item from a queue. Method will block task until there are at least 1 item in queue
	* \param 'qh' - handle to a queue
	* \param 'buffer' - pointer to buffer to store item
	* \param 'mSec' - number of milliseconds to wait for the item present in queue, -1 mean wait forever
	* \return 'QUEUE_OK' - on success, 'QUEUE_OTHER' - other error
	*/
	virtual QUEUECODE queueReceive(QueueHandle qh, void * buffer, int mSec = -1) = 0;

	/** \name Pure virtual method. Method create new queueSet.
	* \param 'queueSetLength' - length of queueset = sum length of all queue in queueset
	* Method returns handle to the queueSet or '0' is any error occured.
	*/
	virtual QueueSetHandle queueSetCreate(int queueSetLength) = 0;

	/** \name Pure virtual method. Method add a queue to a queueset.
	* \param 'qh' - handle to a queue
	* \param 'qsh' - handle to a queueset
	* \return 'QUEUESET_OK' - on success, 'QUEUESET_OTHER' - other error
	*/
	virtual QUEUESETCODE queueSetAddQueue(QueueHandle qh, QueueSetHandle qsh) = 0;

	/** \name Pure virtual method. Method add a semaphore to a queueset.
	* \param 'sh' - handle to a semaphore
	* \param 'qsh' - handle to a queueset
	* \return 'QUEUESET_OK' - on success, 'QUEUESET_OTHER' - other error
	*/
	virtual QUEUESETCODE queueSetAddSemaphore(SemHandle sh, QueueSetHandle qsh) = 0;

	/** \name Pure virtual method. Method wait for an item on a queue or semaphore on a queuese. Method will block task until there are at least 1 item a queue
	* \param 'qsh' - handle to a queueset
	* \return 'QUEUESET_OK' - on success, 'QUEUESET_OTHER' - other error
	*/
	virtual QueueSetMember queueSetWaitAny(QueueSetHandle qsh) = 0;

	/** \name Pure virtual method. Method sleeps the thread execution for a specified time (in milliseconds).
	* Specified time is rounded down or up to the nearest multiple of a system ticks.
	*/
    virtual void sleepMs (int mSec) = 0;

    /*
     * return: time system (ms)
     */
    virtual uint32_t getTimeSystem () = 0;

    /*
     * param: taskmonitorInfor: return task infor
     * param: numElementTask: num element of taskmonitorInfor[]
     * return: num of task monitor
     * Note:
     * enable in FreeRTOSConfig.h
     				   FREERTOS.INCLUDE_uxTaskGetStackHighWaterMark=1
					   GENERATE_RUN_TIME_STATS = 1
					   USE_TRACE_FACILITY  = 1
					   USE_STATS_FORMATTING_FUNCTIONS  = 1
					   RECORD_STACK_HIGH_ADDRESS = 1
	 *implement function: extern void configureTimerForRunTimeStats(void);
       	   	   	   	   	  extern unsigned long getRunTimeCounterValue(void);
     *
     *
     */
    virtual uint8_t getAllTaskInfor(task_monitor_infor_t taskmonitorInfor[], uint8_t numElementTask, uint8_t* heapUsage, uint8_t *cpuUsageIdle) = 0;

    /*
     * method dynamic allocate memory of OS
     * param: xWantedSize: size of memory to allocate (bytes)
     */
    static void *memAlloc(size_t xWantedSize);

    /*
     * method free memory of OS
     * param: *ptr: pointer to memory to be freed
     */
    static void memFree(void *ptr);

	//update queue and semaphore handle when init before
	virtual QueueHandle setQueueHandleInited(void* queueHande) = 0;
	virtual SemHandle setSemHandleInited(void* semHandle) = 0;
protected:
    /// Default constructor is disabled
    OSBase(void){};
    /// Destructor is disabled (releasing the object by a pointer to the base class)
    virtual ~OSBase(void){};

private:
    // Copy constructor is disabled
    OSBase(OSBase&);
	// Copy operator is disabled
    OSBase& operator=(const OSBase&);

};

#endif /* OSBASE_H_ */
