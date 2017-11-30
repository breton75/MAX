#include "sv_tdc1000_7200EVM.h"

extern SvSQLITE *SQLITE;
SvSelectTDC1000_7200EVMDevice *SELECTTDC1000_TDC7200EV_DEVICE;

SvTDC1000_7200EVM::SvTDC1000_7200EVM(svidev::DeviceInfo deviceInfo, QObject *parent)
{
  setDeviceInfo(deviceInfo);
  _port_info = QSerialPortInfo(_device_info.portName);
  _port.setPort(_port_info);
  
  setParent(parent);
}

SvTDC1000_7200EVM::~SvTDC1000_7200EVM()
{
  close();
  deleteLater();
}

bool SvTDC1000_7200EVM::open()
{
  if(!_port.open(QIODevice::ReadWrite)) {
    
    setLastError(_port.errorString());
    return false;
  }
  setOpened(true);
  
  return true;
}

bool SvTDC1000_7200EVM::start(quint32 msecs)
{

  connect(&_port, &QSerialPort::readyRead, this, &SvTDC1000_7200EVM::read_data);
  
  write_params(TDC_PARAMS_SETUP);
  write_params(TDC_PARAMS_TDC1000);
  write_params(TDC_PARAMS_TDC7200);
//  write_params(TDC_PARAMS_CONT_TRIGG);
  write_params(TDC_CMD_START);
  
  return true;
  
}

void SvTDC1000_7200EVM::stop()
{
  
  if(_port.isOpen()) {

    write_params(TDC_CMD_STOP);
  }
  
  disconnect(&_port, &QSerialPort::readyRead, this, &SvTDC1000_7200EVM::read_data);

}

void SvTDC1000_7200EVM::close()
{  
  if(_port.isOpen())
    _port.close();
  
  setOpened(false);
}

void SvTDC1000_7200EVM::write_params(const QByteArray &params)
{
  ParamsWritten = false;
  _port.write(params);
  while(!ParamsWritten) qApp->processEvents();
}

void SvTDC1000_7200EVM::read_data()
{
  
  QByteArray b = _port.readAll();
  
  if(b.size() == TDC_PARAMS_ANSWER_SIZE) {
//    qDebug() << b.at(1);
    ParamsWritten = true;
    return;
  }
  else if((b.at(1) != 0x08) && (b.size() != sizeof(TDC1000_ANSWER)))
    return;
  
  
  qreal calCount = 0;
  qreal normLSB = 0;
  
  quint16 calibr2;
  quint16 calibr1;
  quint16 time1;
  quint16 time2;
  quint16 clc1;
  
  TDC1000_ANSWER tdc1000data;
  memcpy(&tdc1000data, b.data(), sizeof(TDC1000_ANSWER));
  
  calibr2 = qFromBigEndian<quint16>(tdc1000data.calibr2);
  calibr1 = qFromBigEndian<quint16>(tdc1000data.calibr1);
  time1 = qFromBigEndian<quint16>(tdc1000data.time1);
  time2 = qFromBigEndian<quint16>(tdc1000data.time2);
  clc1 = qFromBigEndian<quint16>(tdc1000data.clc1);
//  qDebug() << time1 << time2 << clc1 << calibr1 << calibr2;
  calCount = (calibr2 - calibr1) / qreal(TDC_CALIBRATION2_PERIODS - 1);
  normLSB = (1.0 / qreal(TDC_CLOCK)) / calCount;
  
  switch (_ch_num % 2) {
    
    case 0:
      
      last_data.tof2 = 1000000000 * (normLSB * (time1 - time2) + clc1 * (1.0 / qreal(TDC_CLOCK)));
      emit new_data(last_data);
      
      break;
      
    default:
      last_data.tof1 = 1000000000 * (normLSB * (time1 - time2) + clc1 * (1.0 / qreal(TDC_CLOCK)));
      break;
  }
//  qDebug() << _ch_num;
  _ch_num = 1 + (_ch_num % 2);
              
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



