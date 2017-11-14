#ifndef SV_DEVICE_INTERFACE_H
#define SV_DEVICE_INTERFACE_H

#include <QObject>
#include <QMutex>


namespace svidev {

  typedef struct {
    qreal tof1 = 0;
    qreal tof2 = 0;
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
  
//  class SvDeviceInterface;
  class SvIDevice;

}


class svidev::SvIDevice : public QObject
{
    Q_OBJECT
    
public:

  virtual ~SvIDevice() { }
  
  virtual bool open() = 0;
  virtual void close() = 0;
  
  virtual bool start(quint32 msecs) = 0;
  virtual bool stop() = 0;
  
  QMutex mutex;
  
  void setLastError(QString lastError) { _last_error = lastError; }
  QString lastError() { return _last_error; }
  
  void setDeviceInfo(svidev::DeviceInfo &deviceInfo) { _device_info = deviceInfo; }
  svidev::DeviceInfo deviceInfo() { return _device_info; }
  
  void setOpened(bool isOpened) { _isOpened = isOpened; }
  bool isOpened() { return _isOpened; }
  
protected:
  svidev::DeviceInfo _device_info;
  bool _isOpened;
  QString _last_error;
    
signals:
  void new_data(svidev::MeasuredData data);
      
};

#endif // SV_DEVICE_INTERFACE_H
