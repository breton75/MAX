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
#include "sv_graph.h"
#include "sv_arduino_max.h"

#include "../../svlib/sv_settings.h"

#include "../../svlib/sv_log.h"
#include "../../svlib/sv_tcpserverclient.h"

#define NO_USB_DEVICE

namespace Ui {
class MainWindow;
}

struct GraphInfo {
  int type_id;
  QString name;
  QCPGraph *graph;
};

/** структуры заголовков для записи в файл **/

#pragma pack(1)
struct FileHeader {
  char signature[15] = {'F','L','O','W',' ','R','A','T','E',' ','S','T','A','N','D'};
  int graph_count;
};
#pragma pack(pop)

#pragma pack(1)
struct GraphHeader {
//  int legend_length;
//  uchar legend[256];
//  QByteArray legend;
  int graph_id;
  int line_width;
  quint32 line_color;
  int line_style;
};
#pragma pack(pop)

/** ************************ **/

class SvPullUsb;

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
  QString FILE_EXT = "frs";
  
public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  
  libusb_device_handle* handle = nullptr;
//  QTimer tm;
  
  svlog::SvLog log;
  
private slots:
  void on_bnGetDeviceList_clicked();
  void on_bnOneShot_clicked();
  void on_bnCycle_clicked();
  
  void new_data(pullusb::fres *result);
  
//  void on_checkSaveToFile_clicked(bool checked);
  void on_bnSaveFileSelectPath_clicked();
  void on_bnOpenFile_clicked();
  void on_bnAddGraph_clicked();
  void on_bnSaveToFile_clicked(bool checked);
  void on_bnEditGraph_clicked();
  void on_bnRemoveGraph_clicked();
  void on_listGraphs_currentRowChanged(int currentRow);
  void on_listGraphs_doubleClicked(const QModelIndex &index);
  
  void stateChanged(bool state);
  
private:
  Ui::MainWindow *ui;
  
  QMap<int, QPair<uint16_t, uint16_t>> _devices;
  
  SvPullUsb *_thr = nullptr;
  int _timerId;
  
  pullusb::MAX35101EV_ANSWER _max_data;
  
  QMap<svgraph::GraphIDs, qreal> _calcs;
  
//  svchart::Chart *_chart = nullptr;
//  QChartView *chartView;
  
  svchart::SvChartWidget *_chart_w = nullptr;
  QMap<int, QString> _plot_types;
  
  
  svchart::ChartParams _chp;

  QFile *_file = nullptr;
  
  void _addGraphToList(int graph_id, svgraph::GraphParams &p);
  
  svarduinomax::SvArduinoWidget *arduino;
  
  svtcp::SvTcpClient *_client;
  
signals:
  newState(bool state);
  
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
