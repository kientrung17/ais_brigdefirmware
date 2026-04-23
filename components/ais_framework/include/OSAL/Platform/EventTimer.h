/*
 * EventTimer.h
 *
 *  Created on: May 16, 2024
 *      Author: LUCLV4
 */

#ifndef FRAMEWORK_OSAL_PLATFORM_EVENTTIMER_H_
#define FRAMEWORK_OSAL_PLATFORM_EVENTTIMER_H_

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "EventTimerBase.h"

class EventTimer: public EventTimerBase {
public:
	EventTimer(void);
	virtual void init();
	virtual bool registerTimer   (TIMER_ID_t id, uint32_t freq, const pfuncAlarmCallBack handle, void* parg);
	virtual void reset			(TIMER_ID_t id);
	virtual void start     		(TIMER_ID_t id);
	virtual void stop 	 		(TIMER_ID_t id);

private:
	struct TriggerTimer
	{
		TIMER_ID_t 				id;         ///< Identifier number of the timer trigger.
		uint32_t 				ticks;      ///< The variable counter in normal mode.
		uint32_t 				frequency;  ///< The period of the timer trigger (Hz)
		void* 					parg;    ///< The pointer to parameters of the timer trigger.
		pfuncAlarmCallBack 		callback;   ///< Alarm callback function when a interrupt service routine occurs.
		bool					isStarted;	// Started or not to prevent duplicate
		TimerHandle_t			alarm;		///< Alarm instance when use Altera HAL Board Support Package.

	private:
		TriggerTimer& operator=(const TriggerTimer&);
	};

	static TriggerTimer 	mTimer[NUM_TIMER_MAX];
	static void pxCallbackFunction(TimerHandle_t xTimer);
};

#endif /* FRAMEWORK_OSAL_PLATFORM_EVENTTIMER_H_ */
