#include "sv_tdc1000_7200EVM.h"

extern SvSQLITE *SQLITE;
SvSelectMAX35101EVDevice *SELECTMAX35101EV_DEVICE;

SvTDC1000_7200EVM::SvTDC1000_7200EVM(svidev::DeviceInfo deviceInfo, QObject *parent)
{
  setDeviceInfo(deviceInfo);
  setParent(parent);
}

SvTDC1000_7200EVM::~SvTDC1000_7200EVM()
{
  close();
  deleteLater();
}

bool SvTDC1000_7200EVM::open()
{
  _serial = new QSerialPort(_device_info.portName);
  
  if (!_serial) {
    
    setLastError(QString("Ошибка при создании устройства.\n%1")
                 .arg(_serial->errorString()));
    return false;
  }
  
  if(!_serial->open(QIODevice::ReadWrite)) {
    
    setLastError(QString("Ошибка при открытии порта %1.\n%2")
                 .arg(_device_info.portName).arg(_serial->errorString()));
    return false;
  }
  
  setOpened(true);
  
  return true;
}

void SvTDC1000_7200EVM::close()
{
  stop();
  
  if(_serial->isOpen())
    _serial->close();
  
  delete _serial;
  _serial = nullptr;
  
  setOpened(false);
}

bool SvTDC1000_7200EVM::start(quint32 msecs)
{
  if(!_serial && !isOpened()) {
    
    setLastError(QString("%1: device is not opened").arg(deviceInfo().deviceName));
    return false;
  }
  
  if(_pull_thr)
    delete _pull_thr;
  
  _pull_thr = new SvPullTDC1000_7200EVM(_serial, msecs, &mutex);
  connect(_pull_thr, &SvPullTDC1000_7200EVM::new_data, this, &svidev::SvIDevice::new_data, Qt::QueuedConnection);
  _pull_thr->start();
  
  
}

bool SvTDC1000_7200EVM::stop()
{
  if(_pull_thr)
    delete _pull_thr;
  
  _pull_thr = nullptr;
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
                                  .arg(int(svidev::MAX35101EV))
                                  .arg(sdDlg->dinfo.deviceName)
                                  .arg(sdDlg->dinfo.idVendor)
                                  .arg(sdDlg->dinfo.idProduct)
                                  .arg(-1)
                                  .arg("")
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
  dinfo.idVendor = _devices.value (ui->cbDevice->currentIndex()).first;
  dinfo.idProduct = _devices.value(ui->cbDevice->currentIndex()).second;
  
  QDialog::accept();
  
}

void SvSelectTDC1000_7200EVMDevice::on_bnUpdateDeviceList_clicked()
{
  QList<QSerialPortInfo> _available_devices = QSerialPortInfo::availablePorts();
    
  for(QSerialPortInfo port: _available_devices) {
    
    QString devdesc = QString("%1 %2 [%2]").arg(port.manufacturer()).arg(porn.description()).arg(port.portName());
    ui->cbDevice->addItem(devdesc);
    
    
//    QString devdesc = QString("%1 %2 [VID:%3 PID:%4]")
//                      .arg(sManufacturer)
//                      .arg(sProduct)
//                      .arg(desc.idVendor, 0, 16)
//                      .arg(desc.idProduct, 0, 16);
    
    _devices.insert(ui->cbDevice->count() - 1, devdesc);
  
  }
}

/** ******************************* **/


qres pullTDC1000(QSerialPort *serial)
{
  TDC1000::qres result;
  TDC1000::TDC1000_ANSWER tdc1000data;
  
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
  
  result = TDC1000::writeReadTDC1000(serial, request, 1);
  
  if(!result.result) return result;
  
  /* TOF_ONE_SHOT */
  request = QByteArray::fromHex("3035303030303030303030303030303030303030303030303030303030303030");
  
  result = TDC1000::writeReadTDC1000(serial, request, 2);
  
  if(!result.result) return result;
  
  if(result.answer.size() != sizeof(TDC1000::TDC1000_ANSWER)) {
    result.result = false;
    result.msg = QString("Неверный ответ. Получено %1 байт. Ожидалось %2.")
                 .arg(answer.size())
                 .arg(sizeof(TDC1000::TDC1000_ANSWER));
    return result;
  }
    
  /* фиксируем время t1 */
  memcpy(&tdc1000data, result.answer.data(), sizeof(TDC1000::TDC1000_ANSWER));
  
  /** ЗДЕСЬ ДОЛЖНА БЫТЬ ФОРМУЛА */
  result.t1 = tdc1000data.time1; 
  /** ****************** ********/
  
  
  /** ******** читаем канал 2 ********* **/
  /* устанавливаем второй канал TX2 */
  request = QByteArray::fromHex("3032303230343030303030303030303030303030303030303030303030303030");
  
  result = TDC1000::writeReadTDC1000(serial, request, 1);
  
  if(!result.result) return result;
  
  /* TOF_ONE_SHOT */
  request = QByteArray::fromHex("3035303030303030303030303030303030303030303030303030303030303030");
  
  result = TDC1000::writeReadTDC1000(serial, request, 2);
  
  if(!result.result) return result;
  
  if(result.answer.size() != sizeof(TDC1000::TDC1000_ANSWER)) {
    result.result = false;
    result.msg = QString("Неверный ответ. Получено %1 байт. Ожидалось %2.")
                 .arg(answer.size())
                 .arg(sizeof(TDC1000::TDC1000_ANSWER));
    return result;
  }
    
  /* фиксируем время t2 */
  memcpy(&tdc1000data, result.answer.data(), sizeof(TDC1000::TDC1000_ANSWER));
  
  /** ЗДЕСЬ ДОЛЖНА БЫТЬ ФОРМУЛА */
  result.t2 = tdc1000data.time2; 
  /** ****************** ********/
  
  
  
  /** ******************************* **/
//  result.t1 = qFromBigEndian<qint16>(_max_data.time1); // / 262.14;   // время пролета 1, в нс.
//  result.t2 = qFromBigEndian<qint16>(_max_data.time2); // / 262.14; // время пролета 2, в нс.
  
  
  return result;
}



/** ----------------------------------------- **/
SvPullTDC1000_7200EVM::SvPullTDC1000_7200EVM(QSerialPort *serial, quint32 timeout)
{
  _serial = serial;
  _timeout = timeout;
}

SvPullTDC1000_7200EVM::~SvPullTDC1000_7200EVM() 
{ 
  stop(); 
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
    
    /** ******** читаем канал 1 ********* **/
    /* устанавливаем первый канал TX1 */
    result = _writeRead(params1, 1);
    
    if(result.result) {
    
      result = _writeRead(request1, 2);
    
      if(result.result) {
      
        /* фиксируем время t1 */
        memcpy(&tdc1000data, result.answer.data(), sizeof(TDC1000_ANSWER));
    
        /** ЗДЕСЬ ДОЛЖНА БЫТЬ ФОРМУЛА */
        result.t1 = tdc1000data.time1; 
        /** ****************** ********/
    
    
    /** ******** читаем канал 2 ********* **/
    /* устанавливаем второй канал TX2 */
        result = _writeRead(request, 1);
    
        if(result.result) {
    
          result = _writeRead(request, 2);
    
          if(result.result) {
      
            /* фиксируем время t2 */
            memcpy(&tdc1000data, result.answer.data(), sizeof(TDC1000_ANSWER));
            
            /** ЗДЕСЬ ДОЛЖНА БЫТЬ ФОРМУЛА */
            qreal calCount = (tdc1000data.calibr2 - tdc1000data.calibr1) / (tdc1000data.);
            
            result.t2 = tdc1000data.time2; 
            /** ****************** ********/
            
          }
        }
      }
    }
    

    
    if(result.result)
      emit new_data(result.answer); 
    
    msleep(_timeout);
    
  }
  
  _finished = true;
  
}

qres SvPullTDC1000_7200EVM::_writeRead(QByteArray& request, int answer_count)
{
  qres res;
  
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
    
    if(res.answer.size() != sizeof(TDC1000_ANSWER)) {
      
      res.result = false;
      res.msg = QString("Неверный ответ. Получено %1 байт. Ожидалось %2.")
                   .arg(res.answer.size())
                   .arg(sizeof(TDC1000_ANSWER));
    }
  }
  
  return res;
  
}

