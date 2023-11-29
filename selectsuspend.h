
#ifndef SELECTSUSPEND_H
#define SELECTSUSPEND_H
#define NUM_OF_SUSPEND 6

#include <QWidget>
#include <QMessageBox>
#include "sprocess.h"

namespace Ui {
class SelectSuspend;
}

class SelectSuspend : public QWidget
{
    Q_OBJECT
signals:
    void selectok(SProcess *selected);//向主界面发送选中的进程
public:
    explicit SelectSuspend(QWidget *parent = 0);
    ~SelectSuspend();
    void initSuspend(int numready, SProcess **ready,SProcess *running);//根据传入的ready队列和目标几次呢指针将目标进程从ready调进挂起
    void init();//置空表格
    void initTable();//创建时的初始化表格函数
    SProcess *searchProcs(QString PID, int pri, int tim);//在挂起队列范围中搜索目标进程

private slots:
    void on_yes_clicked();//确定选定的进程对应的槽函数

private:
    Ui::SelectSuspend *ui;
    SProcess *suspend[NUM_OF_SUSPEND];//暂存挂起队列所有进程指针
    int max_of_suspend;//所有挂起队列进程数目
};

#endif // SELECTSUSPEND_H
