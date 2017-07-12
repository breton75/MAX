#-------------------------------------------------
#
# Project created by QtCreator 2017-04-28T10:51:31
#
#-------------------------------------------------

QT       += core gui charts printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test_1
TEMPLATE = app

#RC_FILE=test_1.rc

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
        mainwindow.cpp \
    pull_usb.cpp \
    ../../Common/sv_settings.cpp \
    sv_chartwidget.cpp \
    ../../Common/qcustomplot/qcustomplot.cpp \
    sv_graph.cpp \
    sv_arduino_max.cpp


HEADERS  += mainwindow.h \
    ../../libusb-1.0.21/include/libusb-1.0/libusb.h \
    pull_usb.h \
    ../../Common/sv_settings.h \
    ../../Common/sv_fnt.h \
    sv_chartwidget.h \
    ../../Common/qcustomplot/qcustomplot.h \
    sv_graph.h \
    sv_arduino_max.h

FORMS    += mainwindow.ui \
    sv_graphparamsdialog.ui \
    engine_control.ui

win32: LIBS += -L$$PWD/../../libusb-1.0.21/MinGW32/dll/ -llibusb-1.0

INCLUDEPATH += $$PWD/../../libusb-1.0.21/include/libusb-1.0
DEPENDPATH += $$PWD/../../libusb-1.0.21/include/libusb-1.0

INCLUDEPATH += $$PWD/../../Common/qcustomplot
DEPENDPATH += $$PWD/../../Common/qcustomplot

RESOURCES += \
    res.qrc
