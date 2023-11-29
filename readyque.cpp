#include "readyque.h"
#include "ui_readyque.h"
using namespace std;

ReadyQue::ReadyQue(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReadyQue)
{
    ui->setupUi(this);
    Max_Procs = 0;
    InitReady();
}

ReadyQue::~ReadyQue()
{
    delete ui;
}

/*
 * 初始化ready队列中的进程（初始化ready进程数组、按钮数组）
 */
void ReadyQue::InitReady()
{
    for(int i=0;i<NUM_OF_PROCESS;i++)
    {
        ready[i] = nullptr;
        btns[i] = new QPushButton(this);
        btns[i]->setVisible(false);
    }

    infordialog = new ReadyInfoDialog;
    //初始化所有按钮，连接槽函数
    initBtns();

}

/**
 * 有新的作业需要进入内存（进ready队列），ready队列接收指针
 * @brief ReadyQue::ReceiveReadyProcs
 * @param receiveProcs
 */
void ReadyQue::ReceiveReadyProcs(SProcess *receiveProcs)
{
    ready[Max_Procs] = receiveProcs;
    Max_Procs++;

    //刷新界面按钮
    refreshAllbtns();

}

void ReadyQue::ReceiveReadyProcsWithSlice(SProcess *receiveProcs)
{
    ready[Max_Procs] = receiveProcs;
    Max_Procs++;

    //刷新界面按钮
    refreshAllbtnsWithSlice();
}

/**
 * 有优先级第一的进程需要进行处理，外部函数接收数据后用本函数释放空间
 * @brief ReadyQue::PopReadyProcs
 */
SProcess* ReadyQue::PopReadyProcs()
{
    if(Max_Procs>0){
        SProcess *rs = ready[0];
        //由于所属对象都不是new出来的（主界面对象数组），故不能delete
        for(int i=0;i<Max_Procs;i++){//覆盖ready[0]
            ready[i] = ready[i+1];
        }
        Max_Procs--;
        refreshAllbtns();
        return rs;
    }
    else {
        return nullptr;
    }
}

SProcess *ReadyQue::PopReadyProcsWithSlice()
{
    if(Max_Procs>0){
        SProcess *rs = ready[0];
        //由于所属对象都不是new出来的（主界面对象数组），故不能delete
        for(int i=0;i<Max_Procs;i++){
            ready[i] = ready[i+1];
        }
        Max_Procs--;
        refreshAllbtnsWithSlice();
        return rs;
    }
    else {
        return nullptr;
    }
}

void ReadyQue::SortReadyProcs()
{
    /* 稳定排序 */
    std::stable_sort(ready, ready + Max_Procs, cmp);
}

/* 排序规则：优先权级 */
bool ReadyQue::cmp(SProcess *a, SProcess *b)
{
    return ( (a->getPCB()->getPriority()) > (b->getPCB()->getPriority()) );//大--》小
}

/* 隐藏所有按钮 */
void ReadyQue::delAllBtn()
{

        for(int i=0;i<NUM_OF_PROCESS;i++){
            btns[i]->setVisible(false);
        }
    return;
}

/* 将所有剩余按钮排出来 */
void ReadyQue::layAllbtns()
{
    if(Max_Procs>0){
        for(int i=0;i<Max_Procs;i++){
            if(btns[i]!=nullptr){
                btns[i]->setVisible(true);
            }
        }
    }else {
        qDebug()<<"lay none";
    }
    return;
}

/* 刷新界面按钮 */
void ReadyQue::refreshAllbtns()
{
    /* 重新排序指针数组（按优先权级） */
    SortReadyProcs();

    delAllBtn();

    /* 建立新的对应关系 */
    connectBtnProcs();

    layAllbtns();
}

void ReadyQue::refreshAllbtnsWithSlice()
{
    delAllBtn();

    /* 建立新的对应关系 */
    connectBtnProcs();

    layAllbtns();
}

/*
 * 建立指针存储数组和按钮数组一一对应的关系
 * 其实不用建立就可一一对应，这里只是把button的text对应一下
 */
void ReadyQue::connectBtnProcs()
{
    if(Max_Procs>0){
        for(int i=0;i<Max_Procs;i++){
            if(ready[i]!=nullptr){
                QString pri = QString::number(ready[i]->getPCB()->getPriority());
                QString tim = QString::number(ready[i]->getPCB()->getRuntime());
                btns[i]->setText("PID:"+ready[i]->getPCB()->getPID()+" 优先权级:"+pri+
                                 " 时间:"+tim);
            }
        }
    }else {
        qDebug()<<"connect none";
    }
    return;
}

//初始化按钮，建立槽函数连接
void ReadyQue::initBtns()
{
    connect(btns[0], SIGNAL(clicked(bool)), this, SLOT(btn_0_clicked()));
    connect(btns[1], SIGNAL(clicked(bool)), this, SLOT(btn_1_clicked()));
    connect(btns[2], SIGNAL(clicked(bool)), this, SLOT(btn_2_clicked()));
    connect(btns[3], SIGNAL(clicked(bool)), this, SLOT(btn_3_clicked()));
    connect(btns[4], SIGNAL(clicked(bool)), this, SLOT(btn_4_clicked()));
    connect(btns[5], SIGNAL(clicked(bool)), this, SLOT(btn_5_clicked()));
    for(int i=0;i<6;i++){
        BtnStyle(btns[i]);
        ui->verticalLayout->addWidget(btns[i]);
    }
    ui->verticalLayout->addStretch();
}

/**
 * @brief ReadyQue::BtnStyle
 * @param btn
 * 设定按钮样式
 */
void ReadyQue::BtnStyle(QPushButton *btn)
{
    btn->setStyleSheet("QPushButton{ height:50px;"
                       "width: 261px;"
                       "border-width:4px;"
                       "border-style:outset;"
                       "border-radius:10px;"
                       "border-color:rgba(255,255,255,30);"
                       "background-color: rgba(170, 255, 255, 150);"
                       "border-radius:3px;"
                       "font: 9pt '微软雅黑'; }"
                       "QPushButton:hover{"
                       "background-color: rgba(197, 161, 255, 200);"
                       "border-color:rgba(255,255,255,200);"
                       "}");
}

int ReadyQue::getMax_Procs() const
{
    return Max_Procs;
}

SProcess **ReadyQue::getReadyQue()
{
    return ready;
}

SProcess *ReadyQue::getMaxPrioProcs()
{
    return ready[0];
}

void ReadyQue::addPriOfAllWait()
{
    for(int i=0;i<Max_Procs;i++){
        ready[i]->getPCB()->addPriority(1);
    }
    connectBtnProcs();
}

/**
 * @brief ReadyQue::delSpecifiedProcsInReady
 * @param specified
 * 删除指定的指针specified
 */
void ReadyQue::delSpecifiedProcsInReady(SProcess *specified)
{
    for(int i=0;i<Max_Procs;i++){
        if(ready[i]->getPCB()->getPID()==specified->getPCB()->getPID()){//找到指定指针
//            ready[i] = nullptr;
            for(int j=i;j<Max_Procs;j++)
            {
                ready[j] = ready[j+1];//删除指针并迭代填补空位
            }
            Max_Procs--;
            refreshAllbtns();
            return;
        }
    }
}

void ReadyQue::delSpecifiedProcsInReadyWithSlice(SProcess *specified)
{
    for(int i=0;i<Max_Procs;i++){
        if(ready[i]->getPCB()->getPID()==specified->getPCB()->getPID()){//找到指定指针
//            ready[i] = nullptr;
            for(int j=i;j<Max_Procs;j++)
            {
                ready[j] = ready[j+1];//删除指针并迭代填补空位
            }
            Max_Procs--;
            refreshAllbtnsWithSlice();
            return;
        }
    }
}

/**
 * @brief ReadyQue::btn_i_clicked  i:0~5
 * 六个按钮固定槽函数：
 * 点击后显示ready[i]->PCB相关详细信息（主要来自进程SProcess对象的PCB信息）
 */

void ReadyQue::btn_0_clicked()
{
    qDebug()<<"connect none";
    if(ready[0]!=nullptr){
        infordialog->setAll(ready[0]);
        infordialog->show();
    }
}

void ReadyQue::btn_1_clicked()
{
    if(ready[1]!=nullptr){
        infordialog->setAll(ready[1]);
        infordialog->show();
    }
}

void ReadyQue::btn_2_clicked()
{
    if(ready[2]!=nullptr){
        infordialog->setAll(ready[2]);
        infordialog->show();
    }
}

void ReadyQue::btn_3_clicked()
{
    if(ready[3]!=nullptr){
        infordialog->setAll(ready[3]);
        infordialog->show();
    }
}

void ReadyQue::btn_4_clicked()
{
    if(ready[4]!=nullptr){
        infordialog->setAll(ready[4]);
        infordialog->show();
    }
}

void ReadyQue::btn_5_clicked()
{
    if(ready[5]!=nullptr){
        infordialog->setAll(ready[5]);
        infordialog->show();
    }
}


