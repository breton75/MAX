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
#include <QDateTime>

#include <QVBoxLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpacerItem>
#include <QPushButton>

#include "libusb.h"

#include "sv_device_interface.h"
#include "ui_sv_select_device_dialog.h"


#define MAX_BUF_SIZE 1024
#define MAX35101EV_REQUEST_SIZE 64


namespace Ui {
class SvSelectDeviceDialog;
}



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

class SvPullMAX35101EV;


class SvMAX35101EV : public svidev::SvIDevice
{
  Q_OBJECT
  
public:
  explicit SvMAX35101EV(svidev::DeviceInfo deviceInfo, QObject *parent = 0);
  
  ~SvMAX35101EV();
  
  bool open();
  void close();
  
  bool start(quint32 msecs);
  void stop();
  
  static bool addNewDevice();
   
private:
  libusb_device_handle* _handle = nullptr;
  
  SvPullMAX35101EV* _pull_thr = nullptr;
  
};


/** ---------------  ---------------- **/

class SvSelectMAX35101EVDevice : public QDialog
{
  Q_OBJECT
  
public:
  explicit SvSelectMAX35101EVDevice(QWidget *parent = 0);
  ~SvSelectMAX35101EVDevice() { close(); deleteLater(); }
  
  svidev::DeviceInfo dinfo;
  
private:
  
  Ui::SvSelectDeviceDialog *ui;
  
  QMap<int, QPair<uint16_t, uint16_t>> _devices;
  
private slots:
  void accept() Q_DECL_OVERRIDE;
  
  void on_bnUpdateDeviceList_clicked();
  
};

/** ----------------  ---------------- **/

class SvPullMAX35101EV: public QThread
{
    Q_OBJECT
  
public:
  explicit SvPullMAX35101EV(libusb_device_handle *handle, quint32 timeout, QMutex *mutex);  
  ~SvPullMAX35101EV();

  void stop();
  
  MAX35101EV_ANSWER max_data;
  
private:
  void run() Q_DECL_OVERRIDE;

  fres* pullData(QByteArray &ba);
  
  bool _started;
  bool _finished;
  quint32 _timeout;
  libusb_device_handle* _handle;
  
  QMutex* _mutex;
  
signals:
  void new_data(const svidev::mdata_t& data);
  
};

#endif // SV_MAX35101EVALUATE_H
