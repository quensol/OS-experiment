
QT       += core gui
QT       += sql
RC_ICONS  = Image/os.ico

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OSsimulation
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        widget.cpp \
    sprocess.cpp \
    spcb.cpp \
    backupque.cpp \
    addworks.cpp \
    readyque.cpp \
    readyinfodialog.cpp \
    selectsuspend.cpp \
    passrate.cpp

HEADERS  += widget.h \
    sprocess.h \
    spcb.h \
    backupque.h \
    addworks.h \
    readyque.h \
    readyinfodialog.h \
    selectsuspend.h \
    passrate.h

FORMS    += widget.ui \
    backupque.ui \
    addworks.ui \
    readyque.ui \
    readyinfodialog.ui \
    selectsuspend.ui \
    passrate.ui

RESOURCES += \
    beautify.qrc
