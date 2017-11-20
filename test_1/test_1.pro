#-------------------------------------------------
#
# Project created by QtCreator 2017-04-28T10:51:31
#
#-------------------------------------------------

QT       += core gui charts printsupport network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test_1
TEMPLATE = app

#RC_FILE=test_1.rc

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG(debug, debug | release) {
  DEFINES += NO_USB_DEVICE
  DEFINES += NO_ARDUINO
}

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    pull_usb.cpp \
    ../../svlib/sv_settings.cpp \
    sv_chartwidget.cpp \
    ../../Common/qcustomplot/qcustomplot.cpp \
    sv_graph.cpp \
    sv_arduino_max.cpp \
    ../../svlib/sv_log.cpp \
    ../../svlib/sv_secondmeter.cpp \
    ../../svlib/sv_tcpserverclient.cpp \
    sv_max35101evaluate.cpp \
    sv_virtual_device.cpp \
    ../../svlib/sv_sqlite.cpp \
    sv_device_interface.cpp \
    sv_select_device_type_dialog.cpp


HEADERS  += mainwindow.h \
    ../../libusb-1.0.21/include/libusb-1.0/libusb.h \
    pull_usb.h \
    ../../svlib/sv_settings.h \
    ../../svlib/sv_fnt.h \
    sv_chartwidget.h \
    ../../Common/qcustomplot/qcustomplot.h \
    sv_graph.h \
    sv_arduino_max.h \
    ../../svlib/sv_log.h \
    ../../svlib/sv_secondmeter.h \
    ../../svlib/sv_tcpserverclient.h \
    sv_max35101evaluate.h \
    sv_virtual_device.h \
    sv_device_interface.h \
    ../../svlib/sv_sqlite.h \
    sv_select_device_type_dialog.h

FORMS    += mainwindow.ui \
    sv_graphparamsdialog.ui \
    engine_control.ui \
    sv_device_editor.ui \
    sv_select_device_type_dialog.ui \
    sv_select_max35101_device_dialog.ui

win32: LIBS += -L$$PWD/../../libusb-1.0.21/MinGW32/dll/ -llibusb-1.0

INCLUDEPATH += $$PWD/../../libusb-1.0.21/include/libusb-1.0
DEPENDPATH += $$PWD/../../libusb-1.0.21/include/libusb-1.0

INCLUDEPATH += $$PWD/../../Common/qcustomplot
DEPENDPATH += $$PWD/../../Common/qcustomplot

RESOURCES += \
    res.qrc
