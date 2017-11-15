#ifndef SV_DEVICE_INTERFACE_H
#define SV_DEVICE_INTERFACE_H

#include <QObject>
#include <QMutex>
#include <QDialog>
#include <QMap>
#include <QDateTime>
#include <QTextEdit>

#include "ui_sv_device_editor.h"

#include "../../svlib/sv_sqlite.h"
#include "../../svlib/sv_log.h"

namespace Ui {
class SvDeviceEditorDialog;
}

#define SQL_SELECT_DEVICE "select id, device_type, vendor_id, product_id, "       \
                          "       manufacturer_id, port, date_time, description " \
                          "from devices where id = %1"

#define SQL_INSERT_DEVICE "insert into devices (device_type, vendor_id, product_id, "             \
                          "                     manufacturer_id, port, date_time, description) "  \
                          "values(%1, %2, %3, %4, %5, '%6', '%7')"

#define SQL_DELETE_DEVICE "delete from devices where id = %1"





namespace svidev {

  typedef struct {
    qreal tof1 = 0;
    qreal tof2 = 0;
    qreal temperature = 0.0;
  } MeasuredData;

  enum SupportedDevices {
    NoDevice,
    MAX35101EV,
    TDC1000_TDC7200EVM,
    AMEBuoy
  };

  typedef struct {
    int dbid;
    uint8_t deviceType;
    QString name = "";
    uint16_t idVendor;
    uint16_t idProduct;
    uint8_t iManufacturer;
    uint16_t port;
  } DeviceInfo;
  
  const QMap<SupportedDevices, QString> SupportedDevicesNames = {
            {NoDevice, "Виртуальное устройство"}, {MAX35101EV, "MAX35101EV"},
            {TDC1000_TDC7200EVM, "TDC1000_TDC7200EVM"}, {AMEBuoy, "Буй АМЭ"} };
  
  
//  class SvDeviceInterface;
  class SvIDevice;

  
  
}

/** ----------- SvIDevice ------------ **/
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

/** ------------ SvDeviceEditorDialog ------------- **/
class SvDeviceEditor : public QDialog
{
  Q_OBJECT
  
public:
  enum ShowMode { smNew = 0, smEdit = 1 };
  
  explicit SvDeviceEditor(QTextEdit *logWidget, int dbid = -1, QWidget *parent = 0);
  ~SvDeviceEditor();
  
  
  svidev::DeviceInfo dinfo;
  
//  int     t_id;
//  QString t_uid = "";
////  QString t_modelName = "";
////  QString t_className = "";
////  QString t_brandName = "";

//  QDateTime t_date_time;
//  QString t_description = "";
  
private:
  Ui::SvDeviceEditorDialog *ui;
  
  int _showMode;
  QString _curConnectionType = "";
  
  svlog::SvLog _log;
  
public slots:
  void accept() Q_DECL_OVERRIDE;
  
private slots:
  void setCurrentConnectionType(int index);
  
};


#endif // SV_DEVICE_INTERFACE_H
