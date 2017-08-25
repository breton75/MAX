#ifndef SV_TDC1000TTHREAD_H
#define SV_TDC1000TTHREAD_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>

class SvTDC1000Thread : public QThread
{
  Q_OBJECT
  
public:
  SvTDC1000Thread(const QSerialPortInfo &serialinfo);
  
  bool start();
  bool stop();
  
  QString lastError() { return _lastError; }
  
private:
  QSerialPort *_serial = nullptr;
  QString _lastError = "";
  
  QSerialPortInfo _current_serial_info;
  
signals:
  void newData(QByteArray &data);
  
private slots:
  void read();
  
  
};

#endif // SV_TDC1000TTHREAD_H
