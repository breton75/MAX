#ifndef SV_DEVICE_INTERFACE_H
#define SV_DEVICE_INTERFACE_H

#include <QObject>
#include <QMutex>
#include <QDialog>
#include <QMap>
#include <QDateTime>
#include <QTextEdit>
#include <QMetaType>

//#include "ui_sv_device_editor.h"
#include "ui_sv_select_device_type_dialog.h"

#include "../../svlib/sv_sqlite.h"
#include "../../svlib/sv_log.h"

namespace Ui {
//class SvDeviceEditorDialog;
class SvSelectDeviceTypeDialog;
}

#define SQL_CREATE_TABLE_DEVICES "create table devices (id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, " \
                                 "device_type INTEGER, device_name TEXT, vendor_id INTEGER, "     \
                                 "product_id INTEGER, manufacturer_id INTEGER, port_name TEXT, " \
                                 "date_time TEXT, description TEXT)"

#define SQL_SELECT_DEVICE_BY_ID "select id, device_type, device_name, vendor_id, product_id, "       \
                                "manufacturer_id, port_name, date_time, description " \
                                "from devices where id = %1"

#define SQL_NEW_DEVICE "insert into devices (device_type, device_name, vendor_id, "  \
                       "product_id, manufacturer_id, port_name, date_time, description) " \
                       "values(%1, '%2', %3, %4, %5, '%6', '%7', '%8')"

#define SQL_SELECT_DEVICE_LIST "select id, device_type, device_name, vendor_id, product_id, "       \
                               "manufacturer_id, port_name, date_time, description " \
                               "from devices order by id asc"

#define SQL_DELETE_DEVICE "delete from devices where id = %1"





namespace svidev {

  typedef struct {
    qreal tof1 = 0;
    qreal tof2 = 0;
    qreal temperature = 0.0;
  } MeasuredData;

  typedef MeasuredData mdata_t;
  
  
  enum SupportedDevices {
    VirtualDevice,
    MAX35101EV,
    TDC1000_TDC7200EVM,
    AMEBuoy
  };

  typedef SupportedDevices dev_t;

  
  typedef struct {
    int dbid;
    dev_t deviceType;
    QString deviceName = "";
    uint16_t idVendor;
    uint16_t idProduct;
    uint8_t iManufacturer;
    QString portName = "";
  } DeviceInfo;
  
  const QMap<SupportedDevices, QString> SupportedDevicesNames = {
            {VirtualDevice, "Виртуальное устройство"}, {MAX35101EV, "MAX35101EV"},
            {TDC1000_TDC7200EVM, "TDC1000_TDC7200EVM"}, {AMEBuoy, "Буй АМЭ"} };
  
  
//  class SvDeviceInterface;
  class SvIDevice;

  class SvSelectDeviceType;
  
  QSqlError open_db(QString dbFileName);
  QSqlError fill_device_list(QComboBox *cb);
  QSqlError fill_device_info(int dbid, svidev::DeviceInfo &dinfo);
  
}

/** ----------- SvIDevice ------------ **/
class svidev::SvIDevice : public QObject
{
    Q_OBJECT
    
public:
  SvIDevice() { 
    
    qRegisterMetaType<svidev::MeasuredData>("svidev::mdata_t"); 
    
  }
  
  virtual ~SvIDevice() { }
  
  virtual bool open() = 0;
  virtual void close() = 0;
  
  virtual bool start(quint32 msecs) = 0;
  virtual void stop() = 0;
  
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
  void new_data(const svidev::mdata_t& data);
      
};

/** ------------ SvDeviceEditorDialog ------------- **/
//class SvDeviceEditor : public QDialog
//{
//  Q_OBJECT
  
//public:
//  enum ShowMode { smNew = 0, smEdit = 1 };
  
//  explicit SvDeviceEditor(QTextEdit *logWidget, int dbid = -1, QWidget *parent = 0);
//  ~SvDeviceEditor();
  
  
//  svidev::DeviceInfo dinfo;
  
////  int     t_id;
////  QString t_uid = "";
//////  QString t_modelName = "";
//////  QString t_className = "";
//////  QString t_brandName = "";

////  QDateTime t_date_time;
////  QString t_description = "";
  
//private:
//  Ui::SvDeviceEditorDialog *ui;
  
//  int _showMode;
//  QString _curConnectionType = "";
  
//  svlog::SvLog _log;
  
//public slots:
//  void accept() Q_DECL_OVERRIDE;
  
//private slots:
//  void setCurrentConnectionType(int index);
  
//};


/** ---------------  ---------------- **/

class svidev::SvSelectDeviceType : public QDialog
{
  Q_OBJECT
  
public:
  explicit SvSelectDeviceType(QWidget *parent = 0);
  ~SvSelectDeviceType() { close(); delete ui; }
  
  svidev::SupportedDevices type_id;
  
private:
  Ui::SvSelectDeviceTypeDialog *ui;
  
  
private slots:
  void accept() Q_DECL_OVERRIDE;
  
};

//Q_DECLARE_METATYPE(svidev::MeasuredData)

#endif // SV_DEVICE_INTERFACE_H
