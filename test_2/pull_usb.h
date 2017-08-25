#ifndef PULL_USB_H
#define PULL_USB_H

#include <QDebug>
#include <QPair>
#include <QMap>
#include <QMessageBox>
#include <QByteArray>
#include <QMutex>

#include "libusb.h"

#define MAX_BUF_SIZE 1024
#define MAX35101EV_REQUEST_SIZE 64

namespace pullusb {

#pragma pack(push, 1)
struct MAX35101EV_ANSWER{
  qint16 in1;
  qint32 hit1_up;
  qint32 hit2_up;
  qint32 hit3_up;
  qint32 hit4_up;
  qint32 hit5_up;
  qint32 hit6_up;
  qint32 hit_up_average;
  qint16 in2;
  qint32 hit1_down;
  qint32 hit2_down;
  qint32 hit3_down;
  qint32 hit4_down;
  qint32 hit5_down;
  qint32 hit6_down;
  qint32 hit_down_average;
  qint16 in3;
  qint16 in4;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TDC1000_ANSWER{
  qint8  z0;
  qint16 time1;
  qint8  z1;
  qint16 clc1;
  qint8  z2;
  qint16 time2;
  qint8  z3;
  qint16 clc2;
  qint8  z4;
  qint16 time3;
  qint8  z5;
  qint16 clc3;
  qint8  z6;
  qint16 time4;
  qint8  z7;
  qint16 clc4;
  qint8  z8;  
  qint16 time5;
  qint8  z9;
  qint16 clc5;
  qint8  z10;
  qint16 time6;
  qint8  z11;
  qint16 calibr1;
  qint8  z12;
  qint16 calibr2;
  qint8  z13;
};
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct{
  int code;
  char *message;
  char* data;
} fres;
#pragma pack(pop)

fres *request(libusb_device_handle *handle);
fres *pullData(libusb_device_handle *handle, QByteArray &ba);

fres *request_texas(libusb_device_handle *handle);
fres *pullData_texas(libusb_device_handle *handle, QByteArray &ba);

}


#endif // PULL_USB_H
