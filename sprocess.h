
#ifndef SPROCESS_H
#define SPROCESS_H

#include <QString>
#include "spcb.h"

class SProcess
{
private:
    QString pname;
    int runTime;
    int priority;
    STATUS pstatus;
    int memoryNeed;
    int memoryBegin;
    SPCB *pcb;

public:
    SProcess();
    SProcess(QString pname, int runTime, int priority,int memoryNeed);
    ~SProcess();
    void setPname(const QString &value);
    void setRunTime(int runtime);
    void setPriority(int prt);
    void setPstatus(STATUS status);
    void setMemoryNeed(int value);
    void setPCB(QString pname, int runtime, int priority, STATUS status, int memoryNeed);//PCB修改模块
    void setDefaultPCB();//默认PCB设置模块

    QString getPname() const;
    int getRunTime() const;
    int getPriority() const;
    int getMemoryNeed() const;
    STATUS getPstatus() const;
    SPCB *getPCB() const;
};

#endif // SPROCESS_H
