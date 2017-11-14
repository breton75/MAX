#ifndef SV_MAX35101EVALUATE_H
#define SV_MAX35101EVALUATE_H

#include <QDebug>
#include <QPair>
#include <QMap>
#include <QMessageBox>
#include <QByteArray>
#include <QMutex>
#include <QThread>
#include <QtEndian>
#include <QApplication>

#include "libusb.h"
#include "sv_device_interface.h"

#define MAX_BUF_SIZE 1024
#define MAX35101EV_REQUEST_SIZE 64


#pragma pack(push, 1)
struct MAX35101EV_ANSWER {
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
  char* message;
  char* data;
} fres;
#pragma pack(pop)

class SvPullMAX35101Evaluate;


class SvMAX35101Evaluate : public svidev::SvIDevice
{
  Q_OBJECT
  
public:
  explicit SvMAX35101Evaluate(svidev::DeviceInfo deviceInfo, QObject *parent = 0);
  
  ~SvMAX35101Evaluate();
  
  bool open();
  void close();
  
  bool start(quint32 msecs);
  bool stop();
  
private:
  libusb_device_handle* _handle = nullptr;
  
  SvPullMAX35101Evaluate* _thr = nullptr;
  
signals:
//  void new_data(svidev::MeasuredData data);
  
};


class SvPullMAX35101Evaluate: public QThread
{
    Q_OBJECT
  
public:
  explicit SvPullMAX35101Evaluate(libusb_device_handle *handle, quint32 timeout, QMutex *mutex)
  {
    _handle = handle;
    _timeout = timeout;
    _mutex = mutex;
  }
  
  ~SvPullMAX35101Evaluate();

  void stop();
  
  MAX35101EV_ANSWER max_data;
  
  
protected:
//  void timerEvent(QTimerEvent *te);
  
private:
  void run() Q_DECL_OVERRIDE;

  fres* pullData(QByteArray &ba);
  
  bool _started;
  bool _finished;
  quint32 _timeout;
  libusb_device_handle* _handle;
  
  QMutex* _mutex;
  
signals:
  void new_data(svidev::MeasuredData data);
  
};

#endif // SV_MAX35101EVALUATE_H
