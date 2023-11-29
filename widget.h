
#ifndef WIDGET_H
#define WIDGET_H
#define NUM_OF_PROCESS 6
#define TIME_SLICE 10
#define MEMORYSIZE 32
#define OSMEMORY 6
#define NOTPARTIED 0
#define PARTIED 1

#include <QWidget>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QPushButton>
#include <QTimer>
#include <QMovie>
#include "sprocess.h"
#include "backupque.h"
#include "addworks.h"
#include "readyque.h"
#include "selectsuspend.h"
#include "passrate.h"
#include "readyinfodialog.h"

struct UnpartitionTable//未分分区表的单元项
{
    UnpartitionTable(int beginValue, int widthValue, int statusValue){
        beginAddr = beginValue+1;
        width = widthValue;
        tableStatus = statusValue;//0-->empty
    }

    int beginAddr;
    int width;
    int tableStatus;
};

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    void initTimers();//初始化驱动器
    void initSustasks();//初始化挂起队列
    void initPassrate();//初始化内存占用百分比表盘
    void stopTimers();//暂停驱动器
    void addProcess();//向ready队列中添加后备队列中的第一个进程
    void addIntoReady(SProcess *procs);//将将信息做成目标指针加入ready队列数组
    void addIntoReadyWithSlice(SProcess *procs);
    void startAddProcsInit();//初始化作业调度，选择6个进程
    int addMultiTasks(QSqlQuery &result);//将查询结果数据填入进程数组

    SProcess *emptyProcs();//返回当前对象数组中可用（可填数据）的空进程
    QSqlDatabase getDB();//获取数据库连接

    int countBackQue();//统计后备队列中的进程数目
    void delFirstOfDB();//删除数据库第一条记录
    void initMoveBtn_1();//初始化界面上的移动按钮
    void upBtn_Move_1();//上部分移动函数
    void downBtn_Move_1();//下部分移动函数
    void downBtn_Move_1_withslice();
    void dataintoMovebtn();//将ready队列中优先权级最高的选出进入running状态
    void stopinCPU();//进程停留在CPU区域内的处理函数
    void delMoveProcsTime();//对停留在CPU区域内的进程执行定时减剩余时间操作
    void CPUscheduling();//整个CPU调度循环开始的入口

    void changeUpXY();
    void changeDownXY();
    void changeDownXYWithSlice();
    void cpuReceivedtoWait();//当cpu进入等待模式，调用stopinCPU()函数减时间
    void cpuIfKeepWait();//cpu等待监测函数
    void cpuIfKeepWaitWithSlice();
    void workScheduling();//作业调度监测函数

    void addSuspendQue(SProcess *select);//将目标指针加入挂起队列数组
    void delSuspendQue(SProcess *del);//删除挂起队列中的指定指针，并刷新挂起队列表格
    void refreshSuspendQue();//刷新挂起队列列表
    SProcess *searchProcsInSuspdQue(QString PID, int pri, int tim);//返回在挂起队列中查找的结果指针
    void crushRunningProcs();//抢占监听

    void initMemoryTable();//初始化内存展示表
    void initMemoryUsage();//初始化代表内存的数组
    void initUnpartiTable();//初始化未分分区表
    void refreshUnpartiTable();//刷新未分分区表展示表格
    void refreshMemoryTable();//根据进程PCB信息刷新内存展示表格
    void unpartitionTableSort();//对未分分区表的各项按照起址进行小到大的排序
    static bool cmpOfUnptTableSort(const UnpartitionTable *info1, const UnpartitionTable *info2);
    int sumOfUnptTableSpace();//返回所有剩余空间之和
    bool isEveryEnough(int procsWidth);//检测函数，是否有未分分区足够
    bool isAllEnough(int procsWidth);//检测函数，未分分区总和是否足够
    int addrAssignment(SProcess *targetProcs);//内存分配，分配成功返回1，否则返回0
    int exitMemoryRelease(SProcess *leaveProcs);//进程出内存后的内存释放，修改未分分区表内容和内存代表数组的内容，刷新未分分区表展示表格
    void exitMemoryMerge();//合并函数，在进程出主存的时候进行调用，合并内存碎片
    void refreshProcsColors();//更新所有进程（不包括即将要进入的进程）的颜色状态
    void upDatePassrate();//更新内存盘的展示效果
    void getCurMemoryPercent();//更新当前的内存占比
    void memoryCompaction();//内存紧缩
    void initProcsInMemory();//更新inmemory表（在内存中的进程的指针的集合）
    void memoryProcsUsedSort();//将在内存中的进程按照地址排序
    static bool cmpOfProcsUsedSort(const SProcess *info1, const SProcess *info2);
    void initWaiting();//初始化加载gif动图
    void unpartitionTableSortBySize();//对未分分区表的各项按照空间大小(width)进行小到大的排序
    static bool cmpOfUnptTableSortBySize(const UnpartitionTable *info1, const UnpartitionTable *info2);
    int getIndexByFirst(int procsWidth);//按照最先适应的算法返回的合适下标，找不到返回-1
    int getIndexByBest(int procsWidth);//按照最优适应的算法返回的合适下标，找不到返回-1
    int getIndexByWorst(int procsWidth);//按照最差适应的算法返回的合适下标，找不到返回-1

private slots:
    void on_AddWork_clicked();

    void on_ViewBackQue_clicked();

    void timerEvent(QTimerEvent *eve) Q_DECL_OVERRIDE;

    void on_Start_clicked();

    void on_Terminate_clicked();

    void on_Random_clicked();

    void on_continue_2_clicked();

    void on_Suspend_clicked();

    void addtoSuspendQue(SProcess *receive);

    void on_DeSuspend_clicked();

    void on_crush_stateChanged(int arg1);

    void on_slice_stateChanged(int arg1);

    void on_addrassignpatterns_currentIndexChanged(int index);

    void on_btn_move_1_clicked();

private:
    Ui::Widget *ui;
    BackUpQue *backque;
    ReadyQue *readyque;
    SelectSuspend *selesuspd;//展示可以挂起的进程
    SProcess *moveProcs;//存储移动的btn所代表的进程对象的信息
    Passrate *passrate;//内存占用圆环形图
    int timerMonitNum;//作业调度控制
    int timerstartall;//CPU调度循环控制
    int timerup;//上部分动画控制
    int timerdown;//下部分动画控制（退出）
    int timercpuwait;//CPU处理等待控制
    int timercpudeltime;//CPU时间减少控制
    int timerdownwithslice;//下部分动画控制（返回ready队，属于时间片部分）
    int timercrush;//抢占监听
    int timerpassrate;//内存占比盘控制
    //各个驱动器

    SProcess multitasks[NUM_OF_PROCESS];//内存中的总进程
    SProcess *sustasks[NUM_OF_PROCESS];//挂起队列
    int num_in_memory;//内存中的总进程数
    int num_in_suspend;//被挂起的总进程数
    int curRunTime;//记录运行时间：时间片
    int addrAssign;
    bool isSlice;
    bool canPopReady;//ready队列是否可以弹出一个优先级最高的进程
    bool canUpMove;//上半移动是否可以开始
    bool cpuWait;//cpu处理等待是否开始
    bool canDownMove;//下半移动是否可以开始
    bool canDownMoveWithSlice;
    bool ifcpuWaitover;//cpu等待是否完毕
    bool ifCPUschedule;//是否可以进行一次cpu调度循环
    bool ifCPUscheduleIsRunning;//cpu调度循环是否正在进行

    int memoryUsage[MEMORYSIZE];//代表内存的数组，32长度

    QList<UnpartitionTable*> unpartitionTable;//整个未分分区表

    int curmemorypercent;//当前的内存占用的百分比
    bool isCrush;//是否处于抢占模式
    QList<SProcess *> inmemory;//内存中所有进程的集合
};

#endif // WIDGET_H
