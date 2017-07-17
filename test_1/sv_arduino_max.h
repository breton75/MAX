#ifndef SV_ARDUINO_MAX_H
#define SV_ARDUINO_MAX_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QException>

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace Ui {
class SvArduinoWidgetUi;
}

#include "../../svlib/sv_log.h"
#include "../../svlib/sv_tcpserverclient.h"

namespace svarduinomax {
  
  struct SvArduinoWidgetParams {
    QString ip = "192.168.44.44";
    quint32 port = 35580;
    bool spin_clockwise = true;
    quint32 engine_pw = 100;
    bool turn_angle_enable = false;
    quint32 turn_angle = 180;
    bool turn_count_enable = false;
    quint32 turn_count = 1;
    qreal current_voltage = 12;
    bool temperature_period_enable = true;
    quint32 temperature_period = 1;
  };
  
  class SvArduinoWidget;
      
}

class SvException: public QException
{
public:
    void raise(QString error) { err = error; throw *this; }
    SvException *clone() const { return new SvException(*this); }

    QString err;

};

class svarduinomax::SvArduinoWidget : public QWidget
{
  Q_OBJECT
public:
  explicit SvArduinoWidget(svarduinomax::SvArduinoWidgetParams params,
                           QTextEdit *logWidget = nullptr,
                           QWidget *parent = nullptr);
  
  void setLog(svlog::SvLog &log) { _log = log; }
  
  qreal currentTemperature() { return _temperature; }
  quint32 currentTurnCount() { return _turn_count; }
  
  svarduinomax::SvArduinoWidgetParams params() { return _params; }
  void setParams(svarduinomax::SvArduinoWidgetParams params) { _params = params; }
  
private:
  Ui::SvArduinoWidgetUi *ui;
  
  svlog::SvLog _log;
  svtcp::SvTcpClient *_client;
  
  QString _ip;
  quint32 _port;
  bool _spin_clockwise;
  quint32 _spin_speed;
  quint32 _turn_angle;
  quint32 _turn_count;
  qreal _temperature;  
  qreal _voltage;
  
  bool _started = false;
  
  QTimer _temperature_timer;
  
  svarduinomax::SvArduinoWidgetParams _params;
  
  SvException _exception;
  
  /** контролы **/
//  void setupUi();
  
//  QVBoxLayout *verticalLayout_7;
//  QGroupBox *gbMain;
//  QVBoxLayout *verticalLayout;
//  QHBoxLayout *horizontalLayout_5;
//  QGroupBox *gbNetworkParams;
//  QVBoxLayout *verticalLayout_4;
//  QHBoxLayout *horizontalLayout_2;
//  QLabel *lblIp;
//  QLineEdit *editIp;
//  QHBoxLayout *horizontalLayout_3;
//  QLabel *lblPort;
//  QSpinBox *spinPort;
//  QPushButton *bnStartStop;
//  QGroupBox *gbSpinDirection;
//  QVBoxLayout *verticalLayout_2;
//  QRadioButton *rbClockwise;
//  QRadioButton *rbContraClockwise;
//  QGroupBox *gbSpinSpeed;
//  QVBoxLayout *verticalLayout_3;
//  QSlider *sliderSpinSpeed;
//  QGroupBox *gbTurnAngle;
//  QVBoxLayout *verticalLayout_5;
//  QSpinBox *spinTurnAngle;
//  QGroupBox *gbTurnCount;
//  QVBoxLayout *verticalLayout_6;
//  QSpinBox *spinTurnCount;
//  QGroupBox *gbTemperature;
//  QVBoxLayout *verticalLayout_9;
//  QSpinBox *spinTemperaturePeriod;
//  QHBoxLayout *horizontalLayout_4;
//  QLabel *lblVoltage;
//  QSpinBox *spinVoltage;
//  QHBoxLayout *horizontalLayout;
//  QPushButton *bnSendCmd;
//  QLineEdit *editCmd;
//  QSpacerItem *horizontalSpacer;
//  QPushButton *bnApply;
//  QTextEdit *textLog;
  
public slots:
  bool start();
  bool stop();
  
private slots:
  void on_bnStart_clicked();
  void on_bnStop_clicked();
  void on_rbClockwise_clicked(bool checked);
  void on_rbContraClockwise_clicked(bool checked);
  void on_sliderEnginePw_valueChanged(int value);
//  void on_gbTurnAngle_clicked(bool checked);
  void on_spinTurnAngle_valueChanged(int arg1);
  void on_gbTurnCount_clicked(bool checked);
  void on_spinTurnCount_valueChanged(int arg1);
  void on_gbTemperature_clicked(bool checked);
  void on_spinTemperaturePeriod_valueChanged(int arg1);
  void on_bnSendCmd_clicked();
  void on_editCmd_returnPressed();
  void on_bnApply_clicked();
  void on_editIp_textChanged(const QString &arg1);
  void on_spinPort_valueChanged(int arg1);
  
  void stateChanged(bool started);
  
  void on_editCmd_editingFinished();
  
  void on_gbTurnAngle_toggled(bool arg1);
  
signals:
  void newState(bool started);
//  void stopped();
  void temperature(qreal t);
  
public slots:
};

#endif // SV_ARDUINO_MAX_H
