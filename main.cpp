
#include "widget.h"
#include <QApplication>
#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.setWindowTitle("OSsimulation");
    w.setWindowIcon(QIcon(":/image/Image/os.png"));
    w.show();
    w.CPUscheduling();

    return a.exec();
}
