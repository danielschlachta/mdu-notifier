#-------------------------------------------------
#
# Project created by QtCreator 2019-05-20T09:21:28
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mdu-notifier
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    bargraph.cpp \
    listdialog.cpp \
        main.cpp \
        mainwindow.cpp \
        runguard.cpp \
        server.cpp \
    secretdialog.cpp \
    showtraffic.cpp \
    slotlist.cpp

HEADERS += \
    bargraph.h \
    common.h \
    common.h \
    listdialog.h \
        mainwindow.h \
        runguard.h \
        server.h \
    secretdialog.h \
    showtraffic.h \
    slotlist.h

FORMS += \
        mainwindow.ui \
        showtraffic.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
else: unix:!android: target.path = ~/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    mdu-notifier.qrc

DISTFILES += \
    mdu-notifier.desktop \
    mdu-notifier.rc

RC_FILE = mdu-notifier.rc
