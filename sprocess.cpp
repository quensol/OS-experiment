#include "sprocess.h"

/*
 * Constuctors
 */
int SProcess::getMemoryNeed() const
{
    return memoryNeed;
}

void SProcess::setMemoryNeed(int value)
{
    memoryNeed = value;
}

SProcess::SProcess(){
    pname = "";
    runTime = 0;
    priority = 0;
    memoryNeed = 0;
    pstatus = BACKUP;
    pcb = new SPCB(pname,runTime,priority,memoryNeed);

}

SProcess::SProcess(QString pname, int runTime, int priority, int memoryNeed)
{
    this->pname = pname;
    this->runTime = runTime;
    this->priority = priority;
    this->memoryNeed = memoryNeed;
    pstatus = BACKUP;
    pcb = new SPCB(pname,runTime,priority,memoryNeed);
}

SProcess::~SProcess(){
    if(pcb!=nullptr) delete pcb;
}

/*
 * Getters and Setters
 */
void SProcess::setPname(const QString &value)
{
    pname = value;
}

void SProcess::setRunTime(int runtime){
    runTime = runtime;
}

void SProcess::setPriority(int prt){
    priority = prt;
}

void SProcess::setPstatus(STATUS status){
    pstatus = status;
}

void SProcess::setPCB(QString pname, int runtime, int priority, STATUS status, int memoryNeed){
    if(pcb!=nullptr){
        pcb->setPname(pname);
        pcb->setPriority(priority);
        pcb->setRuntime(runtime);
        pcb->setStatus(status);
        pcb->setMemoryNeed(memoryNeed);
    }
    else{
        qDebug()<<"pcb pointer is null error in setPCB";
    }
}

/**
 * @brief SProcess::setDefaultPCB
 * 设置默认的PCB值
 */
void SProcess::setDefaultPCB()
{
    if(pcb!=nullptr){
        pcb->setPname(pname);
        pcb->setPriority(priority);
        pcb->setRuntime(runTime);
        pcb->setStatus(pstatus);
        pcb->setMemoryNeed(memoryNeed);
        pcb->setMemoryBegin(0);
        pcb->setColorOrder(0);
    }
    else{
        qDebug()<<"pcb pointer is null error in setDefaultPCB";
    }
}

QString SProcess::getPname() const{
    return pname;
}

int SProcess::getRunTime() const{
    return runTime;
}

int SProcess::getPriority() const{
    return priority;
}

STATUS SProcess::getPstatus() const{
    return pstatus;
}

SPCB *SProcess::getPCB() const{
    if(pcb!=nullptr){
        return pcb;
    }
    else{
        qDebug()<<"pcb pointer is null error";
    }
}
