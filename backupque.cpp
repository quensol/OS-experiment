#include "backupque.h"
#include "ui_backupque.h"

BackUpQue::BackUpQue(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BackUpQue)
{
    ui->setupUi(this);
    setWindowTitle("后备队列");
    /* 设置表格不允许编辑 */
    ui->backupqueue->setEditTriggers(QAbstractItemView::NoEditTriggers);
    initQueue();
}

BackUpQue::~BackUpQue()
{
    delete ui;
}

/*
 * 初始化展示的表格
 */
void BackUpQue::initQueue()
{
    QSqlDatabase db;
    if(QSqlDatabase::contains("backup"))
    {
        db = QSqlDatabase::database("backup");
    }
    else {
        db = QSqlDatabase::addDatabase("QMYSQL","backup");
    }
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("mysql");
    db.setUserName("root");
    db.setPassword("123456");
    bool ok = db.open();

    if(ok){
        qDebug()<<"init backupqueue successful!";
        QSqlQuery result = db.exec(QString("SELECT pname,pstatus,priority,runtime,memory FROM backupque ORDER BY id ASC"));
        while (result.next()) {
            int RowCont = ui->backupqueue->rowCount();
            ui->backupqueue->insertRow(RowCont);//增加一行
            ui->backupqueue->setItem(RowCont,0,new QTableWidgetItem(result.value("pname").toString()));
            ui->backupqueue->setItem(RowCont,1,new QTableWidgetItem(QString("后备")));
            ui->backupqueue->setItem(RowCont,2,new QTableWidgetItem(result.value("priority").toString()));
            ui->backupqueue->setItem(RowCont,3,new QTableWidgetItem(result.value("runtime").toString()));
            ui->backupqueue->setItem(RowCont,4,new QTableWidgetItem(result.value("memory").toString()));
        }
        db.close();
    }
    else{
        qDebug()<<"database open failed!";
    }
}

void BackUpQue::refreshAgain()
{
    ui->backupqueue->clearContents();
    ui->backupqueue->setRowCount(0);
    initQueue();
}

/*
 * 表格刷新函数，清空表格之后再初始化
 */
void BackUpQue::on_refresh_clicked()
{
    ui->backupqueue->setRowCount(0);
    ui->backupqueue->clearContents();
    initQueue();
}
