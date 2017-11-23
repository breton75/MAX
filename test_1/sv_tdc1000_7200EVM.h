#ifndef SV_TDC1000_H
#define SV_TDC1000_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>
#include <QtEndian>

#include "sv_device_interface.h"
#include "ui_sv_select_device_dialog.h"

namespace Ui {
class SvSelectDeviceDialog;
}

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



class SvTDC1000_7200EVM : public svidev::SvIDevice
{
  Q_OBJECT
  
public:
  explicit SvTDC1000_7200EVM(svidev::DeviceInfo deviceInfo, QObject *parent = 0);
  
  ~SvTDC1000_7200EVM();
  
  bool open();
  void close();
  
  bool start(quint32 msecs);
  bool stop();
  
  static bool addNewDevice();
   
private:
  QSerialPort *_serial = nullptr;
  
  SvPullTDC1000_7200EVM* _pull_thr = nullptr;
  
};


/** ------------------------------------------- **/

class SvPullTDC1000_7200EVM: public QThread
{
    Q_OBJECT
  
public:
  explicit SvPullTDC1000_7200EVM(QSerialPort *serial, quint32 timeout);  
  ~SvPullTDC1000_7200EVM();
  
  void stop();
  
private:
  void run() Q_DECL_OVERRIDE;
  
  bool _started;
  bool _finished;
  
  quint32 _timeout;
  QSerialPort *_serial;
  
  qres _writeRead(QByteArray &request, int answer_count);
  
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