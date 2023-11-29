#include "addworks.h"
#include "ui_addworks.h"

AddWorks::AddWorks(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddWorks)
{
    ui->setupUi(this);
    setWindowTitle(QString("Add Works"));
}

AddWorks::~AddWorks()
{
    delete ui;
}

/*
 * 清空重置
 */
void AddWorks::on_retext_clicked()
{
    ui->pnamein->clear();
    ui->priorityin->clear();
    ui->runtimein->clear();
    ui->memoryin->clear();
}

/*
 * 提交新作业，并将其插入数据库
 */
void AddWorks::on_submit_clicked()
{
    QString pname = ui->pnamein->text();
    QString priority = ui->priorityin->text();
    QString runtime = ui->runtimein->text();
    QString memory = ui->memoryin->text();

    if(QString::compare(pname,"")!=0&&QString::compare(priority,"")!=0&&
            QString::compare(runtime,"")!=0&&QString::compare(memory,"")!=0)
    {
        bool priorityok, runtok, memok = false;
        int intprio = priority.toInt(&priorityok);
        int intrunt = runtime.toInt(&runtok);
        int intmemory = memory.toInt(&memok);
        if(priorityok && runtok && memok)
        {
            QSqlDatabase db;
            if(QSqlDatabase::contains("addworks"))
            {
                db = QSqlDatabase::database("addworks");
            }
            else {
                db = QSqlDatabase::addDatabase("QMYSQL","addworks");
            }
            db.setHostName("127.0.0.1");
            db.setPort(3306);
            db.setDatabaseName("mysql");
            db.setUserName("root");
            db.setPassword("123456");
            bool ok = db.open();

            if(ok)
            {
                QString sql = QString("INSERT INTO backupque(pname, pstatus, priority, runtime, memory, memorybegin)"
                                      " VALUES('%1', 0, %2, %3, %4, -1)").arg(pname).arg(intprio).arg(intrunt).arg(intmemory);
                QSqlQuery insert(db);
                bool success = insert.exec(sql);
                if(success)
                {
                    QMessageBox::information(nullptr, "database connect success", "插入成功");
                }
                else
                {
                    QMessageBox::information(nullptr, "database connect error", "数据库连接失败");
                    return;
                }
            }
            else {
                QMessageBox::information(nullptr, "database connect error", "数据库打开失败(addworks)");
                return;
            }
        }
        else
        {
            QMessageBox::warning(nullptr, "please retext", "优先权级或运行时间或主存占用必须输入整数");
            return;
        }
    }
    else
    {
        QMessageBox::warning(nullptr, "please complete", "输入的信息不可为空");
        return;
    }
}

