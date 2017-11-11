#ifndef SV_MAX35101EVALUATE_H
#define SV_MAX35101EVALUATE_H

#include <QDebug>
#include <QPair>
#include <QMap>
#include <QMessageBox>
#include <QByteArray>
#include <QMutex>

#include "libusb.h"
#include "sv_abstract_device_interface.h"

#define MAX_BUF_SIZE 1024
#define MAX35101EV_REQUEST_SIZE 64


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
typedef struct{
  int code;
  char *message;
  char* data;
} fres;
#pragma pack(pop)

fres *request(libusb_device_handle *handle);
fres *pullData(libusb_device_handle *handle, QByteArray &ba);


class SvMAX35101Evaluate : public SvAbstractDevice
{
  Q_OBJECT
  
public:
  SvMAX35101Evaluate();
  ~SvMAX35101Evaluate();
  
  bool open() Q_DECL_OVERRIDE;
  bool start(int msecs) Q_DECL_OVERRIDE;
  bool stop() Q_DECL_OVERRIDE;
  
  QString lastError() Q_DECL_OVERRIDE { return _last_error; }
  
private:
  libusb_device_handle* _handle = nullptr;
  
  QString _last_error = "";
  
signals:
  void new_data(svdevifc::MeasuredData data);
  
};

#endif // SV_MAX35101EVALUATE_H
