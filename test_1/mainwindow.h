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
#include <QComboBox>
#include <QRgb>
#include <QTime>
#include <QFileDialog>

#include <QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

#include "libusb.h"
#include "pull_usb.h"
#include "sv_chartwidget.h"

#include "../../Common/sv_settings.h"

namespace Ui {
class MainWindow;
}

struct GraphInfo {
  int type_id;
  QString name;
  QCPGraph *graph;
};

class SvPullUsb;

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  
  libusb_device_handle* handle;
//  QTimer tm;
  
private slots:
  void on_bnGetDeviceList_clicked();
  void on_bnOneShot_clicked();
  void on_bnCycle_clicked();
  void on_cbViewType_currentIndexChanged(int index);
  
  void new_data(pullusb::fres *result);
  
  void on_checkSaveToFile_clicked(bool checked);
  void on_bnSaveFileSelectPath_clicked();

 
  void on_bnOpenFile_clicked();
  
  void on_bnAddGraph_clicked();
  
private:
  Ui::MainWindow *ui;
  
  QMap<int, QPair<uint16_t, uint16_t>> _devices;
  
  SvPullUsb *_thr = nullptr;
  int _timerId;
  
  pullusb::MAX35101EV_ANSWER _max_data;
  
//  svchart::Chart *_chart = nullptr;
//  QChartView *chartView;
  
  svchart::SvChartWidget *_chart_w = nullptr;
  QMap<int, QString> _plot_types;
  
  
  QMap<int, QCPGraph*> _graphs;
  
  int _tick = 0;
//  qreal _y_range = 1;
  
  svchart::ChartParams _chp;

  QFile *_file = nullptr;
   
};


class SvPullUsb: public QThread
{
    Q_OBJECT
  
public:
  explicit SvPullUsb(libusb_device_handle *handle, quint32 timeout)
  {
    _handle = handle;
    _timeout = timeout;
  }
  
  ~SvPullUsb();

  void stop();
  
  pullusb::MAX35101EV_ANSWER max_data;
  
  
protected:
//  void timerEvent(QTimerEvent *te);
  
private:
  void run() Q_DECL_OVERRIDE;
  
  bool _started;
  bool _finished;
  quint32 _timeout;
  libusb_device_handle* _handle;
  
signals:
  void new_data(pullusb::fres *result/*, pullusb::MAX35101EV_ANSWER *max_data*/);
  
};

#endif // MAINWINDOW_H
