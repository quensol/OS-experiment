#include "spcb.h"
#define random(a,b) (rand() % (b-a+1))+a

/*
 * Getters and Setters
 */
int SPCB::getRuntime() const
{
    return runtime;
}

void SPCB::setRuntime(int value)
{
    runtime = value;
}

void SPCB::delRuntime(int value)
{
    if(runtime>0)
        runtime -= value;
    else
        return;
}

QString SPCB::getPID() const
{
    return PID;
}

void SPCB::setPID(int value)
{
    PID = "P"+QString::number(value,10);
    qDebug()<<PID;
}

QString SPCB::getPname() const
{
    return pname;
}

void SPCB::setPname(const QString &value)
{
    pname = value;
}

STATUS SPCB::getStatus() const
{
    return status;
}

void SPCB::setStatus(STATUS value)
{
    status = value;
}

int SPCB::getPriority() const
{
    return priority;
}

void SPCB::setPriority(int value)
{
    priority = value;
}

void SPCB::delPriority(int value)
{
    if(priority>0){
        priority -= value;
    }
    else
        return;
}

void SPCB::addPriority(int value)
{
    priority += value;
}

void SPCB::setNext(SPCB *value)
{
    next = value;
}

/**
 * Constructor
 * @brief SPCB::SPCB
 * @param pname
 * @param priority
 * @param runtime
 */
int SPCB::getMemoryNeed() const
{
    return memoryNeed;
}

void SPCB::setMemoryNeed(int value)
{
    memoryNeed = value;
}

int SPCB::getMemoryBegin() const
{
    return memoryBegin;
}

void SPCB::setMemoryBegin(int value)
{
    memoryBegin = value;
}

int SPCB::getColorOrder() const
{
    return colorOrder;
}

void SPCB::setColorOrder(int value)
{
    colorOrder = value;
}

void SPCB::refreshColor()
{
    colorOrder = 1;
}

SPCB::SPCB(QString pname, int priority, int runtime, int memoryNeed)
{
    //    srand(( (int)time(NULL) )%100);
    //    PID = "P"+QString::number(random(1000,9999),10);
    PID = "";
    this->pname = pname;
    this->priority = priority;
    this->runtime = runtime;
    //起初新建六个pcb，以状态值代表是否分配PCB，BACKUP表示未成为进程，无PCB
    status = BACKUP;
    next = nullptr;
    this->memoryNeed = memoryNeed;
    memoryBegin = 0;
    colorOrder = 0;
//    qDebug()<<PID;
}
