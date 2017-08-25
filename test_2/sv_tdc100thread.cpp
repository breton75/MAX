#include "sv_tdc100thread.h"

SvTDC1000Thread::SvTDC1000Thread(const QSerialPortInfo &serialinfo)
{
  _current_serial_info = serialinfo;
}

bool SvTDC1000Thread::start() 
{
  _lastError = "";
  
  _serial = new QSerialPort(_current_serial_info);
  
  if (!_serial) {
    _lastError = "Ошибка при создании устройства.";
    return false;
  }
  
  if(!_serial->open(QIODevice::ReadWrite)) {
    _lastError = QString("Не удалось открыть порт %1").arg(_serial->portName());
    return false;
  }
  
  QByteArray request = QByteArray::fromHex("30363030303030303030303030303030"
                                           "30303030303030303030303030303030");
  _serial->write(request);
  
  if(!_serial->waitForBytesWritten(1000)) {
    _lastError = QString("Не удалось записать данные порт.\n%1").arg(_serial->errorString());
    return false;
  }
  
//  connect(this, &QThread::finished, _serial, &QObject::deleteLater);
  connect(_serial, SIGNAL(readyRead()), this, SLOT(read()));
  
  return true;
}

bool SvTDC1000Thread::stop() 
{
  _lastError = "";
  
  if (!_serial) {
    _lastError = "Ошибка при создании устройства.";
    return false;
  }
  
  if(!_serial->isOpen()) {
    _lastError = QString("Порт закрыт %1").arg(_serial->portName());
    return false;
  }
  
  QByteArray request = QByteArray::fromHex("30373030303030303030303030303030"
                                           "30303030303030303030303030303030");
  _serial->write(request);
  
  if(!_serial->waitForBytesWritten(1000)) {
    _lastError = QString("Не удалось записать данные порт.\n%1").arg(_serial->errorString());
    return false;
  }
  
  _serial->close();
  delete _serial;
  _serial = nullptr;
  
  return true;
  
}

void SvTDC1000Thread::read()
{
  QByteArray answer = _serial->readAll();
  
  emit newData(answer);
}
