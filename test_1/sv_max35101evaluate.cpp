#include "sv_max35101evaluate.h"

extern SvSQLITE *SQLITE;
SvSelectMAX35101EvDevice *SELECTMAX35101EV_DEVICE;

SvMAX35101Evaluate::SvMAX35101Evaluate(svidev::DeviceInfo deviceInfo, QObject *parent)
{
  setDeviceInfo(deviceInfo);
  setParent(parent);
}

SvMAX35101Evaluate::~SvMAX35101Evaluate()
{
  stop();
  close();
  deleteLater();
}

bool SvMAX35101Evaluate::open()
{
  libusb_context *ctx = NULL;  
  libusb_init(NULL);  // инициализируем библиотеку
  
  // открываем устройство
  _handle = libusb_open_device_with_vid_pid(ctx, _device_info.idVendor, _device_info.idProduct);
    
  if (!_handle)
  {
    libusb_exit(NULL);
    setLastError(QString("%1: libusb_open_device_with_vid_pid error. (idVendor = %2, idProduct = %3")
                  .arg(deviceInfo().name).arg(deviceInfo().idVendor).arg(deviceInfo().idProduct));
    return false;
  }
  
  // запрашиваем интерфейс 0 для себя
  int err = libusb_claim_interface(_handle, 0); 
  if(err < 0) { 
    setLastError(QString("%1: libusb_claim_interface error: %2").arg(deviceInfo().name).arg(err));
    return false;
  }
  
  setOpened(true);
  
  return true;
}

void SvMAX35101Evaluate::close()
{
  libusb_release_interface(_handle, 0); // отпускаем интерфейс 0
  libusb_close(_handle);  // закрываем устройство
  libusb_exit(NULL);  // завершаем работу с библиотекой 
  
  _handle = nullptr;
  setOpened(false);
}

bool SvMAX35101Evaluate::start(quint32 msecs)
{
  if(!isOpened()) {
    setLastError(QString("%1: device is not opened").arg(deviceInfo().name));
    return false;
  }
  
  if(_thr)
    delete _thr;
  
  _thr = new SvPullMAX35101Evaluate(_handle, msecs, &mutex);
  connect(_thr, SIGNAL(new_data(svidev::MeasuredData )), this, SIGNAL(new_data(svidev::MeasuredData )));
  _thr->start();
  
  
}

bool SvMAX35101Evaluate::stop()
{
  if(_thr)
    delete _thr;
  
  _thr = nullptr;
}

void SvMAX35101Evaluate::addNewDevice()
{
  uint16_t vendor_id;
  uint16_t product_id;
  
  qDebug() << 1;
  /* показываем форму выбора устройства USB */
  SvSelectMAX35101EvDevice *sdDlg = new SvSelectMAX35101EvDevice();
  qDebug() << 2;
  if(sdDlg->exec() != QDialog::Accepted) {
    delete sdDlg;
    return;
  }
  
  vendor_id = sdDlg->dinfo.idVendor;
  product_id = sdDlg->dinfo.idProduct;
  
  delete sdDlg;
//  тут выполняем проверку
  
  QSqlError err = SQLITE->execSQL(QString(SQL_NEW_DEVICE)
                                  .arg(svidev::MAX35101EV)
                                  .arg(svidev::SupportedDevicesNames.value(svidev::MAX35101EV))
                                  .arg(vendor_id, 0, 16)
                                  .arg(product_id, 0, 16)
                                  .arg(vendor_id)
                                  .arg(product_id)
                                  .arg(-1)
                                  .arg(-1));
  
  if(err.type() != QSqlError::NoError) {
    QMessageBox::critical(0, "Ошибка", QString("Не удалось добавить новое устройство:\n%1").arg(err.databaseText()), QMessageBox::Ok);
    return;
  }
 
  
}

/** ---------  ------------ **/
SvSelectMAX35101EvDevice::SvSelectMAX35101EvDevice(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SvSelectMAX35101DeviceDialog)
{
  
  ui->setupUi(this);
  
  on_bnUpdateDeviceList_clicked();
  
  connect(ui->bnOk, SIGNAL(pressed()), this, SLOT(accept()));
  connect(ui->bnCancel, SIGNAL(pressed()), this, SLOT(reject()));
  
  setModal(true);
  show();
    
}

void SvSelectMAX35101EvDevice::accept()
{
  svidev::DeviceInfo dinfo;
  
  dinfo.idVendor = _devices.value (ui->cbDevice->currentIndex()).first;
  dinfo.idProduct = _devices.value(ui->cbDevice->currentIndex()).second;
  
  QDialog::accept();
  
}

void SvSelectMAX35101EvDevice::on_bnUpdateDeviceList_clicked()
{
  libusb_device **devs;
  libusb_device_handle *devHandle = NULL;
	
  int r;
	ssize_t cnt;
  unsigned char strDesc[256];  
  
	r = libusb_init(NULL);
	if (r < 0)
		return ;

	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0)
		return ;
  
  libusb_device *dev;
	int i = 0;

  ui->cbDevice->clear();
  
	while ((dev = devs[i++]) != NULL) {
		
    if(devHandle != NULL) {
      libusb_close(devHandle);
      devHandle = NULL;
    }
    
    struct libusb_device_descriptor desc;
    
		int r = libusb_get_device_descriptor(dev, &desc);
		
    if (r < 0) {
			QMessageBox::critical(this, "Ошибка", "Failed to get device descriptor", QMessageBox::Ok);
			continue;
		}
    
    r = libusb_open (dev, &devHandle);
    if (r != LIBUSB_SUCCESS)
      continue;
    
    // Get the string associated with iManufacturer index.
    if (desc.iManufacturer > 0) {
      
       r = libusb_get_string_descriptor_ascii(devHandle, desc.iManufacturer, strDesc, 256);
       
       if (r < 0)
          continue;

       qDebug() << "   iManufacturer" <<  strDesc;
    }

    //========================================================================
    // Get string associated with iProduct index.
    if (desc.iProduct > 0) {
      
       r = libusb_get_string_descriptor_ascii(devHandle, desc.iProduct, strDesc, 256);
       
       if (r < 0)
          continue;

       qDebug() << "   iProduct" << strDesc;
    }
    
    
    QString devdesc = QString("%1:%2 (bus %3, device %4)")
                      .arg(desc.idVendor, 0, 16)
                      .arg(desc.idProduct, 0, 16)
                      .arg(libusb_get_bus_number(dev))
                      .arg(libusb_get_device_address(dev));
    
    ui->cbDevice->addItem(devdesc);
    _devices.insert(ui->cbDevice->count() - 1, QPair<uint16_t, uint16_t>(desc.idVendor, desc.idProduct));
  
  }

  libusb_free_device_list(devs, 1);
	libusb_exit(NULL);
  
}

/** ******************************* **/

SvPullMAX35101Evaluate::~SvPullMAX35101Evaluate()
{ 
  stop();
  deleteLater();
}

void SvPullMAX35101Evaluate::run()
{
  _started = true;
  _finished = false;
  
  fres* result_1 = nullptr;
  fres* result_2 = nullptr;
  fres* result_3 = nullptr;
  
  svidev::MeasuredData measured_data;
  
  while(_started)
  {
    /* первый, второй и третий запросы */  
    QByteArray b1 = QByteArray::fromHex("32000200000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
    QByteArray b2 = QByteArray::fromHex("3222C400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
    QByteArray b3 = QByteArray::fromHex("3422C400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
    
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

void SvPullMAX35101Evaluate::stop()
{
  _started = false;
  while(!_finished) QApplication::processEvents();
}

fres* SvPullMAX35101Evaluate::pullData(QByteArray &ba)
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


