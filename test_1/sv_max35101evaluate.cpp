#include "sv_max35101evaluate.h"

extern SvSQLITE *SQLITE;
SvSelectMAX35101EVDevice *SELECTMAX35101EV_DEVICE;

SvMAX35101EV::SvMAX35101EV(svidev::DeviceInfo deviceInfo, QObject *parent)
{
  setDeviceInfo(deviceInfo);
  setParent(parent);
}

SvMAX35101EV::~SvMAX35101EV()
{
  close();
  deleteLater();
}

bool SvMAX35101EV::open()
{
  libusb_context *ctx = NULL;  
  libusb_init(NULL);  // инициализируем библиотеку
  
  // открываем устройство
  _handle = libusb_open_device_with_vid_pid(ctx, _device_info.idVendor, _device_info.idProduct);
    
  if (!_handle) {
    libusb_exit(NULL);
    setLastError(QString("%1: libusb_open_device_with_vid_pid error. (idVendor = %2, idProduct = %3")
                  .arg(deviceInfo().deviceName).arg(deviceInfo().idVendor).arg(deviceInfo().idProduct));
    return false;
  }
  
  // запрашиваем интерфейс 0 для себя
  int err = libusb_claim_interface(_handle, 0); 
  if(err < 0) { 
    setLastError(QString("%1: libusb_claim_interface error: %2").arg(deviceInfo().deviceName).arg(err));
    return false;
  }
  
  setOpened(true);
  
  return true;
}

void SvMAX35101EV::close()
{
  stop();
  
  libusb_release_interface(_handle, 0); // отпускаем интерфейс 0
  libusb_close(_handle);  // закрываем устройство
  libusb_exit(NULL);  // завершаем работу с библиотекой 
  
  _handle = nullptr;
  setOpened(false);
}

bool SvMAX35101EV::start(quint32 msecs)
{
  if(!isOpened()) {
    setLastError(QString("%1: device is not opened").arg(deviceInfo().deviceName));
    return false;
  }
  
  if(_pull_thr)
    delete _pull_thr;
  
  _pull_thr = new SvPullMAX35101EV(_handle, msecs, &mutex);
  connect(_pull_thr, &SvPullMAX35101EV::new_data, this, &svidev::SvIDevice::new_data, Qt::QueuedConnection);
  _pull_thr->start();
  
  return true;
}

void SvMAX35101EV::stop()
{
  if(_pull_thr)
    delete _pull_thr;
  
  _pull_thr = nullptr;
}

bool SvMAX35101EV::addNewDevice()
{
  /* показываем форму выбора устройства USB */
  SvSelectMAX35101EVDevice *sdDlg = new SvSelectMAX35101EVDevice();
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

/** ---------  ------------ **/
SvSelectMAX35101EVDevice::SvSelectMAX35101EVDevice(QWidget *parent) :
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

void SvSelectMAX35101EVDevice::accept()
{
  dinfo. deviceName = ui->cbDevice->currentText();
  dinfo.idVendor = _devices.value (ui->cbDevice->currentIndex()).first;
  dinfo.idProduct = _devices.value(ui->cbDevice->currentIndex()).second;
  
  QDialog::accept();
  
}

void SvSelectMAX35101EVDevice::on_bnUpdateDeviceList_clicked()
{
  libusb_device **devs;
  libusb_device_handle *handle = NULL;
  struct libusb_device_descriptor desc;
  unsigned char str[256];  
  
  QString sManufacturer, sProduct;
  

  if (libusb_init(NULL) < 0) {
    QMessageBox::critical(0, "Ошибка", "Ошибка при инициализации интерфейса USB", QMessageBox::Ok);
		return ;
  }

	ssize_t usbDevsCnt = libusb_get_device_list(NULL, &devs);

  ui->cbDevice->clear();
	for(int i = 0; i < usbDevsCnt; i++) {
    
    if(handle != NULL) {
      libusb_close(handle);
      handle = NULL;
    }

    if (libusb_get_device_descriptor(devs[i], &desc) < 0) {
			QMessageBox::critical(this, "Ошибка", "Failed to get device descriptor", QMessageBox::Ok);
			continue;
		}
    
    if (libusb_open (devs[i], &handle) != LIBUSB_SUCCESS)
      continue;
    
    /// iManufacturer 
    if (desc.iManufacturer > 0) {
       
       if (libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, str, 256) < 0)
          continue;
       
       sManufacturer = QString::fromLocal8Bit((const char*)str);
    }
    
    /// iProduct
    if (desc.iProduct > 0) {

      if (libusb_get_string_descriptor_ascii(handle, desc.iProduct, str, 256) < 0)
        continue;
      
      sProduct = QString::fromLocal8Bit((const char*)str);
    }
    
    libusb_close(handle);
    handle = NULL;
    
    
    QString devdesc = QString("%1 %2 [VID:%3 PID:%4]")
                      .arg(sManufacturer)
                      .arg(sProduct)
                      .arg(desc.idVendor, 0, 16)
                      .arg(desc.idProduct, 0, 16);
    
    ui->cbDevice->addItem(devdesc);
    _devices.insert(ui->cbDevice->count() - 1, QPair<uint16_t, uint16_t>(desc.idVendor, desc.idProduct));
  
  }

  libusb_free_device_list(devs, 1);
	libusb_exit(NULL);
  
}

/** ******************************* **/

SvPullMAX35101EV::SvPullMAX35101EV(libusb_device_handle *handle, quint32 timeout, QMutex *mutex)
{
  _handle = handle;
  _timeout = timeout;
  _mutex = mutex;
}

SvPullMAX35101EV::~SvPullMAX35101EV()
{ 
  stop();
  deleteLater();
}

void SvPullMAX35101EV::run()
{
  _started = true;
  _finished = false;
  
  fres* result_1 = nullptr;
  fres* result_2 = nullptr;
  fres* result_3 = nullptr;
  
  svidev::mdata_t measured_data;
  
  /* первый, второй и третий запросы */  
  QByteArray b1 = QByteArray::fromHex("32000200000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
  QByteArray b2 = QByteArray::fromHex("3222C400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
  QByteArray b3 = QByteArray::fromHex("3422C400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
  
  while(_started)
  {
    result_1 = pullData(b1);
    if(result_1->code == 0) {
      
      result_2 = pullData(b2);
      if((result_2->code == 0) & (nullptr != result_2->data)) { 
        
        result_3 = pullData(b3);
        if(result_3->code == 0) { }
        
      }
    }
    
    if(result_2) {
      
      _mutex->lock();
      
      memcpy(&max_data, result_2->data, sizeof(MAX35101EV_ANSWER));
      
      measured_data.tof1 = qFromBigEndian<qint32>(max_data.hit_up_average) / 262.14;   // время пролета 1, в нс.
      measured_data.tof2 = qFromBigEndian<qint32>(max_data.hit_down_average) / 262.14; // время пролета 2, в нс.
      
      _mutex->unlock();
      
      emit new_data(measured_data); 
      
    }
        
    delete result_1;
    delete result_2;
    delete result_3;
    
    msleep(_timeout);
    
  }
  
  _finished = true;
  
}

void SvPullMAX35101EV::stop()
{
  _started = false;
  while(!_finished) QApplication::processEvents();
}

fres* SvPullMAX35101EV::pullData(QByteArray &ba)
{
  fres *result = new fres;
  result->code = 0;
  result->data = nullptr;
    
  int transfered;
  
  result->code = libusb_bulk_transfer(_handle, 0x01, (unsigned char*)ba.data(), ba.size(), &transfered, 1000);
  
  if(result->code != 0) {
    result->message = "Ошибка при отправке\0";
    return result;
  }
  
  // принимаем ответ
  result->data = (char*)malloc(sizeof(MAX35101EV_ANSWER));
  
  result->code = libusb_bulk_transfer(_handle, 0x81, (unsigned char*)result->data, sizeof(MAX35101EV_ANSWER), &transfered, 1000);

  if((result->code != 0)) {
    result->message = "Ошибка при получении ответа\0";
    return result;
  }

  return result;
}

/** ****************************************** **/


