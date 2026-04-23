#ifndef EVENTTIMERBASE_H_
#define EVENTTIMERBASE_H_
#include <stdio.h>

typedef void (*pfuncAlarmCallBack) (void* parg);

static constexpr uint8_t NUM_TIMER_MAX = 8;

enum TIMER_ID_t {
	TIMER_ID_0 = 0,
	TIMER_ID_1,
	TIMER_ID_2,
	TIMER_ID_3,
	TIMER_ID_4,
	TIMER_ID_5,
	TIMER_ID_6,
	TIMER_ID_MAX
};

class EventTimerBase {
public:
	virtual void init() = 0;
	virtual bool registerTimer(TIMER_ID_t id, uint32_t freq, const pfuncAlarmCallBack handle, void* parg) = 0;
	virtual void reset(TIMER_ID_t id) = 0;
	virtual void start(TIMER_ID_t id) = 0;
	virtual void stop(TIMER_ID_t id) = 0;
protected:
    /// Default constructor is disabled
    EventTimerBase(void){};
    /// Destructor is disabled (releasing the object by a pointer to the base class)
    virtual ~EventTimerBase(void){};

private:
    // Copy constructor is disabled
    EventTimerBase(EventTimerBase&);
	// Copy operator is disabled
    EventTimerBase& operator=(const EventTimerBase&);
};
#endif
