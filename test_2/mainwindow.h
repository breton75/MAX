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
#include <QIcon>
#include <QSerialPort>
#include <QSerialPortInfo>

#include <QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

#include "libusb.h"
#include "sv_chartwidget.h"
#include "sv_graph.h"
#include "sv_arduino_max.h"
#include "sv_tdc100.h"

#include "../../svlib/sv_settings.h"
#include "../../svlib/sv_log.h"
#include "../../svlib/sv_tcpserverclient.h"

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

class SvPullTDC1000;

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
  QString FILE_EXT = "frs";
  
public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  
//  libusb_device_handle* handle;
//  QTimer tm;
  
  svlog::SvLog log;
  
private slots:
  void on_bnGetDeviceList_clicked();
  void on_bnOneShot_clicked();
  void on_bnCycle_clicked();
  
  void new_data(QByteArray &data);
  
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
  
  QSerialPort *_serial = nullptr;
//  QMap<int, QPair<uint16_t, uint16_t>> _devices;
  QList<QSerialPortInfo> _available_devices;
  
  SvPullTDC1000 *_tdc100thr = nullptr;
  int _timerId;
  
//  pullusb::MAX35101EV_ANSWER _max_data;
  TDC1000::TDC1000_ANSWER _tdc1000_data;
  
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


class SvPullTDC1000: public QThread
{
    Q_OBJECT
  
public:
  explicit SvPullTDC1000(QSerialPort *serial, quint32 timeout)
  {
    _serial = serial;
    _timeout = timeout;
  }
  
  ~SvPullTDC1000() 
  { 
    stop(); 
    deleteLater(); 
  }

  void stop() 
  { 
    _started = false; 
    while(!_finished) QApplication::processEvents(); 
  }
  
protected:
//  void timerEvent(QTimerEvent *te);
  
private:
  void run() Q_DECL_OVERRIDE;
  
  bool _started;
  bool _finished;
  quint32 _timeout;
  QSerialPort *_serial;
  
signals:
  void new_data(QByteArray data);
  
};

#endif // MAINWINDOW_H
