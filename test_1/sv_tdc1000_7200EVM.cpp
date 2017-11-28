#include "sv_tdc1000_7200EVM.h"

extern SvSQLITE *SQLITE;
SvSelectTDC1000_7200EVMDevice *SELECTTDC1000_TDC7200EV_DEVICE;

SvTDC1000_7200EVM::SvTDC1000_7200EVM(svidev::DeviceInfo deviceInfo, QObject *parent)
{
  setDeviceInfo(deviceInfo);
  _port_info = QSerialPortInfo(_device_info.portName);
  
  setParent(parent);
}

SvTDC1000_7200EVM::~SvTDC1000_7200EVM()
{
  close();
  deleteLater();
}

bool SvTDC1000_7200EVM::open()
{

/*  
  if(_pull_thr)
    delete _pull_thr;

  _pull_thr = new SvPullTDC1000_7200EVM(_port_info, &mutex);
  
  if(!_pull_thr->open()) {
  
    setLastError(_pull_thr->lastError());
    
    delete _pull_thr;
    return false;
    
  }
  
//  _pull_thr->_serial->moveToThread(_pull_thr);
  */
  
  _serial_port = new SvSerialTDC1000_7200EVM(_port_info);
  
  if(!_serial_port->open()) {
    setLastError(_serial_port->lastError());
    return false;
  }
  
  _serial_thread = new QThread();
  _serial_port->moveToThread(_serial_thread);
  _serial_port->port.moveToThread(_serial_thread);
  connect(_serial_thread, &QThread::finished, this, &SvTDC1000_7200EVM::err);
  connect(_serial_thread, &QThread::finished, _serial_thread, &QThread::deleteLater);
  _serial_thread->start();
  
  qDebug() << 77;
  setOpened(true);
  qDebug() << 88;
  return true;
}

bool SvTDC1000_7200EVM::start(quint32 msecs)
{
  if(!_serial_port->start(msecs)) {
    setLastError(_serial_port->lastError()); 
    qDebug() << _serial_port->lastError();
    return false;
  }
  
  connect(_serial_port, &SvSerialTDC1000_7200EVM::new_data, this, &svidev::SvIDevice::new_data, Qt::QueuedConnection);
  
  return true;
  
  //  if(!_serial && !isOpened()) {
      
//  if(!_pull_thr) {
//      setLastError(QString("%1: device is not opened").arg(deviceInfo().deviceName));
//      return false;
//    }
  
//  _pull_thr->setTimeout(msecs);
  
//  _pull_thr->start();
  
  
}

void SvTDC1000_7200EVM::stop()
{
  
  _serial_port->stop();
//  _serial_thread->quit();
  
  delete _serial_port;
  delete _serial_thread;
  
/*
  if(_pull_thr)
    delete _pull_thr;
  
  _pull_thr = nullptr;
  */
}

void SvTDC1000_7200EVM::close()
{
  stop();
  
//  if(_serial->isOpen())
//    _serial->close();
  
//  delete _serial;
//  _serial = nullptr;
  
  setOpened(false);
}


bool SvTDC1000_7200EVM::addNewDevice()
{
  /* показываем форму выбора устройства USB */
  SvSelectTDC1000_7200EVMDevice *sdDlg = new SvSelectTDC1000_7200EVMDevice();
  if(sdDlg->exec() != QDialog::Accepted) {
    delete sdDlg;
    return false;
  }
  
  QSqlError err = SQLITE->execSQL(QString(SQL_NEW_DEVICE) 
                                  .arg(int(svidev::TDC1000_TDC7200EVM))
                                  .arg(sdDlg->dinfo.deviceName)
                                  .arg(-1)
                                  .arg(-1)
                                  .arg(-1)
                                  .arg(sdDlg->dinfo.portName)
                                  .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
                                  .arg(""));
  
  if(err.type() != QSqlError::NoError) {
    QMessageBox::critical(0, "Ошибка", QString("Не удалось добавить новое устройство:\n%1").arg(err.databaseText()), QMessageBox::Ok);
    return false;
  }
 
  delete sdDlg;
  
  return true;
}



/** ******************************* **/
SvSelectTDC1000_7200EVMDevice::SvSelectTDC1000_7200EVMDevice(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SvSelectDeviceDialog)
{
  
  ui->setupUi(this);
  
  on_bnUpdateDeviceList_clicked();
  
  connect(ui->bnOk, SIGNAL(pressed()), this, SLOT(accept()));
  connect(ui->bnCancel, SIGNAL(pressed()), this, SLOT(reject()));
  
  setModal(true);
  show();
    
}

void SvSelectTDC1000_7200EVMDevice::accept()
{
  dinfo.deviceName = ui->cbDevice->currentText();
  dinfo.portName = _devices.value(ui->cbDevice->currentIndex());
  
  QDialog::accept();
  
}

void SvSelectTDC1000_7200EVMDevice::on_bnUpdateDeviceList_clicked()
{
  QList<QSerialPortInfo> _available_devices = QSerialPortInfo::availablePorts();
    
  for(QSerialPortInfo port: _available_devices) {
    
    QString devdesc = QString("%1 %2 [%3]").arg(port.manufacturer()).arg(port.description()).arg(port.portName());

    ui->cbDevice->addItem(devdesc);
    _devices.insert(ui->cbDevice->count() - 1, port.portName());
  
  }
}

/** ******************************* **/



/** ----------------------------------------- **/
SvPullTDC1000_7200EVM::SvPullTDC1000_7200EVM(const QSerialPortInfo &portInfo, QMutex *mutex)
{
  _port_info = portInfo;
//  _timeout = timeout;
  _mutex = mutex;
}

bool SvPullTDC1000_7200EVM::open()
{
  _serial = new QSerialPort(_port_info.portName(), this);
  
  if (!_serial) {
    
    _last_error = QString("Ошибка при создании устройства.\n%1")
                            .arg(_serial->errorString());
    return false;
  }
  
  if(!_serial->open(QIODevice::ReadWrite)) {
    
    _last_error = QString("Ошибка при открытии порта %1.\n%2")
                      .arg(_port_info.portName()).arg(_serial->errorString());
    return false;
  }
  
  _serial->moveToThread(this);
  
  return true;
  
}

SvPullTDC1000_7200EVM::~SvPullTDC1000_7200EVM() 
{ 
  stop(); 
  
  if(_serial) {
    
    if(_serial->isOpen()) {
      _serial->close();
    }
    
    delete _serial;    
  }
  
  deleteLater(); 
}

void SvPullTDC1000_7200EVM::stop() 
{ 
  _started = false; 
  while(!_finished) QApplication::processEvents(); 
}

void SvPullTDC1000_7200EVM::run()
{
  _started = true;
  _finished = false;
  
  svidev::mdata_t measured_data;
  qres result;
  TDC1000_ANSWER tdc1000data;
  
  QByteArray params1 = QByteArray::fromHex("3032303230303030303030303030303030303030303030303030303030303030");
  QByteArray request1 = QByteArray::fromHex("3035303030303030303030303030303030303030303030303030303030303030");
  QByteArray params2 = QByteArray::fromHex("3032303230343030303030303030303030303030303030303030303030303030");
  QByteArray request2 = QByteArray::fromHex("3035303030303030303030303030303030303030303030303030303030303030");
  
  QByteArray answer = QByteArray();
  
  while(_started) {
    
    _mutex->lock();
    
    /** ******** читаем канал 1 ********* **/
    /* устанавливаем первый канал TX1 */
    result = _writeRead(params1, 1);
    
    if(result.result) {
    
      result = _writeRead(request1, 2);
      
      if(result.result) {
      
        /* фиксируем время t1 */
        memcpy(&tdc1000data, result.answer.data(), sizeof(TDC1000_ANSWER));
    
        /** ЗДЕСЬ ДОЛЖНА БЫТЬ ФОРМУЛА */
        qreal calCount = qreal(tdc1000data.calibr2 - tdc1000data.calibr1) / qreal(TDC_CALIBRATION2_PERIODS - 1);
        qreal normLSB = (1.0 / qreal(TDC_CLOCK)) / calCount;
                   
        measured_data.tof1 = normLSB * qreal(tdc1000data.time1 - tdc1000data.time2) + qreal(tdc1000data.clc1) * (1.0 / qreal(TDC_CLOCK)); 
        /** ****************** ********/
    
    
        /** ******** читаем канал 2 ********* **/
        /* устанавливаем второй канал TX2 */
        result = _writeRead(params2, 1);
        
        if(result.result) {
    
          result = _writeRead(request2, 2);
          
          if(result.result) {
      
            /* фиксируем время t2 */
            memcpy(&tdc1000data, result.answer.data(), sizeof(TDC1000_ANSWER));
            
            /** ЗДЕСЬ ДОЛЖНА БЫТЬ ФОРМУЛА */
            calCount = qreal(tdc1000data.calibr2 - tdc1000data.calibr1) / qreal(TDC_CALIBRATION2_PERIODS - 1);
            normLSB = (1.0 / qreal(TDC_CLOCK)) / calCount;
                       
            measured_data.tof2 = normLSB * qreal(tdc1000data.time1 - tdc1000data.time2) + qreal(tdc1000data.clc1) * (1.0 / qreal(TDC_CLOCK)); 
            /** ****************** ********/
            
          }
        }
      }
    }
    
    _mutex->unlock();
//    qDebug() << tdc1000data.time1;
//    qDebug() << tdc1000data.time2;
    
    if(result.result) 
      emit new_data(measured_data); 
    
    msleep(_timeout);
    
  }
  
  _finished = true;
  
}

qres SvPullTDC1000_7200EVM::_writeRead(const QByteArray& request, int answer_count)
{
  qres res;
  res.result = true;
  
//  qDebug() << 1;
  _serial->write(request);
  
  if(!_serial->waitForBytesWritten(1000)) {
    
    res.result = false;
    res.msg = QString("Не удалось записать данные порт.\n%1").arg(_serial->errorString());
    return res;
  }
  
  /* пакет ответа */
  for(int i = 0; i < answer_count; i++) {
    
    if(!_serial->waitForReadyRead(1000)) {
      
      res.result = false;
      res.msg = "Таймаут. Ответ не получен.";
      break;
    }
    
    res.answer.clear();
    res.answer = _serial->readAll();
    
//    if(res.answer.size() != sizeof(TDC1000_ANSWER)) {
      
//      res.result = false;
//      res.msg = QString("Неверный ответ. Получено %1 байт. Ожидалось %2.")
//                   .arg(res.answer.size())
//                   .arg(sizeof(TDC1000_ANSWER));
//    }
  }
  
  return res;
  
}


/** **************************************** **/


bool SvSerialTDC1000_7200EVM::open()
{
  
  if(!port.open(QIODevice::ReadWrite)) {
    
    _last_error = QString("Ошибка при открытии порта %1.\n%2")
                      .arg(_port_info.portName()).arg(port.errorString());
    
    emit error(_last_error);
    
    return false;
  }
  
  
  return true;
  
}

void SvSerialTDC1000_7200EVM::close()
{
  stop();
}

bool SvSerialTDC1000_7200EVM::start(quint32 msecs)
{
  _timer.setInterval(msecs);
  connect(&_timer, &QTimer::timeout, this, &SvSerialTDC1000_7200EVM::write_data);
//  connect(port, &QSerialPort::error(QSerialPort::SerialPortError), this, &SvSerialTDC1000_7200EVM::error(QSerialPort::SerialPortError));
  _timer.start();
  return true;
}

void SvSerialTDC1000_7200EVM::stop()
{
  if(port.isOpen())
    port.close();
  
  _timer.stop();
}

void SvSerialTDC1000_7200EVM::write_data()
{
  svidev::mdata_t measured_data;
  TDC1000_ANSWER tdc1000data1, tdc1000data2;
  
  qreal calCount = 0;
  qreal normLSB = 0;
  
  quint16 calibr2;
  quint16 calibr1;
  quint16 time1;
  quint16 time2;
  quint16 clc1;
  
  QByteArray params1 = QByteArray::fromHex("3032303230303030303030303030303030303030303030303030303030303030");
  QByteArray request1 = QByteArray::fromHex("3035303030303030303030303030303030303030303030303030303030303030");
  QByteArray params2 = QByteArray::fromHex("3032303230343030303030303030303030303030303030303030303030303030");
  QByteArray request2 = QByteArray::fromHex("3035303030303030303030303030303030303030303030303030303030303030");
  QByteArray b;
  
  /// первый канал
  port.write(params1);
  if(port.waitForReadyRead(200)) {
    
    b = port.readAll();
    
    port.write(request1);
    if(port.waitForReadyRead(200)) {
      
      b = port.readAll();
      
      if(port.waitForReadyRead(200)) {
        
        b = port.readAll(); 
        memcpy(&tdc1000data1, b.data(), sizeof(TDC1000_ANSWER));
        
        calibr2 = qFromBigEndian<quint16>(tdc1000data1.calibr2);
        calibr1 = qFromBigEndian<quint16>(tdc1000data1.calibr1);
        time1 = qFromBigEndian<quint16>(tdc1000data1.time1);
        time2 = qFromBigEndian<quint16>(tdc1000data1.time2);
        clc1 = qFromBigEndian<quint16>(tdc1000data1.clc1);
        
        calCount = (calibr2 - calibr1) / qreal(TDC_CALIBRATION2_PERIODS - 1);
        normLSB = (1.0 / qreal(TDC_CLOCK)) / calCount;
        
//        qDebug() << b.toHex();           
        measured_data.tof1 = 1000000000 * (normLSB * (time1 - time2) + clc1 * (1.0 / qreal(TDC_CLOCK))); 
//        qDebug() << calibr2 << calibr1 << calCount << normLSB << clc1 << measured_data.tof1;
    
        
        /// второй канал
        port.write(params2);
        if(port.waitForReadyRead(200)) {
          
          b = port.readAll();
          
          port.write(request2);
          if(port.waitForReadyRead(200)) {
            
            b = port.readAll();
            if(port.waitForReadyRead(200)) {

              b = port.readAll();
              memcpy(&tdc1000data2, b.data(), sizeof(TDC1000_ANSWER));
              
              calibr2 = qFromBigEndian<quint16>(tdc1000data2.calibr2);
              calibr1 = qFromBigEndian<quint16>(tdc1000data2.calibr1);
              time1 = qFromBigEndian<quint16>(tdc1000data2.time1);
              time2 = qFromBigEndian<quint16>(tdc1000data2.time2);
              clc1 = qFromBigEndian<quint16>(tdc1000data2.clc1);
              
              calCount = (calibr2 - calibr1) / qreal(TDC_CALIBRATION2_PERIODS - 1);
              normLSB = (1.0 / qreal(TDC_CLOCK)) / calCount;
              
              measured_data.tof2 = 1000000000 *(normLSB * (time1 - time2) + clc1 * (1.0 / qreal(TDC_CLOCK)));
//              qDebug() << measured_data.tof1 << measured_data.tof2;
              emit new_data(measured_data);
              
            }
          }
        }
      }
    }
  }
}

