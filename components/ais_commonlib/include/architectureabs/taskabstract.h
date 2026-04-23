/*
 * File:   processorabstract.h
 * Author: sangnv17@viettel.com.vn
 *
 * Created on December 1, 2019, 6:00 PM
 */
#ifndef SERVICEABSTRACT_H
#define SERVICEABSTRACT_H

#include <memory>
#include <stdio.h>
#include "OSBase.h"
#include <vector>
/*
 * @brief TaskAbstract
 * @param processorName
 * abstract class for all processor
 */
class TaskAbstract
{
public:
    TaskAbstract(const std::string &processorName, int numElementQueueset);
    virtual ~TaskAbstract();

    virtual void onTimer100HzProcess() = 0;
    virtual void onQueueSetMessageProcess(OSBase::QueueHandle queue_sem) = 0;
    virtual void onInitProcess() = 0;

public:
    std::string getTaskName() const;
    OSBase::SemHandle getSemaphore100Hz();
    // onStartProcess on init task to run task
    void onStartProcess();
    void onStopProcess();
    bool initRegisterSemaphoreToQueueset(OSBase::SemHandle *sem);
    bool initregisterQueueToQueueset(OSBase::QueueHandle *queue, int sizeElement, int numElement);

    bool registerSemaphoreToQueueset(OSBase::SemHandle sem);
    bool registerQueueToQueueset(OSBase::QueueHandle queue);

private:
    TaskAbstract(const TaskAbstract &) = delete;
    TaskAbstract(TaskAbstract &&) = delete;
    TaskAbstract &operator=(const TaskAbstract &) = delete;
    TaskAbstract &operator=(TaskAbstract &&) = delete;

private:
    std::string mTaskName;
    OSBase::SemHandle mMonitorSemaphore100Hz;
    OSBase::QueueSetHandle mQueueSet;
};

#endif // SERVICEABSTRACT_H
