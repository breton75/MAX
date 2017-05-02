#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QPair>
#include <QMap>
#include <QMessageBox>
#include <QByteArray>
#include <QMutex>
#include <QThread>
#include <QtEndian>

#include <QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

#include "libusb.h"
#include "pull_usb.h"
#include "chart.h"

namespace Ui {
class MainWindow;
}



class SvPullUsb;

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  
  libusb_device_handle* handle;
  
private slots:
  void on_bnGetDeviceList_clicked();
  
  void on_bnOneShot_clicked();
  
  void on_bnCycle_clicked();
  
  void new_data(pullusb::fres *result, pullusb::MAX35101EV_ANSWER *max_data);
  void pulling_finished();
  
private:
  Ui::MainWindow *ui;
  
  QMap<int, QPair<uint16_t, uint16_t>> _devices;
  SvPullUsb *_thr = nullptr;
  
  int _timerId;
  
  pullusb::MAX35101EV_ANSWER _max_data;
  
  Chart *_chart;
  QChartView *chartView;
  int _tick = 0;
  
//  int pullData(libusb_device_handle *handle, QByteArray &ba);
  
};


class SvPullUsb: public QThread
{
    Q_OBJECT
  
public:
  explicit SvPullUsb(libusb_device_handle *handle);
  
  ~SvPullUsb();
  
//  bool isWorking() { return _isWorking; }
//  bool isFinished() { return _isFinished; }
  
  void stop();
  pullusb::MAX35101EV_ANSWER max_data;
  
  
  
protected:
//  void run() Q_DECL_OVERRIDE;
  void timerEvent(QTimerEvent *te);
  
private:
//  bool _isWorking = false;
//  bool _isFinished = true;
  libusb_device_handle* _handle;
  
signals:
  void new_data(pullusb::fres *result, pullusb::MAX35101EV_ANSWER *max_data);
//  void consumer_updated(int , ConsumerDataStruct);
//  void sensor_updated(int , SensorDataStruct);
  
//  void recalc();
  
};

#endif // MAINWINDOW_H
