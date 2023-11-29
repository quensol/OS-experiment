
#ifndef SPCB_H
#define SPCB_H

#include <QString>
#include <cstdlib>
#include <ctime>
#include <QDebug>

enum STATUS{
    BACKUP, MACRORUN, READY, RUNNING, SUSPEND, TERMINATED
};

class SPCB
{
private:
    QString PID;
    QString pname;
    STATUS status;
    int priority;
    int runtime;
    SPCB *next;
    int memoryNeed;
    int memoryBegin;

    int colorOrder;//展示效果：不同颜色

public:
    SPCB(QString pname,int priority,int runtime,int memoryNeed);
    void setRuntime(int value);
    void delRuntime(int value);//减去传入的时间值
    void setPname(const QString &value);
    void setStatus(STATUS value);
    void setPID(int value);
    void setPriority(int value);
    void delPriority(int value);//减去传入的优先级值
    void addPriority(int value);//加上传入的优先级值
    void setNext(SPCB *value);
    void setMemoryNeed(int value);
    void setMemoryBegin(int value);

    QString getPID() const;
    int getRuntime() const;
    QString getPname() const;
    STATUS getStatus() const;
    int getPriority() const;
    int getMemoryNeed() const;
    int getMemoryBegin() const;
    int getColorOrder() const;
    void setColorOrder(int value);

    void refreshColor();//刷新本进程代表颜色为默认
};

#endif // SPCB_H
