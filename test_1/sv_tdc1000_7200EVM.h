#ifndef SV_TDC1000_H
#define SV_TDC1000_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>
#include <QtEndian>
#include <QTimer>

#include "sv_device_interface.h"
#include "ui_sv_select_device_dialog.h"

namespace Ui {
class SvSelectDeviceDialog;
}

#define TDC_CALIBRATION2_PERIODS 10
#define TDC_CLOCK 8000000

#define TDC_PARAMS_ANSWER_SIZE 32

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

struct qres {
  bool result = true;
  QString msg = "";
  qreal t1 = 0;
  qreal t2 = 0;
  QByteArray answer = QByteArray();  
};

//qres pullTDC1000(QSerialPort *serial);
//qres writeReadTDC1000(QSerialPort *serial, QByteArray data, int answer_count);

class SvPullTDC1000_7200EVM;
class SvSerialTDC1000_7200EVM;

class SvTDC1000_7200EVM : public svidev::SvIDevice
{
  Q_OBJECT
  
public:
  explicit SvTDC1000_7200EVM(svidev::DeviceInfo deviceInfo, QObject *parent = 0);
  
  ~SvTDC1000_7200EVM();
  
  bool open();
  void close();
  
  bool start(quint32 msecs);
  void stop();
  
  static bool addNewDevice();
  
  QSerialPort port;
  
  svidev::mdata_t last_data;
  
private:
  QSerialPortInfo _port_info;
  
//  SvSerialTDC1000_7200EVM *_serial_port = nullptr;
//  SvPullTDC1000_7200EVM* _pull_thr = nullptr;
  
//  QThread *_serial_thread = nullptr;
  
  void err() { qDebug() << "thread finished"; }

  void write_params(const QByteArray &params);
  bool ParamsWritten = true;
  
  quint8 _ch_num = 1;
  
private slots:
  void read_data();
  void write_data();
  
};

//class SvSerialTDC1000_7200EVM : public QObject
//{
//  Q_OBJECT
  
//public:
//  explicit SvSerialTDC1000_7200EVM(const QSerialPortInfo &portInfo, QObject *parent = 0):
//    QObject(parent)
//  {
//    _port_info = portInfo;
//    port.setPort(_port_info);
//  }
  
//  ~SvSerialTDC1000_7200EVM() { qDebug() << "destroyed"; deleteLater(); }
  
//  bool open();
//  void close();
  
//  bool start(quint32 msecs);
//  void stop();
  
//  QString lastError() { return _last_error; }
  
//  QSerialPort port;
  
//  svidev::mdata_t last_data;
   
//private:
//  QSerialPortInfo _port_info;
//  QString _last_error = "";
  
//  QTimer _timer;
  
//  quint8 ch_num = 1;
  
//private slots:
//  void read_data();
//  void write_data();
////  void onSerialPortError();
  
  
//signals:
//  void error(const QString&);
//  void new_data(const svidev::mdata_t& data);
  
//};


/** ------------------------------------------- **/

class SvPullTDC1000_7200EVM: public QThread
{
    Q_OBJECT
  
public:
  explicit SvPullTDC1000_7200EVM(const QSerialPortInfo &portInfo, QMutex *mutex);  
  ~SvPullTDC1000_7200EVM();
  
  bool open();
  void stop();
  
  QString lastError() { return _last_error; }
  
  void setTimeout(quint32 msecs) { _timeout = msecs; }
  
  QSerialPort *_serial = nullptr;
  
private:
  void run() Q_DECL_OVERRIDE;
  
  bool _started = false;
  bool _finished = true;
  
  quint32 _timeout;
  
  QSerialPortInfo _port_info;
  
  QMutex* _mutex;
  
  QString _last_error = "";
  
  qres _writeRead(const QByteArray &request, int answer_count);
  
signals:
  void new_data(const svidev::mdata_t& data);
  
};


/** ---------------  ---------------- **/

class SvSelectTDC1000_7200EVMDevice : public QDialog
{
  Q_OBJECT
  
public:
  explicit SvSelectTDC1000_7200EVMDevice(QWidget *parent = 0);
  ~SvSelectTDC1000_7200EVMDevice() { close(); deleteLater(); }
  
  svidev::DeviceInfo dinfo;
  
private:
  
  Ui::SvSelectDeviceDialog *ui;
  
  QMap<int, QString> _devices;
  
private slots:
  void accept() Q_DECL_OVERRIDE;
  
  void on_bnUpdateDeviceList_clicked();
  
};

/** ----------------  ---------------- **/

#endif // SV_TDC1000_H
