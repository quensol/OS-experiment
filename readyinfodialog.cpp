#include "readyinfodialog.h"
#include "ui_readyinfodialog.h"

ReadyInfoDialog::ReadyInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReadyInfoDialog)
{
    ui->setupUi(this);
    setWindowTitle(QString("Process Information"));
}

ReadyInfoDialog::~ReadyInfoDialog()
{
    delete ui;
}

void ReadyInfoDialog::setAll(SProcess *procs)
{
    ui->view_pid->setText(procs->getPCB()->getPID());
//    ui->view_status->setText(QString::number(procs->getPCB()->getStatus()));
    ui->view_status->setText("就绪（Ready）");
    ui->view_priority->setText(QString::number(procs->getPCB()->getPriority()));
    ui->view_time->setText(QString::number(procs->getPCB()->getRuntime()));
    ui->view_memory->setText(QString::number(procs->getPCB()->getMemoryNeed()));
    ui->view_beginAddr->setText(QString::number(procs->getPCB()->getMemoryBegin()+1));
}
