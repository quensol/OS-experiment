
#ifndef READYQUE_H
#define READYQUE_H
#define NUM_OF_PROCESS 6

#include <QWidget>
#include <QPushButton>
#include <QDebug>
#include <algorithm>
#include <QDialog>
#include "sprocess.h"
#include "readyinfodialog.h"

namespace Ui {
class ReadyQue;
}

class ReadyQue : public QWidget
{
    Q_OBJECT

public:
    explicit ReadyQue(QWidget *parent = 0);
    ~ReadyQue();
    void InitReady();//初始化ready队列中的进程
    void ReceiveReadyProcs(SProcess *receiveProcs);//接收目标进程指针（加入ready队列）
    void ReceiveReadyProcsWithSlice(SProcess *receiveProcs);
    SProcess * PopReadyProcs();//弹出优先级最高的进程指针
    SProcess * PopReadyProcsWithSlice();
    void SortReadyProcs();//按照优先级进行排序
    static bool cmp(SProcess *a, SProcess *b);
    void delAllBtn();//删除所有展示按钮
    void layAllbtns();//排出所有剩余按钮
    void refreshAllbtns();//刷新界面按钮
    void refreshAllbtnsWithSlice();
    void connectBtnProcs();//建立指针存储数组和按钮数组一一对应的关系
    void initBtns();//初始化按钮，建立槽函数连接
    void BtnStyle(QPushButton *btn);//设定按钮样式

    int getMax_Procs() const;
    SProcess **getReadyQue();
    SProcess *getMaxPrioProcs();
    void addPriOfAllWait();//所有等待中的ready队列进程优先级+1
    void delSpecifiedProcsInReady(SProcess *specified);//删除指定的指针specified
    void delSpecifiedProcsInReadyWithSlice(SProcess *specified);

private:
    Ui::ReadyQue *ui;
    int Max_Procs;//ready队列中的进程数目
    QPushButton *btns[NUM_OF_PROCESS];//所有展示按钮的集合
    SProcess *ready[NUM_OF_PROCESS];//ready队列存储容器
    ReadyInfoDialog *infordialog;//展示进程信息

private slots://展示进程信息接口
    void btn_0_clicked();
    void btn_1_clicked();
    void btn_2_clicked();
    void btn_3_clicked();
    void btn_4_clicked();
    void btn_5_clicked();
};

#endif // READYQUE_H
