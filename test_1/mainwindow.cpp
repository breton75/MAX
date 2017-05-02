#include "mainwindow.h"
#include "ui_mainwindow.h"



QMutex MUTEX1;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  
  on_bnGetDeviceList_clicked();
  
  _chart = new Chart(); 
  _chart->setTitle("Значения датчика (мкс.)");
  _chart->legend()->hide();
  _chart->setAnimationOptions(QChart::NoAnimation);
  
  chartView = new QChartView(_chart);
  chartView->setRenderHint(QPainter::Antialiasing);
  chartView->setParent(ui->dockWidget);
  chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//  chartView->setGeometry(100, 100, 400, 400);
  
  ui->verticalLayout_2->addWidget(chartView);
  
//  chartView->show();
  
//  setCentralWidget(&chartView);
  
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::on_bnGetDeviceList_clicked()
{
  libusb_device **devs;
	int r;
	ssize_t cnt;

	r = libusb_init(NULL);
	if (r < 0)
		return ;

	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0)
		return ;
  
  libusb_device *dev;
	int i = 0;

	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			qDebug() << "failed to get device descriptor";
			return;
		}
    
//    libusb_get_string_descriptor(dev,)
    
    QString devdesc = QString("%1:%2 (bus %3, device %4): %5")
                      .arg(desc.idVendor, 0, 16)
                      .arg(desc.idProduct, 0, 16)
                      .arg(libusb_get_bus_number(dev))
                      .arg(libusb_get_device_address(dev));
    
    ui->cbDevices->addItem(devdesc);
    _devices.insert(ui->cbDevices->count() - 1, QPair<uint16_t, uint16_t>(desc.idVendor, desc.idProduct));
  
  }

  libusb_free_device_list(devs, 1);
	libusb_exit(NULL);
  
  ui->cbDevices->setCurrentIndex(3);
}

void MainWindow::on_bnOneShot_clicked()
{
  libusb_context *ctx = NULL;  
  libusb_init(&ctx);  // инициализируем библиотеку
  
  handle = libusb_open_device_with_vid_pid(ctx, _devices.value(ui->cbDevices->currentIndex()).first, _devices.value(ui->cbDevices->currentIndex()).second); // открываем устройство
    
  if (!handle)
  {
    QMessageBox::critical(0, "Error", "Device could not be open or found", QMessageBox::Ok);
    libusb_exit(ctx);
    return;
  }
   
  libusb_claim_interface(handle, 0);  // запрашиваем интерфейс 0 для себя
  
  pullusb::MAX35101EV_ANSWER max_data;
  
  MUTEX1.lock();
  pullusb::fres *result = pullusb::request(handle, max_data);
  MUTEX1.unlock();
  
  emit new_data(result, &max_data);

  
  libusb_release_interface(handle, 0); // отпускаем интерфейс 0
  libusb_close(handle);  // закрываем устройство
  libusb_exit(ctx);  // завершаем работу с библиотекой
  
}

void MainWindow::on_bnCycle_clicked()
{
  ui->bnCycle->setEnabled(false);
  QApplication::processEvents();
  
  if(!_thr)
  {
    libusb_context *ctx = NULL;  
    libusb_init(NULL);  // инициализируем библиотеку
    
    handle = libusb_open_device_with_vid_pid(ctx, _devices.value(ui->cbDevices->currentIndex()).first, _devices.value(ui->cbDevices->currentIndex()).second); // открываем устройство
      
    if (!handle)
    {
      QMessageBox::critical(0, "Error", "Device could not be open or found", QMessageBox::Ok);
      libusb_exit(NULL);
    }
     
    libusb_claim_interface(handle, 0);  // запрашиваем интерфейс 0 для себя
    
    ui->bnCycle->setText("Stop");
    ui->bnCycle->setEnabled(true);
    
    _thr = new SvPullUsb(handle);
//    connect(_thr, SIGNAL(finished()), _thr, SLOT(deleteLater()));
//    connect(_thr, SIGNAL(finished()), this, SLOT(pulling_finished()));
    connect(_thr, SIGNAL(new_data(pullusb::fres*, pullusb::MAX35101EV_ANSWER*)), this, SLOT(new_data(pullusb::fres*, pullusb::MAX35101EV_ANSWER*)));
    _timerId = _thr->startTimer(200);
    
  }
  else /*if(_thr->is)*/
  {
    _thr->killTimer(_timerId);
    _thr->deleteLater(); // stop();
    delete _thr;
    _thr = nullptr;
    
    libusb_release_interface(handle, 0); // отпускаем интерфейс 0
    libusb_close(handle);  // закрываем устройство
    libusb_exit(NULL);  // завершаем работу с библиотекой  
    
    ui->bnCycle->setText("Cycle");
    ui->bnCycle->setEnabled(true);
    
  }
}

void MainWindow::new_data(pullusb::fres *result, pullusb::MAX35101EV_ANSWER *max_data)
{
  if(result->code == 0)
  {
    MUTEX1.lock();
    memcpy(&_max_data, max_data, sizeof(pullusb::MAX35101EV_ANSWER));
    MUTEX1.unlock();
    
    _tick++;

//    qDebug() << qFromBigEndian<qint32>(_max_data.hit_up_average) << qFromBigEndian<qint32>(_max_data.hit_down_average);
    
    /** ******************************* **/
    qreal up = qFromBigEndian<qint32>(_max_data.hit_up_average) / 262.14;
    qreal down = qFromBigEndian<qint32>(_max_data.hit_down_average) / 262.14;
    qreal diff = up - down;
//    qDebug() << diff;
    
    _chart->m_series->append(_tick, diff);
    
    ui->textEdit->append(QString("Hit Up Avg. %1\tHit Down Avg. %2\tTOF %3").arg(up).arg(down).arg(diff));
    
    
  }
  else
  {
    qDebug() << "Error: " << result->message;
  }
  
  delete result;
  
  
}

void MainWindow::pulling_finished()
{

}

/** ******************************* **/
SvPullUsb::SvPullUsb(libusb_device_handle* handle)
{
  _handle = handle;
//  _isWorking = true;
}

SvPullUsb::~SvPullUsb()
{ 
//  _isWorking = false;
  
//  while(!_isFinished) QApplication::processEvents();
//  deleteLater();
}

void SvPullUsb::timerEvent(QTimerEvent *te)
{
  MUTEX1.lock();
  pullusb::fres *result = pullusb::request(_handle, max_data);
  MUTEX1.unlock();

  emit new_data(result, &max_data);
  
}

void SvPullUsb::stop()
{
  
}
