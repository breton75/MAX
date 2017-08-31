#ifndef SV_TDC1000TTHREAD_H
#define SV_TDC1000TTHREAD_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>
#include <QtEndian>

namespace TDC1000 {

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
  
  qres pullTDC1000(QSerialPort *serial);
  QByteArray writeReadTDC1000(QSerialPort *serial, QByteArray data, int answer_count);
  
}

class SvTDC1000Thread : public QThread
{
  Q_OBJECT
  
public:
  SvTDC1000Thread(const QSerialPortInfo &serialinfo);
  
//  bool start();
  bool stop();
  
  QString lastError() { return _lastError; }
  
private:
  QSerialPort *_serial = nullptr;
  QString _lastError = "";
  
  QSerialPortInfo _current_serial_info;
  
  quint32 _channel = 0;
  
  QStringList _ch_setup = {"3032303230303030303030303030303030303030303030303030303030303030",
                              "3032303230343030303030303030303030303030303030303030303030303030"};
  
  bool _started;
  bool _finished;
  
protected:
  void run() Q_DECL_OVERRIDE;
  
signals:
  void newData(QByteArray &data);
  
private slots:
  void read();
  
  
};

#endif // SV_TDC1000TTHREAD_H
