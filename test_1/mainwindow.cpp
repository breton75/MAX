#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../../Common/sv_fnt.h"

QMutex MUTEX1;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  
  /* заполняем список устройств */
  on_bnGetDeviceList_clicked();
  
 
//  ui->cbViewType->addItem("Скорость м/с.");
//  ui->cbViewType->addItem("TOF diff нс.");
//  ui->cbViewType->addItem("(t1 + t2) / 2");

  /* читаем параметры программы */
  ui->spinTimer->setValue(AppParams::readParam(this, "General", "RequestTimer", 500).toInt());
  ui->checkLog->setChecked(AppParams::readParam(this, "General", "Log", true).toBool());
//  ui->cbViewType->setCurrentIndex(ui->cbViewType->findData(AppParams::readParam(this, "Chart", "ViewType", 0)));
  ui->cbDevices->setCurrentIndex(ui->cbDevices->findText(AppParams::readParam(this, "General", "LastDeviceName", "").toString()));
//  ui->checkShowTOF->setChecked(AppParams::readParam(this, "Chart", "ShowTOF", false).toBool());
  ui->editSaveFileNameTemplate->setText(AppParams::readParam(this, "General", "SaveFileNameTemplate", "").toString());
  ui->editSaveFilePath->setText(AppParams::readParam(this, "General", "SaveFilePath", "").toString());

  _chp.x_range = AppParams::readParam(this, "Chart", "x_range", 300).toInt();
  _chp.x_tick_count = AppParams::readParam(this, "Chart", "x_tick_count", 26).toInt();
  _chp.y_autoscale = AppParams::readParam(this, "Chart", "y_autoscale", false).toBool();
  _chp.y_range = AppParams::readParam(this, "Chart", "y_range", 5).toInt();
//  _chp.y_tick_count = AppParams::readParam(this, "Chart", "y_tick_count", 11).toInt();
//  _chp.line_color = QColor(AppParams::readParam(this, "Chart", "line_color", 0xFFFF0000).toUInt());
//  _chp.line_width = AppParams::readParam(this, "Chart", "line_width", 2).toInt();
//  _chp.show_TOF = ui->checkShowTOF->isChecked();

  _chart_w = new svchart::SvChartWidget(_chp);
  _chart_w->setParent(0);
  _chart_w->show();
//  ui->horizontalLayout_2->addWidget(_chart_w);
  
  
//  _chart = new svchart::Chart(_chp); 
//  _chart->legend()->hide();
//  _chart->setAnimationOptions(QChart::NoAnimation);
  
//  chartView = new QChartView(_chart);
//  chartView->setRenderHint(QPainter::Antialiasing);
//  chartView->setParent(this);
////  chartView->setGeometry(0,0,1200, 800);
//  chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//  chartView->setRubberBand(QChartView::RectangleRubberBand);
//  ui->verticalLayout_2->addWidget(chartView);
  
  /* параметры главного окна */
  AppParams::WindowParams p = AppParams::readWindowParams(this);
  this->resize(p.size);
  this->move(p.position);
  this->setWindowState(p.state);
    
  /* параметры окна графиков */
  AppParams::WindowParams gw = AppParams::readWindowParams(this, "GRAPH WINDOW");
  _chart_w->resize(gw.size);
  _chart_w->move(gw.position);
  _chart_w->setWindowState(gw.state);
  
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
  AppParams::saveWindowParams(_chart_w, _chart_w->size(), _chart_w->pos(), _chart_w->windowState(), "GRAPH WINDOW");
  AppParams::saveParam(this, "General", "RequestTimer", ui->spinTimer->value());
  AppParams::saveParam(this, "General", "Log", ui->checkLog->isChecked());
  AppParams::saveParam(this, "General", "LastDeviceName", ui->cbDevices->currentText());
  AppParams::saveParam(this, "General", "SaveFileNameTemplate", ui->editSaveFileNameTemplate->text());
  AppParams::saveParam(this, "General", "SaveFilePath", ui->editSaveFilePath->text());
//  AppParams::saveParam(this, "Chart", "ViewType", ui->cbViewType->currentIndex());
  AppParams::saveParam(this, "Chart", "Autoscale", _chart_w->chartParams().y_autoscale);
  AppParams::saveParam(this, "Chart", "x_range", _chart_w->chartParams().x_range);
//  AppParams::saveParam(this, "Chart", "ShowTOF", ui->checkShowTOF->isChecked());

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
    
    ui->bnSaveToFile->setEnabled(true);
    
    
    _thr = new SvPullUsb(handle, ui->spinTimer->value());
    connect(_thr, SIGNAL(new_data(pullusb::fres*/*, pullusb::MAX35101EV_ANSWER**/)), this, SLOT(new_data(pullusb::fres*/*, pullusb::MAX35101EV_ANSWER**/)));
    _thr->start();

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
      
      ui->bnCycle->setText("Start");
      ui->bnCycle->setEnabled(true);
      
      on_bnSaveToFile_clicked(false);
//      ui->bnSaveToFile->setChecked(false);
      ui->bnSaveToFile->setEnabled(false);
  }
 
}

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
    qreal Vpot = 3*Vsnd * (TOFdiff / (t1 + t2)); // определяем скорость потока, м/с.
    qreal tAvg = (t1 + t2) / 2;

    qreal val;
    
    for(int i = 0; i < _chart_w->graphList().count(); i++) {
      
      int graph_id = _chart_w->graphList()[i];
      
      switch (graph_id) {
        case 0:
          val = Vpot;
          break;
          
        case 1:
          val = TOFdiff;
          break;
          
        case 2:
          val = tAvg;
          break;
          
        default:
          val = Vsnd;
          
      }
      
      _chart_w->appendData(graph_id, val);
      
      if(_file)
          _file->write((const char*)&val, sizeof(qreal));
      
    }
        
    _chart_w->customplot()->replot();

    if(ui->checkLog->isChecked())
      ui->textLog->append(QString("%1%2\tHit Up Avg: %3\tHit Down Avg: %4\tTOF diff: %5\tVpot: %6\tVsnd: %7")
                          .arg(_chart_w->pointCount())
                          .arg(QTime::currentTime().toString("mm:ss.zzz"))
                          .arg(t1).arg(t2).arg(TOFdiff, 0, 'f', 6).arg(Vpot, 0, 'f', 3).arg(Vsnd, 0, 'f', 4));
      
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
  
  if(!_chart_w) return;
  
//  _chart_w->chart()->scroll(-_tick, 0);
//  _chart_w->chart()->m_series->clear();
//  _tick = 0;
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

/** ****************************************** **/

void MainWindow::on_bnSaveToFile_clicked(bool checked)
{
    if(checked) {
        ui->bnSaveToFile->setText("Stop saving");
      
        QDateTime dt = QDateTime::currentDateTime();
        QString fn = ui->editSaveFileNameTemplate->text();
        QString path = ui->editSaveFilePath->text();
        QString ext = "dat";

        QString folder = svfnt::get_folder_name(dt, ext, path);
        if(folder.isEmpty()) {
            QMessageBox::critical(0, "Error", "Неверный путь для сохранения", QMessageBox::Ok);
            ui->bnSaveToFile->setChecked(false);
            return;
        }

        QString s = folder + svfnt::replace_re(dt, ext, fn) + "." + ext;

        MUTEX1.lock();

        _file = new QFile(s);
        bool b = _file->open(QIODevice::WriteOnly);
        
        /* если не удалось создать файл, то сначала запоминаем сообщение об ошибке,
         *  удаляем указатели  и разлочиваем мьютекс, чтобы продолжался опрос устройства */
        if(!b) {
            s = _file->errorString();
            delete _file;
            _file = nullptr;
        }
        /* если файл создан, то пишем в него заголовок и данные графиков */
        else {
          FileHeader file_head;
          file_head.graph_count = _chart_w->graphCount();
          _file->write((const char*)&file_head, sizeof(FileHeader));
          
          for(int i = 0; i < _chart_w->graphList().count(); i++) {
            
            int graph_id = _chart_w->graphList()[i];
            
            GraphHeader graph_head;
            graph_head.graph_id = graph_id;
            
//            memset(&(graph_head.legend), 0, sizeof(GraphHeader::legend));
//            QString lgnd = _chart_w->graphParams(graph_id).legend;
//            lgnd.truncate(sizeof(GraphHeader::legend) - 1);
//            strcpy(&(graph_head.legend[0]), lgnd.toStdString().c_str());
            
            graph_head.line_color = quint32(_chart_w->graphParams(graph_id).line_color.rgb());
            graph_head.line_style = _chart_w->graphParams(graph_id).line_style;
            graph_head.line_width = _chart_w->graphParams(graph_id).line_width;
            
            _file->write((const char*)&graph_head, sizeof(GraphHeader));
            
          }
        }

        MUTEX1.unlock();

        /* затем выводим сообщение об ошибке и выходим */
        if(!b) {
            QMessageBox::critical(0, "Error", s, QMessageBox::Ok);
            ui->bnSaveToFile->setChecked(false);
//            return;
        }

    }

    else if(_file)
    {
        MUTEX1.lock();
        _file->close();
        delete _file;
        _file = nullptr;
        MUTEX1.unlock();
        
        ui->bnSaveToFile->setChecked(false);
        ui->bnSaveToFile->setText("Save to file");
    }
}

void MainWindow::on_bnSaveFileSelectPath_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Путь для сохранения файлов"));
    if(!path.isEmpty())
        ui->editSaveFilePath->setText(path);
}

void MainWindow::on_bnOpenFile_clicked()
{
  QDateTime dt = QDateTime::currentDateTime();
  QString dirpath = ui->editSaveFilePath->text();
  QString ext = "dat";
  
  QDir dir(svfnt::get_folder_name(dt, ext, dirpath));
  if(!dir.exists())
    dir.setPath(QDir::currentPath());
  
  QString fn = QFileDialog::getOpenFileName(this, tr("Открыть файл"), dir.path(), "dat file (*.dat);;all files (*.*)");
  
  if(fn.isEmpty()) return;
  
  QFile f(fn);
  if(!f.open(QIODevice::ReadOnly)) {
    QMessageBox::critical(this, "Error", f.errorString(), QMessageBox::Ok);
    return;
  }
  
  int t = 0;
  svchart::ChartParams p = _chart_w->chartParams();
  svchart::SvChartWidget *chart = new svchart::SvChartWidget(p);
  
  qreal val;
  QCPGraph *graph = _chart_w->customplot()->addGraph();
  while(!f.atEnd()) {
    f.read((char*)(&val), sizeof(qreal));
    
    graph->data()->insert(t, QCPData(double(t), val));
//    chart->chart()->m_series->append(qreal(t++), val);
    t++;
  }
  
  f.close();

  chart->show();
  
}

void MainWindow::on_bnAddGraph_clicked()
{
  SvGraphParamsDialog* chDlg = new SvGraphParamsDialog();
  chDlg->setModal(true);
  chDlg->show();
  
  if(chDlg->result() == QDialog::Accepted)
  {
    qDebug() << "ok";
  }
  
//  int graph_id = ui->cbViewType->currentData().toInt();
  
//  /* если такой график уже есть, то ничего не добавляем и выходим */
//  if(_chart_w->findGraph(graph_id))
//    return;
  
//  svchart::GraphParams gp;
  
//  _chart_w->addGraph(graph_id, gp);
  
}


