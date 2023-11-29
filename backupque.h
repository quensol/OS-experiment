
#ifndef BACKUPQUE_H
#define BACKUPQUE_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QTableWidget>
#include <QDebug>

namespace Ui {
class BackUpQue;
}

class BackUpQue : public QWidget
{
    Q_OBJECT

public:
    explicit BackUpQue(QWidget *parent = 0);
    ~BackUpQue();
    void initQueue();//初始化载入数据库数据
    void refreshAgain();//刷新表格

private slots:
    void on_refresh_clicked();

private:
    Ui::BackUpQue *ui;
};

#endif // BACKUPQUE_H
