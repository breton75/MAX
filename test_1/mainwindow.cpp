#include "mainwindow.h"
#include "ui_mainwindow.h"



QMutex MUTEX1;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  
  /* заполняем список устройств */
  on_bnGetDeviceList_clicked();
  
  /* режимы отображения */
  ui->cbViewType->addItem("Скорость м/с.");
  ui->cbViewType->addItem("TOF diff нс.");

  /* читаем параметры программы */
  ui->spinTimer->setValue(AppParams::readParam(this, "General", "RequestTimer", 500).toInt());
  ui->checkLog->setChecked(AppParams::readParam(this, "General", "Log", true).toBool());
  ui->cbViewType->setCurrentIndex(AppParams::readParam(this, "Chart", "ViewType", 0).toInt());
  ui->checkAutoscale->setChecked(AppParams::readParam(this, "Chart", "Autoscale", true).toBool());
  ui->cbDevices->setCurrentIndex(ui->cbDevices->findText(AppParams::readParam(this, "General", "LastDeviceName", "").toString()));
  
  _chp.x_range = AppParams::readParam(this, "Chart", "x_range", 300).toInt();
  ui->spinXRange->setValue(_chp.x_range);
  _chp.x_tick_count = AppParams::readParam(this, "Chart", "x_tick_count", 26).toInt();
  _chp.y_range = AppParams::readParam(this, "Chart", "y_range", 5).toInt();
  _chp.y_tick_count = AppParams::readParam(this, "Chart", "y_tick_count", 11).toInt();
  _chp.line_color = QColor(AppParams::readParam(this, "Chart", "line_color", 0xFFFF0000).toUInt());
  _chp.line_width = AppParams::readParam(this, "Chart", "line_width", 2).toInt();
  
  _chart = new Chart(_chp); 
  _chart->legend()->hide();
  _chart->setAnimationOptions(QChart::NoAnimation);
  
  chartView = new QChartView(_chart);
  chartView->setRenderHint(QPainter::Antialiasing);
  chartView->setParent(this);
//  chartView->setGeometry(0,0,1200, 800);
  chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  chartView->setRubberBand(QChartView::RectangleRubberBand);
  ui->verticalLayout_2->addWidget(chartView);
  
  /* параметры окна */
  AppParams::WindowParams p = AppParams::readWindowParams(this);
  this->resize(p.size);
  this->move(p.position);
  this->setWindowState(p.state);
    
}

MainWindow::~MainWindow()
{
  /* завершаем работу с платой */
  if(_thr)
  {
//    _thr->killTimer(_timerId);
    _thr->stop();
    _thr->deleteLater(); 
    delete _thr;
    _thr = nullptr;
    
    libusb_release_interface(handle, 0); // отпускаем интерфейс 0
    libusb_close(handle);  // закрываем устройство
    libusb_exit(NULL);  // завершаем работу с библиотекой  
  }
  
  /* сохраняем парметры программы */
  AppParams::saveWindowParams(this, this->size(), this->pos(), this->windowState());
  AppParams::saveParam(this, "General", "RequestTimer", ui->spinTimer->value());
  AppParams::saveParam(this, "General", "Log", ui->checkLog->isChecked());
  AppParams::saveParam(this, "General", "LastDeviceName", ui->cbDevices->currentText());
  AppParams::saveParam(this, "Chart", "ViewType", ui->cbViewType->currentIndex());
  AppParams::saveParam(this, "Chart", "Autoscale", ui->checkAutoscale->isChecked());
  AppParams::saveParam(this, "Chart", "x_range", _chp.x_range);
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
  
//  pullusb::MAX35101EV_ANSWER max_data;
  
  MUTEX1.lock();
  pullusb::fres *result = pullusb::request(handle/*, max_data*/);
  MUTEX1.unlock();
  
  emit new_data(result/*, &max_data*/);

  
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
    
//    tm.setSingleShot(true);
//    tm.connect(&tm, SIGNAL(timeout()), this, SLOT(tmTimeout()));
    
    _thr = new SvPullUsb(handle, ui->spinTimer->value());
    connect(_thr, SIGNAL(new_data(pullusb::fres*/*, pullusb::MAX35101EV_ANSWER**/)), this, SLOT(new_data(pullusb::fres*/*, pullusb::MAX35101EV_ANSWER**/)));
//    _timerId = _thr->startTimer(200);
    _thr->start();

    //    tm.start(1000);
  }
  else
  {
//    tm.stop();
    //    _thr->killTimer(_timerId);
      _thr->stop();
      _thr->deleteLater(); // 
      delete _thr;
      _thr = nullptr;
      
      libusb_release_interface(handle, 0); // отпускаем интерфейс 0
      libusb_close(handle);  // закрываем устройство
      libusb_exit(NULL);  // завершаем работу с библиотекой  
      
      ui->bnCycle->setText("Cycle");
      ui->bnCycle->setEnabled(true);
  }
 
}

//void MainWindow::tmTimeout()
//{
//  qDebug() << "tm";
//  MUTEX1.lock();
//  QGraphicsLineItem *l =_chart->scene()->addLine(_tick * _chart->plotArea().width() / _chp.x_range, -1, _tick * _chart->plotArea().width() / _chp.x_range, 1, QPen(Qt::blue));
//  l->setZValue(100);
//  l->show();
//  MUTEX1.unlock();
  
//    _thr->stop();
//    _thr->deleteLater(); // 
//    delete _thr;
//    _thr = nullptr;
    
//    libusb_release_interface(handle, 0); // отпускаем интерфейс 0
//    libusb_close(handle);  // закрываем устройство
//    libusb_exit(NULL);  // завершаем работу с библиотекой  
    
//    ui->bnCycle->setText("Cycle");
//    ui->bnCycle->setEnabled(true);
//}

void MainWindow::new_data(pullusb::fres *result/*, pullusb::MAX35101EV_ANSWER *max_data*/)
{
  if(result->code == 0)
  {
    MUTEX1.lock();

    memcpy(&_max_data, result->data, sizeof(pullusb::MAX35101EV_ANSWER));
    
    /** ******************************* **/
    qreal t1 = qFromBigEndian<qint32>(_max_data.hit_up_average) / 262.14;   // время пролета 1, в нс.
    qreal t2 = qFromBigEndian<qint32>(_max_data.hit_down_average) / 262.14; // время пролета 2, в нс.
    qreal L = 0.0895; // расстояние между излучателями в метрах
    
    qreal TOFdiff = t1 - t2;
    qreal Vsnd = 2 * L / ((t1 + t2) / 1000000000); // определяем скорость звука в среде, м/с.
    qreal Vpot = Vsnd * (TOFdiff / (t1 + t2)); // определяем скорость потока, м/с.
    
//    qDebug() << TOFdiff << Vsnd << Vpot;

    qreal val = ui->cbViewType->currentIndex() == 1 ? TOFdiff : Vpot;

    _chart->m_series->append(_tick++, val);

    if(_tick >= _chart->axX->max())
      _chart->scroll(_chart->plotArea().width() / _chp.x_range, 0);
    
    
    if(ui->checkLog->isChecked())
      ui->textLog->append(QString("%1\tHit Up Avg: %2\tHit Down Avg: %3\tTOF diff: %4\tVpot: %5\tVsnd: %6")
                          .arg(QTime::currentTime().toString("mm:ss.zzz"))
                          .arg(t1).arg(t2).arg(TOFdiff, 0, 'f', 6).arg(Vpot, 0, 'f', 3).arg(Vsnd, 0, 'f', 4));
    
    if(ui->checkAutoscale->isChecked() & (qAbs<qreal>(val) > qAbs<qreal>(_chp.y_range)))
    {
      _chp.y_range = qAbs<qreal>(val) * 1.1;
      _chart->axisY()->setRange(-_chp.y_range, _chp.y_range);
      _chart->update();
    }
    
    MUTEX1.unlock();
    
  }
  else
  {
    ui->textLog->append(QString("Error: %1").arg(QString(result->message)));
  }
  
  delete result;
  
}

void MainWindow::on_cbViewType_currentIndexChanged(int index)
{
  Q_UNUSED(index);
  
  if(!_chart) return;
  
  _chart->scroll(-_tick, 0);
  _chart->m_series->clear();
  _tick = 0;
}


/** ******************************* **/
//void SvPullUsb::timerEvent(QTimerEvent *te)
//{
//  MUTEX1.lock();
//  pullusb::fres *result = pullusb::request(_handle, max_data);
//  MUTEX1.unlock();

//  emit new_data(result, &max_data);
  
//}

SvPullUsb::~SvPullUsb()
{ 
  stop();
  deleteLater();
}

void SvPullUsb::run()
{
  _started = true;
  _finished = false;
  
  while(_started)
  {
    MUTEX1.lock();
    pullusb::fres *result = pullusb::request(_handle/*, max_data*/);
    MUTEX1.unlock();
    
    if(result)
      emit new_data(result/*, &max_data*/); 
    
    msleep(_timeout);
    
  }
  
  _finished = true;
  
}

void SvPullUsb::stop()
{
  _started = false;
  while(!_finished) QApplication::processEvents();
}


/** ****************************** **/


void MainWindow::on_actionChartSettings_triggered()
{
    
}

void MainWindow::on_bnSetXRange_clicked()
{
  _chart->axX->setRange(0, ui->spinXRange->value());  
  _chp.x_range = ui->spinXRange->value();
}

void MainWindow::on_bnYRangeUp_clicked()
{
  _chp.y_range = ((_chart->axY->max() - _chart->axY->min()) / 2) * 1.25;
  _chp.y_range = _chp.y_range < 1 ? 1 : _chp.y_range;
  _chart->axY->setRange(-_chp.y_range, _chp.y_range);
}

void MainWindow::on_bnYRangeDown_clicked()
{
  _chp.y_range = ((_chart->axY->max() - _chart->axY->min()) / 2) / 1.25;
  _chp.y_range = _chp.y_range < 1 ? 1 : _chp.y_range;
  _chart->axY->setRange(-_chp.y_range, _chp.y_range);
}

void MainWindow::on_bnXRangeUp_clicked()
{
  _chp.x_range *= 1.25;
  _chart->axX->setRange(0, _chp.x_range);
  _chart->update();
}

void MainWindow::on_bnXRangeDown_clicked()
{
  _chp.x_range /= 1.25;
  _chart->axX->setRange(0, _chp.x_range);
}

void MainWindow::on_bnYRangeActual_clicked()
{
  MUTEX1.lock();
  
  qreal max = -1000000000;
  qreal min = 1000000000;
  
  foreach (QPointF pnt, _chart->m_series->pointsVector()) {
    if(pnt.y() > max)
      max = pnt.y();
    
    if(pnt.y() < min)
      min = pnt.y();
  }
  
  _chp.y_range = qAbs<qreal>(max) > qAbs<qreal>(min) ? qAbs<qreal>(max) : qAbs<qreal>(min);
  _chart->axY->setRange(-_chp.y_range, _chp.y_range);
  
  MUTEX1.unlock();
}

void MainWindow::on_bnXRangeActual_clicked()
{
  _chart->axX->setRange(0, _tick);
}

void MainWindow::on_bnResetChart_clicked()
{
  _chart->axX->setRange(0, _chp.x_range);
  _chart->m_series->clear();
  _tick = 0;   
}

void MainWindow::on_checkAutoscale_clicked(bool checked)
{
  if(checked)
    on_bnYRangeActual_clicked();
}
