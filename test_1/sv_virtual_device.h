#ifndef SV_VIRTUALDEVICE_H
#define SV_VIRTUALDEVICE_H

#include <QDebug>
#include <QPair>
#include <QMap>
#include <QMessageBox>
#include <QByteArray>
#include <QMutex>
#include <QThread>
#include <QtEndian>
#include <QApplication>

#include "sv_device_interface.h"


class SvPullVirtualDevice;


class SvVirtualDevice : public svidev::SvIDevice
{
  Q_OBJECT
  
public:
  SvVirtualDevice();
  ~SvVirtualDevice();
  
  bool open();
  void close();
  
  bool start(quint32 msecs);
  bool stop();
    
private:
  SvPullVirtualDevice* _thr = nullptr;
  
};


class SvPullVirtualDevice: public QThread
{
    Q_OBJECT
  
public:
  explicit SvPullVirtualDevice(quint32 timeout, QMutex *mutex)
  {
    _timeout = timeout;
    _mutex = mutex;
  }
  
  ~SvPullVirtualDevice();

  void stop();
  
private:
  void run() Q_DECL_OVERRIDE;
  
  bool _started;
  bool _finished;
  quint32 _timeout;
  
  QMutex* _mutex;
  
signals:
  void new_data(svidev::MeasuredData data);
  
};

#endif // SV_VIRTUALDEVICE_H
