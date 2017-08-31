#include "sv_tdc100thread.h"

qres pullTDC1000(QSerialPort *serial)
{
  qres result;
  TDC1000_ANSWER tdc1000data;
  
  if (!serial) {
    result.result = false;
    result.msg = "Ошибка устройства.";
    return result;
  }
  
  if(!serial->isOpen()) {
    if(!serial->open(QIODevice::ReadWrite)) {
      result.result = false;
      result.msg = QString("Не удалось открыть порт %1").arg(serial->portName());
      return result;
    }
  }
  
  QByteArray request = QByteArray();
  QByteArray answer = QByteArray();
  
  /** ******** читаем канал 1 ********* **/
  /* устанавливаем первый канал TX1 */
  request = QByteArray::fromHex("3032303230303030303030303030303030303030303030303030303030303030");
  
  result = writeReadTDC1000(serial, request, 1);
  
  if(!result.result) return result;
  
  /* TOF_ONE_SHOT */
  request = QByteArray::fromHex("3035303030303030303030303030303030303030303030303030303030303030");
  
  result = writeReadTDC1000(serial, request, 2);
  
  if(!result.result) return result;
  
  if(result.answer.size() != sizeof(TDC1000_ANSWER)) {
    result.result = false;
    result.msg = QString("Неверный ответ. Получено %1 байт. Ожидалось %2.")
                 .arg(answer.size())
                 .arg(sizeof(TDC1000_ANSWER));
    return result;
  }
    
  /* фиксируем время t1 */
  memcpy(&tdc1000data, result.answer.data(), sizeof(TDC1000_ANSWER));
  
  /** ЗДЕСЬ ДОЛЖНА БЫТЬ ФОРМУЛА */
  result.t1 = tdc1000data.time1; 
  /** ****************** ********/
  
  
  /** ******** читаем канал 2 ********* **/
  /* устанавливаем второй канал TX2 */
  request = QByteArray::fromHex("3032303230343030303030303030303030303030303030303030303030303030");
  
  result = writeReadTDC1000(serial, request, 1);
  
  if(!result.result) return result;
  
  /* TOF_ONE_SHOT */
  request = QByteArray::fromHex("3035303030303030303030303030303030303030303030303030303030303030");
  
  result = writeReadTDC1000(serial, request, 2);
  
  if(!result.result) return result;
  
  if(result.answer.size() != sizeof(TDC1000_ANSWER)) {
    result.result = false;
    result.msg = QString("Неверный ответ. Получено %1 байт. Ожидалось %2.")
                 .arg(answer.size())
                 .arg(sizeof(TDC1000_ANSWER));
    return result;
  }
    
  /* фиксируем время t2 */
  memcpy(&tdc1000data, result.answer.data(), sizeof(TDC1000_ANSWER));
  
  /** ЗДЕСЬ ДОЛЖНА БЫТЬ ФОРМУЛА */
  result.t2 = tdc1000data.time2; 
  /** ****************** ********/
  
  
  
  /** ******************************* **/
  result.t1 = qFromBigEndian<qint16>(_max_data.time1); // / 262.14;   // время пролета 1, в нс.
  result.t2 = qFromBigEndian<qint16>(_max_data.time2); // / 262.14; // время пролета 2, в нс.
  
  
  return result;
}

qres writeReadTDC1000(QSerialPort *serial, QByteArray request, int answer_count)
{
  qres res;
  
  serial->write(request);
  
  if(!serial->waitForBytesWritten(1000)) {
    res.result = false;
    res.msg = QString("Не удалось записать данные порт. %1").arg(serial->errorString());
    return res;
  }
  
  /* пакет ответа */
  for(int i = 0; i < answer_count; i++) {
    
    if(!serial->waitForReadyRead(1000)) {
      res.result = false;
      res.msg = "Таймаут. Ответ не получен.";
      break;
    }
    
    res.answer.clear();
    res.answer = serial->readAll();
    
    if(res.answer.size() != sizeof(TDC1000_ANSWER)) {
      res.result = false;
      res.msg = QString("Неверный ответ. Получено %1 байт. Ожидалось %2.")
                   .arg(res.answer.size())
                   .arg(sizeof(TDC1000_ANSWER));
    }
  }
  
  return res;
  
}


SvTDC1000Thread::SvTDC1000Thread(const QSerialPortInfo &serialinfo)
{
  _current_serial_info = serialinfo;
}

void SvTDC1000Thread::run() 
{
  _lastError = "";
  
  _serial = new QSerialPort(_current_serial_info);
  
  if (!_serial) {
    _lastError = "Ошибка при создании устройства.";
    return;
  }
  
  if(!_serial->open(QIODevice::ReadWrite)) {
    _lastError = QString("Не удалось открыть порт %1").arg(_serial->portName());
    return;
  }
  
  _started = true;
  
  while(_started) {
  
    /** ******** читаем канал 1 ********* **/
    /* отправляем настройки */
    QByteArray request = QByteArray::fromHex(_ch_setup.at(0));
    
    _serial->write(request);
    
    if(!_serial->waitForBytesWritten(1000)) {
      _lastError = QString("Не удалось записать данные порт.\n%1").arg(_serial->errorString());
      _started = false;
      continue;
    }
    
    /** ждем ответа **/
    if(!_serial->waitForReadyRead(1000)) {
      _lastError = QString("Таймаут. Ответ не получен.");
      _started = false;
      continue;
    }
    
    QByteArray answer = QByteArray();
    answer = _serial->readAll();
    
    if(request != answer) {
      _lastError = QString("Неверный ответ на запрос.");
      _started = false;
      continue;
    }
    
    /** ******** читаем канал 2 ********* **/
    /* отправляем настройки */
    QByteArray request = QByteArray::fromHex(_ch_setup.at(0));
    
    _serial->write(request);
    
    if(!_serial->waitForBytesWritten(1000)) {
      _lastError = QString("Не удалось записать данные порт.\n%1").arg(_serial->errorString());
      _started = false;
      continue;
    }
    
    /** ждем ответа **/
    if(!_serial->waitForReadyRead(1000)) {
      _lastError = QString("Таймаут. Ответ не получен.");
      _started = false;
      continue;
    }
    
    QByteArray answer = QByteArray();
    answer = _serial->readAll();
    
    if(request != answer) {
      _lastError = QString("Неверный ответ на запрос.");
      _started = false;
      continue;
    }
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
  _channel ^= 1;
  
  emit newData(answer);
}
