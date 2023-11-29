#include "selectsuspend.h"
#include "ui_selectsuspend.h"

SelectSuspend::SelectSuspend(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectSuspend)
{
    ui->setupUi(this);
    setWindowTitle(QString("Selection"));
    init();
}

SelectSuspend::~SelectSuspend()
{
    delete ui;
}

/**
 * @brief SelectSuspend::initSuspend
 * @param numready
 * @param ready
 * @param running
 * 传入参数初始化挂起队列数组
 */
void SelectSuspend::initSuspend(int numready, SProcess **ready, SProcess *running)
{
    for(int i=0;i<numready;i++){
        suspend[i] = ready[i];
        max_of_suspend++;
    }
    if(running!=nullptr)
    {
        if(running->getPCB()->getStatus()==RUNNING){
            suspend[max_of_suspend] = running;
            max_of_suspend++;
        }
    }
}

/**
 * @brief SelectSuspend::init
 * 创建时初始化
 */
void SelectSuspend::init()
{
    ui->suspendque->setRowCount(0);
    ui->suspendque->clearContents();
    max_of_suspend = 0;
    for(int i=0;i<NUM_OF_SUSPEND;i++){
        suspend[i] = nullptr;
    }
}

/**
 * @brief SelectSuspend::initTable
 * 初始化刷新表格
 */
void SelectSuspend::initTable()
{
    QString pri;
    QString tim;
    QString status;
    for(int i=0;i<max_of_suspend;i++){
        pri = QString::number(suspend[i]->getPCB()->getPriority());
        tim = QString::number(suspend[i]->getPCB()->getRuntime());
        status = suspend[i]->getPCB()->getStatus()==READY?"就绪":"运行中";
        int RowCont = ui->suspendque->rowCount();
        ui->suspendque->insertRow(RowCont);//增加一行
        ui->suspendque->setItem(RowCont,0,new QTableWidgetItem(suspend[i]->getPCB()->getPID()));
        ui->suspendque->setItem(RowCont,1,new QTableWidgetItem(status));
        ui->suspendque->setItem(RowCont,2,new QTableWidgetItem(pri));
        ui->suspendque->setItem(RowCont,3,new QTableWidgetItem(tim));
    }
}

/**
 * @brief SelectSuspend::searchProcs
 * @param PID
 * @param pri
 * @param tim
 * @return 返回搜索到的符合条件的指针
 */
SProcess *SelectSuspend::searchProcs(QString PID, int pri,int tim)
{
    for(int i=0;i<max_of_suspend;i++){
        if(suspend[i]->getPCB()->getPID()==PID && suspend[i]->getPCB()->getPriority()==pri
                &&suspend[i]->getPCB()->getRuntime()==tim)
        {
            return suspend[i];
        }
    }
    qDebug()<<"not find in searchProcs";
    return nullptr;
}

/**
 * @brief SelectSuspend::on_yes_clicked
 * 选定要挂起的进程
 */
void SelectSuspend::on_yes_clicked()
{
    if(!ui->suspendque->selectedItems().empty()){
        int row = ui->suspendque->currentRow();
        QString pid = ui->suspendque->item(row,0)->text();
        int pri = ui->suspendque->item(row,2)->text().toInt();
        int tim = ui->suspendque->item(row,3)->text().toInt();
        SProcess *selected = searchProcs(pid, pri, tim);

        if(selected!=nullptr){
            emit selectok(selected);
        }
        else {
            QMessageBox::warning(nullptr, "error", "no such process");
        }
    }
    else {
        QMessageBox::warning(nullptr, "error", "请选择");
    }
}
