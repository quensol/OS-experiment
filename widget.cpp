#include "widget.h"
#include "ui_widget.h"
#define random(a,b) (rand() % (b-a+1))+a

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    num_in_memory = 0;
    curRunTime = 0;
    num_in_suspend = 0;
    isSlice = false;//初始默认纯优先级
    ui->status_2->setVisible(false);
    ui->compaction->setVisible(false);
    ui->waiting->setVisible(false);
    moveProcs = nullptr;
    canPopReady = false;//ready队列不能pop->可能是：正在upmove或者队列为空，前期默认不能
    canUpMove = false;
    canDownMove = false;
    canDownMoveWithSlice = false;
    cpuWait = false;//cpu处于未进入等待状态
    ifcpuWaitover = true;//一开始cpu处于等待完毕状态，cpu对move的进程的时间--不会发生
    ifCPUschedule = false;//CPU调度尚未开始
    ifCPUscheduleIsRunning = false;//cpu调度未正在进行
    srand((int)time(NULL));//PID随机数种子init

    curmemorypercent = 0;//初始内存占比为0
    isCrush = false;//初始默认为非抢占状态
    addrAssign = 0;//初始默认为最先适应算法

    initMoveBtn_1();//初始化移动的按钮
    initSustasks();//初始化挂起数组
    initMemoryTable();//初始化内存展示数组
    initUnpartiTable();//初始化未分分区表
    initPassrate();//初始化内存分析图
    initWaiting();


    backque = new BackUpQue;
    readyque = new ReadyQue(this);
    readyque->raise();
    readyque->show();
    selesuspd = new SelectSuspend;
    selesuspd->hide();
    startAddProcsInit();

    connect(selesuspd, SIGNAL(selectok(SProcess*)), this, SLOT(addtoSuspendQue(SProcess*)) );
    //选中挂起的进程连接槽函数
}

Widget::~Widget()
{
    delete ui;
    if(backque!=nullptr)  delete backque;
    if(readyque!=nullptr) delete readyque;
    if(selesuspd!=nullptr) delete selesuspd;
    if(passrate!=nullptr) delete passrate;
}

/**
 * @brief Widget::initTimers
 * 初始化（开始）计时器
 * 纯优先级、纯时间片均可用
 */
void Widget::initTimers()
{
    timerstartall = startTimer(5);
    timerup = startTimer(3);
    timerdown = startTimer(3);
    timercpuwait = startTimer(5);
    timercpudeltime = startTimer(500);
    timerMonitNum = startTimer(1500);
    timerdownwithslice = startTimer(3);
}

/**
 * @brief Widget::initSustasks
 * 初始化挂起队列
 * 纯优先级、纯时间片均可用
 */
void Widget::initSustasks()
{
    ui->suspendque->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for(int i=0;i<NUM_OF_PROCESS;i++)
        sustasks[i] = nullptr;
}

/**
 * @brief Widget::initPassrate
 * 初始化内存占用显示图
 */
void Widget::initPassrate()
{
    passrate = new Passrate(this);
    passrate->setGeometry(820,440,280,280);
    passrate->updateValue(OSMEMORY);
    passrate->show();
}

/**
 * @brief Widget::stopTimers
 * 终止计时器
 * 纯优先级、纯时间片均可用
 */
void Widget::stopTimers()
{
    this->killTimer(timerstartall);
    this->killTimer(timerup);
    this->killTimer(timerdown);
    this->killTimer(timercpuwait);
    this->killTimer(timercpudeltime);
    this->killTimer(timerMonitNum);
    this->killTimer(timerdownwithslice);
}

/**
 * 选取后备队列第一个往内存新加一个进程（然后进ready队列）:
 * 1、进主界面的multitasks[]数组
 * 2、进ready队列
 * 每选一个，填入一个SProcess数组元素（改变状态值），删除头一条数据库记录
 * 纯优先级、纯时间片均可用
 * @brief Widget::addProcess
 */
void Widget::addProcess()
{
    if(num_in_memory < NUM_OF_PROCESS && num_in_memory>=0)//原先的条件：内存中的进程数目不满6个即可
    {
        QSqlDatabase db = getDB();
        bool ok = db.open();
        if(ok){
            QSqlQuery result = db.exec(QString("SELECT pname,priority,runtime,memory"
                                               " FROM backupque ORDER BY id ASC LIMIT 1"));//查出第一条记录
            if (result.next()) {
                if(addMultiTasks(result)==1){//内存分配也成功了，新加进主界面的multitasks[]数组和ready队列
                   num_in_memory += 1;
                   delFirstOfDB();//删除头一条记录
                   backque->refreshAgain();
                   refreshMemoryTable();//刷新内存占用的展示表格
                   getCurMemoryPercent();
//                   upDatePassrate();//刷新圆盘
                }
                else{//内存分配失败
                    db.close();
                    return;
                }
            }
            db.close();//内存分配成功后关闭
        }
        else {
           qDebug()<< "Database open failed in addProcess() ";
        }
    }
    else {
        qDebug()<< "number of processes is limited in addProcess()";
        return;
    }
}

/**
 * @brief Widget::addIntoReady
 * 将信息做成指针存进ready(自动刷新ready展示的按钮)
 * 纯优先级均可用
 */
void Widget::addIntoReady(SProcess *procs)
{
    readyque->ReceiveReadyProcs(procs);
}

/**
 * @brief Widget::addIntoReady
 * 将信息做成指针存进ready(自动刷新ready展示的按钮)
 * 纯时间片均可用
 */
void Widget::addIntoReadyWithSlice(SProcess *procs)
{
    readyque->ReceiveReadyProcsWithSlice(procs);
}

/**
 * 开始初始化：
 * 1、如果后备队列中个数>=6，则从后备队列中按照id的顺序（FCFS）选取6个进内存（ready）
 * 2、如果后备队列中个数<6，则将后备队列中所有进内存（ready）
 * for循环每选一个，填入一个SProcess数组元素（改变状态值），删除一条记录
 * 纯优先级、纯时间片均可用
 * @brief Widget::startAddProcsInit
 */
void Widget::startAddProcsInit()
{
    if(num_in_memory == 0)
    {
        QSqlDatabase db = getDB();
        bool ok = db.open();
        if(ok){
            if(countBackQue()>=6)
            {
                //选最前面的6个（按照FCFS）
                for(int i=0;i<NUM_OF_PROCESS;i++)
                {
                    addProcess();
                }
            }
            else {
                int num = countBackQue();
                for(int i=0;i<num;i++)
                {
                    qDebug()<<countBackQue();
                    addProcess();
                }
            }
            getCurMemoryPercent();
            upDatePassrate();
        }
        else {
           qDebug()<< "Database open failed in startAddProcsInit()";
           db.close();
        }
    }
    else {
        qDebug()<< "number of processes is not 0 in startAddProcsInit()33";
        return;
    }
}

/**
 * @brief Widget::addMultiTasks
 * @param result
 * @return 0/1
 * 将信息填入一个multitasks数组元素（进程）并分配内存，成功返回1，否则返回0
 * 纯优先级、纯时间片均可用
 */
int Widget::addMultiTasks(QSqlQuery &result)
{
    SProcess *p = emptyProcs();//数据暂存
    p->setPname(result.value("pname").toString());
//    p->setPstatus(READY);//因为不一定可以进内存（进ready），所以先不急着设置状态值
    p->setPriority(result.value("priority").toInt());
    p->setRunTime(result.value("runtime").toInt());
    p->setMemoryNeed(result.value("memory").toInt());
    p->setDefaultPCB();//设置pname、status、priority、runtime

    if(addrAssignment(p)==1){//内存分配成功
        p->getPCB()->setStatus(READY);
        p->getPCB()->setPID(random(1000,9999));

        addIntoReady(p);//新加进ready队列
        return 1;
    }
    else{//内存分配失败
        return 0;
    }
}

/**
 * @brief Widget::emptyProcs
 * @return 返回可用的进程对象数组空间
 * 纯优先级、纯时间片均可用
 */
SProcess *Widget::emptyProcs()
{
    if(num_in_memory<NUM_OF_PROCESS){
        for(int i=0;i<NUM_OF_PROCESS;i++){
            if(multitasks[i].getPCB()->getStatus() == BACKUP||
                    multitasks[i].getPCB()->getStatus() == TERMINATED)
            {
                return &multitasks[i];
            }
        }
    }
}

/**
 * @brief Widget::getDB
 * @return 返回一个既定的QSqlDatabase对象简化代码
 * 纯优先级、纯时间片均可用
 */
QSqlDatabase Widget::getDB()
{
    QSqlDatabase db;
    if(QSqlDatabase::contains("mysql"))
    {
        db = QSqlDatabase::database("mysql");
    }
    else {
        db = QSqlDatabase::addDatabase("QMYSQL","mysql");
    }
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("mysql");
    db.setUserName("root");
    db.setPassword("123456");
    return db;
}

/**
 * @brief Widget::countBackQue
 * @return 统计后备队列中的个数
 * 纯优先级、纯时间片均可用
 */
int Widget::countBackQue()
{
    QSqlDatabase db = getDB();
    bool ok = db.open();
    int count;
    if(ok){
        QSqlQuery result = db.exec(QString("SELECT COUNT(*) FROM backupque"));
        while (result.next()) {
            count =  result.value("COUNT(*)").toInt();
            db.close();
            return count;
        }
    }
    else {
       qDebug()<< "Database open failed in countBackQue()";
       return 0;
    }
}

/**
 * 删除按照id（FCFS）顺序的第一条数据库记录
 * 纯优先级、纯时间片均可用
*/
void Widget::delFirstOfDB()
{
    QSqlDatabase db = getDB();
    bool ok = db.open();
    if(ok){
        QSqlQuery q(db);
        QSqlQuery result = db.exec(QString("SELECT id FROM backupque ORDER BY id ASC LIMIT 1"));
        while (result.next()) {
            QString del = QString("DELETE FROM backupque WHERE id=%1").arg(result.value("id").toString());
            bool scss = q.exec(del);
            if(scss)
                qDebug()<< "delete first success";
        }
    }
    else {
       qDebug()<< "Database open failed in delFirstOfDB()";
    }
    db.close();
    backque->initQueue();
}

/**
 * @brief Widget::initMoveBtn_1
 * 初始化移动按钮的位置和状态
 * 纯优先级、纯时间片均可用
 */
void Widget::initMoveBtn_1()
{
    ui->btn_move_1->setVisible(false);
    ui->btn_move_1->setText("");
    ui->btn_move_1->move(20,70);
}

/**
 * @brief Widget::upBtn_Move
 * 上半部分动画函数，从readyque后面把预先做好的button移出至CPU进行running
 * 纯优先级、纯时间片均可用
 */
void Widget::upBtn_Move_1()
{
    if(canUpMove){

        ui->btn_move_1->setVisible(true);
        changeUpXY();
    }
//    ui->btn_move_1->setVisible(true);
//    timerup->start(5);
//    return;
//    while(ui->btn_move_1->x()==400&&ui->btn_move_1->y()==220){
//        timerup->stop();
//        return;
//    }
}

/**
 * @brief Widget::downBtn_Move_1
 * 下半部分动画函数，从CPU把固定好的button移出至terminate
 * 纯优先级、纯时间片均可用
 */
void Widget::downBtn_Move_1()
{
    if(canDownMove){
        changeDownXY();
    }
//    timerdown->start(5);
//    return;
//    while(ui->btn_move_1->x()==280&&ui->btn_move_1->y()==380){
//        timerdown->stop();
//        ui->btn_move_1->setVisible(false);
//        return;
    //    }
}

/**
 * @brief Widget::downBtn_Move_1_withslice
 * 纯时间片可用
 */
void Widget::downBtn_Move_1_withslice()
{
    if(canDownMoveWithSlice){
        changeDownXYWithSlice();
    }
}

/**
 * @brief Widget::dataintoMovebtn
 * 将最高优先级Pop出ready队列，并接收其数据
 * 纯优先级专属
 */
void Widget::dataintoMovebtn()
{
    ui->status_2->setVisible(false);
    readyque->SortReadyProcs();//对ready队列排序
    moveProcs = readyque->PopReadyProcs();//从ready取出一个指针:优先级最高
    readyque->refreshAllbtns();//刷新ready的显示队列
    if(moveProcs!=nullptr)
    {
        canUpMove = true;
        ifCPUschedule = false;
        moveProcs->getPCB()->setStatus(MACRORUN);
        QString pri = QString::number(moveProcs->getPCB()->getPriority());
        ui->btn_move_1->setText("PID:"+moveProcs->getPCB()->getPID()+" 优先权:"+pri+" 时间:"+
                                moveProcs->getPCB()->getRuntime() );
        return;
    }
    else {
        qDebug()<< "ready is null";
        ifCPUschedule = false;
        return;
    }
}

/**
 * @brief Widget::stopinCPU
 * 纯优先级、纯时间片均可用
 */
void Widget::stopinCPU()
{
    if(cpuWait&&ifcpuWaitover)
    {
        ui->waiting->setVisible(true);
        ifcpuWaitover = false;
        //开始等待，计时器本来就一直在调用减时间函数，此时这个函数会响应，对该进程减时间
    }
}

/**
 * @brief Widget::delMoveProcsTime
 * 逐秒删除时间的函数，纯优先级可用
 */
void Widget::delMoveProcsTime()
{
    if(!ifcpuWaitover){
        //剩余时间减一
        moveProcs->getPCB()->delRuntime(1);
        moveProcs->setRunTime(moveProcs->getPCB()->getRuntime());
        moveProcs->getPCB()->delPriority(1);
        curRunTime++;//计录器加1

        //还在等待的进程优先权提升1
        readyque->addPriOfAllWait();

        //刷新按钮内容
        QString pri = QString::number(moveProcs->getPCB()->getPriority());
        QString tim = QString::number(moveProcs->getPCB()->getRuntime());
        ui->btn_move_1->setText("PID:"+moveProcs->getPCB()->getPID()+
                                " 优先权:"+pri+" 时间:"+tim);

        if(isSlice)
            cpuIfKeepWaitWithSlice();
        else
            cpuIfKeepWait();//判断是否运行完毕
    }
}

/**
 * @brief Widget::CPUscheduling
 * CPU调度总函数，调用包括：
 * 1、从后备队列选一个进内存：ready =====>addProcess()
 * 2、从readyque选优先级最高的出来给move button =====>dataintoMovebtn()
 * 3、移动move button：上、下     =====>upBtn_Move_1()、downBtn_Move_1()
 * 4、处理完进程后进行判断：出内存
 * 纯优先级、纯时间片均可用
 */
void Widget::CPUscheduling()
{
    if(ifCPUschedule){//开始cpu调度循环
//        startAddProcsInit();
        getCurMemoryPercent();
        upDatePassrate();
        ui->status->setText("调度中");
        ifCPUscheduleIsRunning = true;
        dataintoMovebtn();
    }
    else
        return;
}


/**
 * @brief Widget::on_xxx_clicked
 * 各类槽函数
 */

void Widget::on_AddWork_clicked()
{
    AddWorks *addwork = new AddWorks();
    addwork->show();
}

void Widget::on_ViewBackQue_clicked()
{
    backque->refreshAgain();
    backque->show();
}

void Widget::changeUpXY()
{
    int xnow = ui->btn_move_1->x();
    int ynow = ui->btn_move_1->y();
    if(xnow<400){
        ui->btn_move_1->move(xnow+1, ynow);
        return;
    }
    if(ynow<220){
        ui->btn_move_1->move(xnow, ynow+1);
        return;
    }
    else {
        canUpMove = false;
        cpuWait = true;//停止upmove，开始cpu等待运行时间为0的过程
        moveProcs->getPCB()->setStatus(RUNNING);
        ui->status->setText("处理中");
        ui->compaction->setVisible(false);
    }
}

void Widget::changeDownXY()
{
    int xnow = ui->btn_move_1->x();
    int ynow = ui->btn_move_1->y();
    if(ynow<460){
        ui->btn_move_1->move(xnow, ynow+1);
        return;
    }
    if(xnow>-270){
        ui->btn_move_1->move(xnow-1, ynow);
        return;
    }
    else {
        canDownMove = false;
        ifCPUschedule = true;//开启下一cpu调度循环
        ifCPUscheduleIsRunning = false;//cpu调度循环结束
        initMoveBtn_1();

    }
}

void Widget::changeDownXYWithSlice()
{
    int xnow = ui->btn_move_1->x();
    int ynow = ui->btn_move_1->y();
    if(ynow<380){
        ui->btn_move_1->move(xnow, ynow+1);
        return;
    }
    if(xnow>20){
        ui->btn_move_1->move(xnow-1, ynow);
        return;
    }
    else {
        canDownMoveWithSlice = false;
        ifCPUschedule = true;//开启下一cpu调度循环
        ifCPUscheduleIsRunning = false;//cpu调度循环结束
        addIntoReadyWithSlice(moveProcs);//进程进入ready队列
        initMoveBtn_1();
    }
}

/**
 * @brief Widget::cpuReceivedtoWait
 * 当cpu进入等待模式，调用stopinCPU()函数减时间
 */
void Widget::cpuReceivedtoWait()
{
    if(!cpuWait&&!canDownMove)//还在up移动
        return;
    else if(cpuWait&&!canDownMove)//up移动完毕，开始cpu等待
    {
        stopinCPU();
    }
    else
    {
        return;
    }
}

/**
 * @brief Widget::cpuIfKeepWait
 * cpu等待监测函数--不含时间片，纯优先权
 */
void Widget::cpuIfKeepWait()
{
    if(ifcpuWaitover)
        return;
    else {
        if(moveProcs->getPCB()->getRuntime()<=0){//运行完毕
            moveProcs->setPstatus(TERMINATED);
            moveProcs->getPCB()->setStatus(TERMINATED);//进程终止
            exitMemoryRelease(moveProcs);
            exitMemoryMerge();
            getCurMemoryPercent();
            ui->waiting->setVisible(false);
            ifcpuWaitover = true;//结束减时间
            cpuWait = false;//结束stopinCPU()的调用
            canDownMove = true;//开始下半移动
            moveProcs = nullptr;
            num_in_memory--;
        }
    }
}

/**
 * @brief Widget::cpuIfKeepWaitWithSlice
 * cpu等待监测函数--纯时间片（也可附带优先级，因为这不是优先级的代表函数）
 */
void Widget::cpuIfKeepWaitWithSlice()
{
    if(ifcpuWaitover)
        return;
    else {
        if(curRunTime>=TIME_SLICE&&moveProcs->getPCB()->getRuntime()>0)//时间片限制时间到达
        {
            moveProcs->setPstatus(READY);
            moveProcs->getPCB()->setStatus(READY);
            curRunTime = 0;//清空运行时间记录器
            getCurMemoryPercent();
            canDownMoveWithSlice = true;
            ifcpuWaitover = true;
            cpuWait = false;
            ui->status->setText("调度中");
        }
        if(moveProcs->getPCB()->getRuntime()<=0)//运行完毕
        {
            moveProcs->setPstatus(TERMINATED);
            moveProcs->getPCB()->setStatus(TERMINATED);//进程终止
            exitMemoryRelease(moveProcs);
            exitMemoryMerge();
            getCurMemoryPercent();
            ifcpuWaitover = true;//结束减时间
            cpuWait = false;//结束stopinCPU()的调用
            canDownMove = true;//开始下半移动
            moveProcs = nullptr;
            curRunTime = 0;
            num_in_memory--;
            ui->status->setText("终止中");
        }
    }
}

/**
 * @brief Widget::workScheduling
 * 作业调度监测函数
 */
void Widget::workScheduling()
{
    if(num_in_memory < NUM_OF_PROCESS && countBackQue() > 0){
        addProcess();
    }
    else
        return;
}

/**
 * @brief Widget::addSuspendQue
 * @param select
 * 将目标指针加入挂起队列数组
 */
void Widget::addSuspendQue(SProcess *select)
{
    sustasks[num_in_suspend] = select;
    num_in_suspend++;
}

/**
 * @brief Widget::delSuspendQue
 * @param del
 * 删除挂起队列中的指定指针，并刷新挂起队列表格
 */
void Widget::delSuspendQue(SProcess *del)
{
    for(int i=0;i<num_in_suspend;i++){
        if(sustasks[i]->getPCB()->getPID()==del->getPCB()->getPID()){//找到指定指针
            for(int j=i;j<num_in_suspend;j++)
            {
                sustasks[j] = sustasks[j+1];//删除指针并迭代填补空位
            }
            num_in_suspend--;
            refreshSuspendQue();
            return;
        }
    }
}

/**
 * @brief Widget::refreshSuspendQue
 * 刷新挂起队列列表
 */
void Widget::refreshSuspendQue()
{
    ui->suspendque->clearContents();
    ui->suspendque->setRowCount(0);
    for(int i=0;i<num_in_suspend;i++)
    {
        int RowCont = ui->suspendque->rowCount();
        ui->suspendque->insertRow(RowCont);//增加一行
        ui->suspendque->setItem(RowCont,0,new QTableWidgetItem(sustasks[i]->getPCB()->getPID()));
        ui->suspendque->setItem(RowCont,1,new QTableWidgetItem(QString::number(sustasks[i]->getPCB()->getPriority())) );
        ui->suspendque->setItem(RowCont,2,new QTableWidgetItem(QString::number(sustasks[i]->getPCB()->getRuntime())) );
    }
}

/**
 * @brief Widget::searchProcsInSuspdQue
 * @param PID
 * @param pri
 * @param tim
 * @return 返回在挂起队列中查找的结果指针
 */
SProcess *Widget::searchProcsInSuspdQue(QString PID, int pri, int tim)
{
    for(int i=0;i<num_in_suspend;i++){
        if(sustasks[i]->getPCB()->getPID()==PID && sustasks[i]->getPCB()->getPriority()==pri
                &&sustasks[i]->getPCB()->getRuntime()==tim)
        {
            return sustasks[i];
        }
    }
    qDebug()<<"not find in searchProcsInSuspdQue of widget";
    return nullptr;
}

/**
 * @brief Widget::crushRunningProcs
 * 抢占监听
 */
void Widget::crushRunningProcs()
{
    if(readyque->getMax_Procs()>0 && moveProcs!=nullptr && !ifcpuWaitover){
        if(readyque->getMaxPrioProcs()->getPCB()->getPriority() > moveProcs->getPCB()->getPriority()){
            moveProcs->setPstatus(READY);
            moveProcs->getPCB()->setStatus(READY);
            curRunTime = 0;//清空运行时间记录器
            canDownMoveWithSlice = true;
            ifcpuWaitover = true;
            cpuWait = false;
            ui->status->setText("调度中");
            ui->status_2->setVisible(true);
//            qDebug()<<readyque->getMaxPrioProcs()->getPCB()->getPriority();
        }
        else {
            return;
        }
    }
    else {
        return;
    }
}

/**
 * @brief Widget::initMemoryTable
 * 初始化内存展示表
 */
void Widget::initMemoryTable()
{
    ui->memorysituationview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->memorysituationview->setRowCount(MEMORYSIZE);
    for(int i=0;i<OSMEMORY;i++){
        if(i==0)
            ui->memorysituationview->setItem(i,0,new QTableWidgetItem(QString("OS")));
        ui->memorysituationview->setItem(i,1,new QTableWidgetItem(QString::number(i)));
        ui->memorysituationview->setItem(i,2,new QTableWidgetItem(QString("占用")));
        ui->memorysituationview->item(i,1)->setBackgroundColor(QColor("#A0FEFB"));
        ui->memorysituationview->item(i,2)->setBackgroundColor(QColor("#A0FEFB"));
    }
}

/**
 * @brief Widget::initMemoryUsage
 * 初始化代表内存的数组
 */
void Widget::initMemoryUsage()
{
    for(int i=0;i<MEMORYSIZE;i++)
        memoryUsage[i] = 0;

    for(int i=0;i<OSMEMORY;i++)
        memoryUsage[i] = 1;
}

/**
 * @brief Widget::initUnpartiTable
 * 初始化未分分区表
 */
void Widget::initUnpartiTable()
{
    UnpartitionTable *item = new UnpartitionTable(OSMEMORY, MEMORYSIZE-OSMEMORY, NOTPARTIED);
    unpartitionTable.append(item);
    int rowCount = ui->unpartitableview->rowCount();
    ui->unpartitableview->insertRow(rowCount);
    ui->unpartitableview->setItem(rowCount, 0, new QTableWidgetItem(QString::number(item->beginAddr)));
    ui->unpartitableview->setItem(rowCount, 1, new QTableWidgetItem(QString::number(item->width)));
    ui->unpartitableview->setItem(rowCount, 2, new QTableWidgetItem(QString(item->tableStatus==0?"Empty":"Not Empty")));
}

/**
 * @brief Widget::refreshUnpartiTable
 * 刷新未分分区表展示表格
 */
void Widget::refreshUnpartiTable()
{
    ui->unpartitableview->clearContents();
    ui->unpartitableview->setRowCount(0);
    if(unpartitionTable.size()==0)
        return;
    foreach (auto item, unpartitionTable) {
        int rowCount = ui->unpartitableview->rowCount();
        ui->unpartitableview->insertRow(rowCount);
        ui->unpartitableview->setItem(rowCount, 0, new QTableWidgetItem(QString::number(item->beginAddr)));
        ui->unpartitableview->setItem(rowCount, 1, new QTableWidgetItem(QString::number(item->width)));
        ui->unpartitableview->setItem(rowCount, 2, new QTableWidgetItem(QString(item->tableStatus==0?"Empty":"Not Empty")));
    }
}

/**
 * @brief Widget::refreshMemoryTable
 * 根据进程PCB信息刷新内存展示表格
 */
void Widget::refreshMemoryTable()
{
    ui->memorysituationview->clearContents();
    ui->memorysituationview->setRowCount(0);
    initMemoryTable();

    for(int i=0;i<NUM_OF_PROCESS;i++){
        if(multitasks[i].getPCB()->getStatus()!=TERMINATED &&
                multitasks[i].getPCB()->getStatus()!=BACKUP)
        {//将这个进程的信息更新到内存区域展示表格中
            int beginAddr = multitasks[i].getPCB()->getMemoryBegin();
            int width = multitasks[i].getPCB()->getMemoryNeed();
            int colorOrder = multitasks[i].getPCB()->getColorOrder();
            QColor color = QColor(colorOrder==0?"#FBF89A":"#B0EACC");
//            qDebug()<<width;
            /* 处理第一行（因为左边有要附加PID信息） */
            ui->memorysituationview->setItem(beginAddr, 0, new QTableWidgetItem(multitasks[i].getPCB()->getPID()));
            ui->memorysituationview->setItem(beginAddr, 1, new QTableWidgetItem(QString("0")));
            ui->memorysituationview->setItem(beginAddr, 2, new QTableWidgetItem(QString("占用")));
            ui->memorysituationview->item(beginAddr,1)->setBackgroundColor(color);
            ui->memorysituationview->item(beginAddr,2)->setBackgroundColor(color);
            /* 处理第一条之后的所有条 */
            for(int j=beginAddr+1, k=1;j<beginAddr+width;j++,k++){
                ui->memorysituationview->setItem(j, 1, new QTableWidgetItem(QString::number(k)));
                ui->memorysituationview->setItem(j, 2, new QTableWidgetItem(QString("占用")));
                ui->memorysituationview->item(j,1)->setBackgroundColor(QColor(color));
                ui->memorysituationview->item(j,2)->setBackgroundColor(QColor(color));
            }
        }
    }


}

/**
 * @brief Widget::unpartitionTableSort
 * 对未分分区表的各项按照起址进行小到大的排序
 */
void Widget::unpartitionTableSort()
{
    qSort(unpartitionTable.begin(), unpartitionTable.end(), cmpOfUnptTableSort);
}

bool Widget::cmpOfUnptTableSort(const UnpartitionTable *info1, const UnpartitionTable *info2)
{
    return info1->beginAddr < info2->beginAddr;//从小到大，按起址排序
}

/**
 * @brief Widget::sumOfUnptTableSort
 * @return Sum
 * 求和函数，求余下所有碎片的总剩余内存
 */
int Widget::sumOfUnptTableSpace()
{
    int result = 0;
    foreach (auto item, unpartitionTable) {
        result += item->width;
    }
    return result;
}

/**
 * @brief Widget::isEveryEnough
 * @param procsWidth
 * @return 是否有未分分区足够
 */
bool Widget::isEveryEnough(int procsWidth)
{
//    foreach (auto item, unpartitionTable) {
//        if(item->width>=procsWidth){
//            index = unpartitionTable.indexOf(item);
//            return true;
//        }
//    }
    for(int i=0;i<unpartitionTable.size();i++){
        if(unpartitionTable.at(i)->width>=procsWidth){
            return true;
        }
    }
    return false;
}

/**
 * @brief Widget::isAllEnough
 * @param procsWidth
 * @return 未分分区总和是否足够
 */
bool Widget::isAllEnough(int procsWidth)
{
    if(sumOfUnptTableSpace()>=procsWidth)
        return true;
    else
        return false;
}

/**
 * @brief Widget::addrAssignment
 * @param targetProcs
 * @return 0/1
 * 最先适应的内存分配，分配成功返回1，否则返回0
 */
int Widget::addrAssignment(SProcess *targetProcs)
{
    if( isEveryEnough(targetProcs->getPCB()->getMemoryNeed()) ){//有独立的分区可以装的下
//    int size = unpartitionTable.size();
//    for(int i=0;i<size;i++){//for可以去掉了
//        int width = unpartitionTable.at(i)->width;
//        int need = targetProcs->getPCB()->getMemoryNeed();
//        if(width >= need){//最先适应条件成立,存在某分区大于请求的内存，为该进程分配内存起址(初值为-1)
//            /*未分分区表的单元项保存的是显示的表格中的编号，是从1开始的，但是实际计算应该从0开始*/
//            targetProcs->getPCB()->setMemoryBegin(unpartitionTable.at(i)->beginAddr-1);
//            if(width==need){//表示该分区已经被全部分配完毕，可以删除该记录
//                unpartitionTable.removeAt(i);
//            }
//            else {
//                unpartitionTable.at(i)->beginAddr += need;//起址下移
//                unpartitionTable.at(i)->width -= need;
//            }
//            refreshUnpartiTable();//刷新未分分区表的显示表格
//            int begin = targetProcs->getPCB()->getMemoryBegin();
//            for(int j=begin;j<begin+need;j++){//设置代表内存的数组
//                memoryUsage[j] = 1;
//            }
//            return 1;
//        }
//    }
        int index = -1;
        if(addrAssign==0){
            index = getIndexByFirst(targetProcs->getPCB()->getMemoryNeed());//分配下标
            if(index==-1)
                return 0;
            }
        else if(addrAssign==1){
            index = getIndexByBest(targetProcs->getPCB()->getMemoryNeed());
            if(index==-1)
                return 0;
        }
        else if(addrAssign==2){
            index = getIndexByWorst(targetProcs->getPCB()->getMemoryNeed());
            if(index==-1)
                return 0;
        }

        int width = unpartitionTable.at(index)->width;
        int need = targetProcs->getPCB()->getMemoryNeed();
        if(width >= need){//。。。条件成立,存在某分区大于请求的内存，为该进程分配内存起址(初值为-1)
            /*未分分区表的单元项保存的是显示的表格中的编号，是从1开始的，但是实际计算应该从0开始*/
            targetProcs->getPCB()->setMemoryBegin(unpartitionTable.at(index)->beginAddr-1);
            if(width==need){//表示该分区已经被全部分配完毕，可以删除该记录
                UnpartitionTable *t = unpartitionTable.at(index);
                unpartitionTable.removeAt(index);
                delete t;
                t = NULL;
            }
            else {
                unpartitionTable.at(index)->beginAddr += need;//起址下移
                unpartitionTable.at(index)->width -= need;
            }
            refreshUnpartiTable();//刷新未分分区表的显示表格
            int begin = targetProcs->getPCB()->getMemoryBegin();
            for(int j=begin;j<begin+need;j++){//设置代表内存的数组
                memoryUsage[j] = 1;
            }
            return 1;//分配成功
        }
        else
            return 0;
    }
    else if ( isAllEnough(targetProcs->getPCB()->getMemoryNeed()) ) {
        memoryCompaction();
        int width = unpartitionTable.at(0)->width;
        int need = targetProcs->getPCB()->getMemoryNeed();
        targetProcs->getPCB()->setMemoryBegin(unpartitionTable.at(0)->beginAddr-1);
        if(width==need){//表示该分区已经被全部分配完毕，可以删除该记录
            UnpartitionTable *t = unpartitionTable.at(0);
            unpartitionTable.removeAt(0);
            delete t;
            t = NULL;
        }
        else {
            unpartitionTable.at(0)->beginAddr += need;//起址下移
            unpartitionTable.at(0)->width -= need;
        }
        refreshUnpartiTable();//刷新未分分区表的显示表格
        int begin = targetProcs->getPCB()->getMemoryBegin();
        for(int j=begin;j<begin+need;j++){//设置代表内存的数组
            memoryUsage[j] = 1;
        }
        return 1;
    }
    else{//条件不成立视为内存不足
        qDebug()<<"memory space is not enough when addrAssignment()";
        return 0;
    }
}

/**
 * @brief Widget::exitMemoryRelease
 * @param leaveProcs
 * @return 1/0
 * 进程出内存后的内存释放，修改未分分区表内容和内存代表数组的内容，刷新未分分区表展示表格
 */
int Widget::exitMemoryRelease(SProcess *leaveProcs)
{
    int begin = leaveProcs->getPCB()->getMemoryBegin();
    int usage = leaveProcs->getPCB()->getMemoryNeed();
    unpartitionTable.append(new UnpartitionTable(begin, usage, NOTPARTIED));
    for(int i=begin;i<begin+usage;i++){//置空代表内存的相应空间
        memoryUsage[i] = 0;
    }
    refreshUnpartiTable();//刷新未分分区表
}

/**
 * @brief Widget::exitMemoryMerge
 * 合并函数，在进程出主存的时候进行调用，合并内存碎片
 */
void Widget::exitMemoryMerge()
{
    unpartitionTableSort();//按照起址排序
    int size = unpartitionTable.size();
    for(int i=0;i<size-1;i++){
        if(i>=unpartitionTable.size()-1)   break;
        int beginAddr = unpartitionTable.at(i)->beginAddr;
        int width = unpartitionTable.at(i)->width;
        if(unpartitionTable.at(i+1)->beginAddr==(beginAddr+width) ){
            unpartitionTable.at(i)->width += unpartitionTable.at(i+1)->width;
            UnpartitionTable *t = unpartitionTable.at(i+1);
            unpartitionTable.removeAt(i+1);
            delete t;
            t = NULL;
        }
    }
    refreshProcsColors();
    refreshMemoryTable();
    refreshUnpartiTable();
}

/**
 * @brief Widget::refreshProcsColors
 * 更新所有进程（不包括即将要进入的进程）的颜色状态
 */
void Widget::refreshProcsColors()
{
    for(int i=0;i<NUM_OF_PROCESS;i++){
        if(multitasks[i].getPCB()->getStatus()!=TERMINATED &&
                multitasks[i].getPCB()->getStatus()!=BACKUP)
        {
            multitasks[i].getPCB()->refreshColor();
        }
    }
}

/**
 * @brief Widget::upDatePassrate
 * 更新内存盘的展示效果
 */
void Widget::upDatePassrate()
{
    timerpassrate = startTimer(50);
}

/**
 * @brief Widget::getCurMemoryPercent
 * 更新当前的内存占比
 */
void Widget::getCurMemoryPercent()
{
    float sum = MEMORYSIZE - sumOfUnptTableSpace();
    int result = (sum/MEMORYSIZE)*100;
//    qDebug()<<result;
    curmemorypercent = result;
}

/**
 * @brief Widget::memoryCompaction
 * 内存紧缩
 */
void Widget::memoryCompaction()
{
    ui->compaction->setVisible(true);
    initProcsInMemory();
    memoryProcsUsedSort();//将进程按照起址从小到大排序
    int size = inmemory.size();
    int nextBegin = OSMEMORY;
    for(int i=0;i<size;i++){//将所有在内存的进程移动一遍进行紧缩，主要是改变PCB信息
        if(i>=inmemory.size())   break;
        inmemory.at(i)->getPCB()->setMemoryBegin(nextBegin);
        int beginAddr = inmemory.at(i)->getPCB()->getMemoryBegin();
        int width = inmemory.at(i)->getPCB()->getMemoryNeed();
        nextBegin = beginAddr + width;
    }

    int space = sumOfUnptTableSpace();
    int unptBegin = MEMORYSIZE - space;
    qDeleteAll(unpartitionTable);
    unpartitionTable.clear();
    unpartitionTable.append( new UnpartitionTable(unptBegin, space, NOTPARTIED) );
    refreshMemoryTable();
    refreshUnpartiTable();
}

/**
 * @brief Widget::initProcsInMemory
 * 更新inmemory表（在内存中的进程的指针的集合）
 */
void Widget::initProcsInMemory()
{
    inmemory.clear();
    for(int i=0;i<NUM_OF_PROCESS;i++){
        if(multitasks[i].getPCB()->getStatus()!=TERMINATED &&
                multitasks[i].getPCB()->getStatus()!=BACKUP)
        {
            SProcess *p = &multitasks[i];
            inmemory.append(p);
        }
    }

}

/**
 * @brief Widget::memoryProcsUsedSort
 * 将在内存中的进程按照地址排序
 */
void Widget::memoryProcsUsedSort()
{
    qSort(inmemory.begin(), inmemory.end(), cmpOfProcsUsedSort);
}

bool Widget::cmpOfProcsUsedSort(const SProcess *info1, const SProcess *info2)
{
    return info1->getPCB()->getMemoryBegin() < info2->getPCB()->getMemoryBegin();
}

void Widget::initWaiting()
{
    QMovie *movie = new QMovie(":/image/Image/waiting.gif");
    ui->waiting->setMovie(movie);
    movie->start();
}

void Widget::unpartitionTableSortBySize()
{
    qSort(unpartitionTable.begin(), unpartitionTable.end(), cmpOfUnptTableSortBySize);
}

bool Widget::cmpOfUnptTableSortBySize(const UnpartitionTable *info1, const UnpartitionTable *info2)
{
    return info1->width < info2->width;//按照空间大小排序
}

/**
 * @brief Widget::getIndexByFirst
 * @param procsWidth
 * @return 最先适应
 */
int Widget::getIndexByFirst(int procsWidth)
{
    unpartitionTableSort();//先按照起址排序，从小到大
    for(int i=0;i<unpartitionTable.size();i++){
        if(unpartitionTable.at(i)->width>=procsWidth){
            return i;
        }
    }
    return -1;
}

/**
 * @brief Widget::getIndexByBest
 * @param procsWidth
 * @return 最优适应
 */
int Widget::getIndexByBest(int procsWidth)
{
    unpartitionTableSortBySize();//先按照空间排序，从小到大
    for(int i=0;i<unpartitionTable.size();i++){
        if(unpartitionTable.at(i)->width>=procsWidth){
            return i;
        }
    }
    return -1;
}

/**
 * @brief Widget::getIndexByWorst
 * @param procsWidth
 * @return 最差适应
 */
int Widget::getIndexByWorst(int procsWidth)
{
    unpartitionTableSortBySize();//先按照空间排序，从小到大
    if(unpartitionTable.at(unpartitionTable.size()-1)->width>=procsWidth)
        return unpartitionTable.size()-1;
    else
        return -1;
}

/**
 * @brief Widget::timerEvent
 * @param eve
 * 中央控制总枢纽
 */
void Widget::timerEvent(QTimerEvent *eve)
{
    if(eve->timerId()==timerup){
        upBtn_Move_1();
    }
    else if (eve->timerId()==timercpuwait) {
        cpuReceivedtoWait();
    }
    else if(eve->timerId()==timerdown) {
        downBtn_Move_1();
    }
    else if (eve->timerId()==timercpudeltime) {
        delMoveProcsTime();
    }
    else if (eve->timerId()==timerstartall) {
        CPUscheduling();
    }
    else if (eve->timerId()==timerMonitNum) {
        workScheduling();
    }
    else if (eve->timerId()==timerdownwithslice) {
        downBtn_Move_1_withslice();
    }
    else if (eve->timerId()==timercrush) {
        if(isCrush)
            crushRunningProcs();
    }
    else if (eve->timerId()==timerpassrate) {
        if(passrate->getValue()<curmemorypercent)
            passrate->addValue();
        else if (passrate->getValue()>curmemorypercent)
            passrate->delValue();
        else
            this->killTimer(timerpassrate);
    }
}


void Widget::on_Start_clicked()
{
    initTimers();
    ifCPUschedule = true;
}

void Widget::on_Terminate_clicked()
{
    ifCPUschedule = false;
    ifCPUscheduleIsRunning = false;
    stopTimers();
}

/**
 * 随机添加10个作业进入后备队列
 */
void Widget::on_Random_clicked()
{
    for(int i=0;i<10;i++){
        QSqlDatabase db = getDB();
        bool ok = db.open();

        if(ok){
            QString pname = QString::number(random(10,99));
            QString intprio = QString::number(random(10,999));
            QString intrunt = QString::number(random(10,20));
            QString intmemory = QString::number(random(1,6));
            QString sql = QString("INSERT INTO backupque(pname, pstatus, priority, runtime, memory, memorybegin)"
                                  " VALUES('%1', 0, %2, %3, %4, -1)").arg(pname).arg(intprio).arg(intrunt).arg(intmemory);
            QSqlQuery insert(db);
            bool success = insert.exec(sql);
            if(success)
            {
                qDebug()<< "插入成功";
            }
            else
            {
                QMessageBox::information(nullptr, "database connect error", "数据库连接失败");
                return;
            }
        }
        else {
           qDebug()<< "Database open failed in on_Random_clicked()";
           return;
        }
    }
}

/**
 * @brief Widget::on_continue_2_clicked
 * 继续状态
 */
void Widget::on_continue_2_clicked()
{
    ifCPUscheduleIsRunning = true;
    initTimers();
}

/**
 * @brief Widget::on_Suspend_clicked
 * 点击挂起，弹出可挂起队列
 */
void Widget::on_Suspend_clicked()
{
    if(ifCPUscheduleIsRunning)
        on_Terminate_clicked();
    selesuspd->init();
    selesuspd->initSuspend(readyque->getMax_Procs(), readyque->getReadyQue(), moveProcs);
    selesuspd->initTable();
    selesuspd->show();//初始化可挂起队列表格并显示之
}

/**
 * @brief Widget::addtoSuspendQue
 * @param receive
 * 选择加入挂起的进程之后确定处理逻辑的槽函数
 */
void Widget::addtoSuspendQue(SProcess *receive)
{
//    qDebug()<<receive->getPCB()->getPID();
    if(receive->getPCB()->getStatus()==READY)//如果属于ready队列，直接从ready中删除然后进挂起队列，各种刷新
    {
        readyque->delSpecifiedProcsInReady(receive);//删除receive指针（在ready中）
    }
    if(receive->getPCB()->getStatus()==RUNNING)
    {
        curRunTime = 0;
        canUpMove = false;
        cpuWait = false;
        canDownMove = false;
        ifcpuWaitover = true;
        ifCPUschedule = true;//设置状态值使得从cpu调度开头开始
        moveProcs = nullptr;//置空正在移动的进程
        initMoveBtn_1();//刷新移动按钮位置
    }
    selesuspd->init();
    selesuspd->initSuspend(readyque->getMax_Procs(), readyque->getReadyQue(), moveProcs);
    selesuspd->initTable();//刷新可挂起队列表格

    receive->getPCB()->setStatus(SUSPEND);
    addSuspendQue(receive);
    refreshSuspendQue();
}

/**
 * @brief Widget::on_DeSuspend_clicked
 * 解挂函数，读取选中的进程信息，将其送入ready队列
 */
void Widget::on_DeSuspend_clicked()
{
    if(!ui->suspendque->selectedItems().empty()){
        int row = ui->suspendque->currentRow();
        QString pid = ui->suspendque->item(row,0)->text();
        int pri = ui->suspendque->item(row,1)->text().toInt();
        int tim = ui->suspendque->item(row,2)->text().toInt();
        SProcess *selected = searchProcsInSuspdQue(pid, pri, tim);

        if(selected!=nullptr)
        {
            delSuspendQue(selected);//删除挂起队列中的该指针
            selected->getPCB()->setStatus(READY);
            addIntoReady(selected);//添加进ready队列
        }
    }
    else {
        QMessageBox::warning(nullptr, "error", "请选择");
    }
}

void Widget::on_crush_stateChanged(int arg1)
{
    if(arg1==Qt::Checked){
        isCrush = true;
        timercrush = startTimer(5);
    }
    else{
        isCrush = false;
        this->killTimer(timercrush);
    }
}

void Widget::on_slice_stateChanged(int arg1)
{
    if(arg1==Qt::Checked){
        isSlice = true;
    }
    else{
        isSlice = false;
    }
}

void Widget::on_addrassignpatterns_currentIndexChanged(int index)
{
    addrAssign = index;
}

void Widget::on_btn_move_1_clicked()
{
    ReadyInfoDialog *infordialog = new ReadyInfoDialog;
    if(moveProcs!=nullptr){
        infordialog->setAll(moveProcs);
        infordialog->show();
    }
}
