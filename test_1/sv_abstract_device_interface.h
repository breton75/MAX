#ifndef SV_ABSTRACT_DEVICE_INTERFACE_H
#define SV_ABSTRACT_DEVICE_INTERFACE_H

#include <QObject>

#include "sv_max35101evaluate.h"

namespace svdevifc {

  typedef struct {
    qreal tof1 = 0.0;
    qreal tof2 = 0.0;
    qreal temperature = 0.0;
  } MeasuredData;

  enum SupportedDevices {
    NoDevice,
    MAX35101Evaluate,
    TDC1000_TDC7200EVM,
    AMEBuoy
  };

  typedef struct {
    QString name = "";
    uint16_t idVendor;
    uint16_t idProduct;
    uint8_t iManufacturer;
    uint16_t port;
  } DeviceInfo;
  
  class SvDeviceInterface;
  class SvAbstractDevice;

}

class svdevifc::SvDeviceInterface : public QObject
{
  Q_OBJECT
  
public:
  explicit svdevifc::SvDeviceInterface(svdevifc::SupportedDevices device, QObject *parent = 0);
  
  void setPullPeriod(int msecs) { _pull_period = msecs; }
  
  bool start(int msecs);
  bool stop();
  
  
  
  
private:
  int _pull_period;
  int _current_device = -1;
  
  SvAbstractDevice* _current_device = nullptr;
  
  svdevifc::MeasuredData _last_measure;
  
  
signals:
  void new_data(svdevifc::MeasuredData );
  
public slots:
  
};

class SvAbstractDevice : public QObject
{
    Q_OBJECT
    
public:
  svdevifc::SvAbstractDevice(svdevifc::DeviceInfo deviceInfo, QObject *parent = 0)
  {
    _device_info = deviceInfo;
    setParent(parent);
  }

  virtual ~SvAbstractDevice();
  
  virtual bool open();
  
  virtual bool start(int msecs);
  virtual bool stop();
  
  virtual QString lastError();
  
  void setDeviceInfo(svdevifc::DeviceInfo deviceInfo) { _device_info = deviceInfo; }
  svdevifc::DeviceInfo deviceInfo() { return _device_info; }
  
private:
  svdevifc::DeviceInfo _device_info;
    
signals:
    virtual void new_data(svdevifc::MeasuredData data);
  
public slots:
    
};

#endif // SV_ABSTRACT_DEVICE_INTERFACE_H
