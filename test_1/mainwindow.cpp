#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../../svlib/sv_fnt.h"
//#include "../../svlib/sv_log.h"

QMutex MUTEX1;
extern SvSelectDeviceTypeDialog *SELECTDEVICETYPE_UI;

/** ----------------  --------------- **/

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  
  /* лог */
  log = svlog::SvLog(ui->textLog);
  
  /* заполняем список устройств */
  on_bnGetDeviceList_clicked();
  
 
//  ui->cbViewType->addItem("Скорость м/с.");
//  ui->cbViewType->addItem("TOF diff нс.");
//  ui->cbViewType->addItem("(t1 + t2) / 2");

  /* читаем параметры программы */
  ui->spinTimer->setValue(AppParams::readParam(this, "General", "RequestTimer", 200).toInt());
  ui->checkLog->setChecked(AppParams::readParam(this, "General", "Log", true).toBool());
//  ui->cbViewType->setCurrentIndex(ui->cbViewType->findData(AppParams::readParam(this, "Chart", "ViewType", 0)));
  ui->cbDevices->setCurrentIndex(ui->cbDevices->findText(AppParams::readParam(this, "General", "LastDeviceName", "").toString()));
//  ui->checkShowTOF->setChecked(AppParams::readParam(this, "Chart", "ShowTOF", false).toBool());
  ui->editSaveFileNameTemplate->setText(AppParams::readParam(this, "General", "SaveFileNameTemplate", "").toString());
  ui->editSaveFilePath->setText(AppParams::readParam(this, "General", "SaveFilePath", "").toString());
  ui->gbSynchronizeArduino->setChecked(AppParams::readParam(this, "General", "SynchronizeArduino", true).toBool());

  _chart_params.x_range = AppParams::readParam(this, "Chart", "x_range", 300).toInt();
  _chart_params.x_tick_count = AppParams::readParam(this, "Chart", "x_tick_count", 26).toInt();
  _chart_params.y_autoscale = AppParams::readParam(this, "Chart", "y_autoscale", false).toBool();
  _chart_params.y_range = AppParams::readParam(this, "Chart", "y_range", 5).toInt();
  _chart_params.x_tick_period = ui->spinTimer->value();
  _chart_params.x_autoscroll_type = static_cast<svchart::ChartXAutoscrollTypeIDs>(AppParams::readParam(this, "Chart", "x_autoscroll_type", 0).toInt());
  _chart_params.x_measure_unit = static_cast<svchart::ChartXMeasureUnitIDs>(AppParams::readParam(this, "Chart", "x_measure_unit", 0).toInt());

  _chart = new svchart::SvChartWidget(_chart_params, ui->dockPlot);
  ui->verticalLayout_7->addWidget(_chart);
//  _chart->show();
  
  /* параметры arduino */
  _arduino_params.ip = AppParams::readParam(this, "Arduino", "ip", "192.168.44.44").toString();
  _arduino_params.port = AppParams::readParam(this, "Arduino", "port", 35580).toInt();
  _arduino_params.spin_clockwise = AppParams::readParam(this, "Arduino", "spin_clockwise", true).toBool();
  _arduino_params.engine_pw = AppParams::readParam(this, "Arduino", "engine_pw", 100).toInt();
  _arduino_params.state_period = ui->spinTimer->value(); //AppParams::readParam(this, "Arduino", "state_period", ui->spinTimer->value()).toInt();
  _arduino_params.state_period_enable = AppParams::readParam(this, "Arduino", "state_period_enable", true).toBool();
  _arduino_params.turn_angle = AppParams::readParam(this, "Arduino", "turn_angle", 180).toInt();
  _arduino_params.turn_angle_enable = AppParams::readParam(this, "Arduino", "turn_angle_enable", false).toBool();
  _arduino_params.turn_count = AppParams::readParam(this, "Arduino", "turn_count", 1).toInt();
  _arduino_params.turn_count_enable = AppParams::readParam(this, "Arduino", "turn_count_enable", false).toBool();
  _arduino_params.current_voltage = AppParams::readParam(this, "Arduino", "current_voltage", 12).toInt();
//  _arduino_params. = AppParams::readParam(this, "Arduino", "", );
  
  _arduino = new svarduinomax::SvArduinoWidget(_arduino_params, ui->textLog, 0);
  ui->verticalLayout_9->addWidget(_arduino);
  
  /* параметры главного окна */
  AppParams::WindowParams p = AppParams::readWindowParams(this);
  this->resize(p.size);
  this->move(p.position);
  this->setWindowState(p.state);
    
  /* параметры окна графиков */
  AppParams::WindowParams gw = AppParams::readWindowParams(this, "PLOT WINDOW");
  ui->dockPlot->resize(gw.size);
  ui->dockPlot->move(gw.position);
  ui->dockPlot->setWindowState(gw.state);
  
  /* параметры окна arduino */
  AppParams::WindowParams aw = AppParams::readWindowParams(this, "ARDUINO WINDOW");
  ui->dockArduino->resize(aw.size);
  ui->dockArduino->move(aw.position);
  ui->dockArduino->setWindowState(aw.state);
  
  /* читаем информацию о графиках */
  int gcnt = AppParams::readParam(this, "Chart", "GraphCount", 0).toInt();
  
  for(int i = 0; i < gcnt; i++) {
    svgraph::GraphParams p;
    
    p.type = static_cast<svgraph::GraphIDs>(AppParams::readParam(this, QString("Graph_%1").arg(i), "TypeID", 0).toInt());
    p.line_color = QColor(AppParams::readParam(this, QString("Graph_%1").arg(i), "LineColor", "red").toString());
    p.line_style = AppParams::readParam(this, QString("Graph_%1").arg(i), "LineStyle", static_cast<int>(Qt::SolidLine)).toInt();
    p.line_width = AppParams::readParam(this, QString("Graph_%1").arg(i), "LineWidth", 1).toInt();
    p.legend = AppParams::readParam(this, QString("Graph_%1").arg(i), "LineLegend", "").toString();

    _chart->addGraph(p.type, p);
    _addGraphToList(p.type, p);
    
  }
  
  if(ui->listGraphs->count() > 0)
    ui->listGraphs->setCurrentRow(0);
  
  /* готовим карту для вычисляемых значений */
  foreach (svgraph::GraphIDs key, svgraph::GraphTypes.keys())
    _calcs.insert(key, 0);

  connect(this, SIGNAL(newState(bool)), this, SLOT(stateChanged(bool)));
  
  connect(_chart, SIGNAL(onReset()), this, SLOT(onChartReset()));
  
}

MainWindow::~MainWindow()
{
  /* завершаем работу с платой */
//  if(_thr)
//  {
//    /** ВНИМАНИЕ здесь вызывается деструктор ~SvPullUsb() **/  
//    delete _thr;
//    _thr = nullptr;
    
//#ifndef NO_USB_DEVICE
//    libusb_release_interface(handle, 0); // отпускаем интерфейс 0
//    libusb_close(handle);  // закрываем устройство
//    libusb_exit(NULL);  // завершаем работу с библиотекой  
//#endif
//  }
  
  /* сохраняем парметры программы */
  AppParams::saveWindowParams(this, this->size(), this->pos(), this->windowState());
  AppParams::saveWindowParams(ui->dockPlot, ui->dockPlot->size(), ui->dockPlot->pos(), ui->dockPlot->windowState(), "PLOT WINDOW");
  AppParams::saveWindowParams(ui->dockArduino, ui->dockArduino->size(), ui->dockArduino->pos(), ui->dockArduino->windowState(), "ARDUINO WINDOW");
  
  AppParams::saveParam(this, "General", "RequestTimer", ui->spinTimer->value());
  AppParams::saveParam(this, "General", "Log", ui->checkLog->isChecked());
  AppParams::saveParam(this, "General", "LastDeviceName", ui->cbDevices->currentText());
  AppParams::saveParam(this, "General", "SaveFileNameTemplate", ui->editSaveFileNameTemplate->text());
  AppParams::saveParam(this, "General", "SaveFilePath", ui->editSaveFilePath->text());
  AppParams::saveParam(this, "General", "SynchronizeArduino", ui->gbSynchronizeArduino->isChecked());
  
  AppParams::saveParam(this, "Chart", "Autoscale", _chart->params().y_autoscale);
  AppParams::saveParam(this, "Chart", "x_range", _chart->params().x_range);
  AppParams::saveParam(this, "Chart", "x_autoscroll_type", static_cast<int>(_chart->params().x_autoscroll_type));
  AppParams::saveParam(this, "Chart", "x_measure_unit", static_cast<int>(_chart->params().x_measure_unit));
//  AppParams::saveParam(this, "Chart", "", );
  
  /* сохраняем список графиков */
  AppParams::saveParam(this, "Chart", "GraphCount", _chart->graphCount());
  
  for (int i = 0; i < _chart->graphList().count(); i++) {
    svgraph::GraphIDs graph_id = _chart->graphList()[i];
    AppParams::saveParam(this, QString("Graph_%1").arg(i), "TypeID", int(graph_id));
    AppParams::saveParam(this, QString("Graph_%1").arg(i), "LineColor", _chart->graphParams(graph_id).line_color.name());
    AppParams::saveParam(this, QString("Graph_%1").arg(i), "LineStyle", _chart->graphParams(graph_id).line_style);
    AppParams::saveParam(this, QString("Graph_%1").arg(i), "LineWidth", _chart->graphParams(graph_id).line_width);
    AppParams::saveParam(this, QString("Graph_%1").arg(i), "LineLegend", _chart->graphParams(graph_id).legend);
  }
  
  /* параметры arduino */
  AppParams::saveParam(this, "Arduino", "ip", _arduino->params().ip);
  AppParams::saveParam(this, "Arduino", "port", _arduino->params().port);
  AppParams::saveParam(this, "Arduino", "spin_clockwise", _arduino->params().spin_clockwise);
  AppParams::saveParam(this, "Arduino", "engine_pw", _arduino->params().engine_pw);
  AppParams::saveParam(this, "Arduino", "state_period", _arduino->params().state_period);
  AppParams::saveParam(this, "Arduino", "state_period_enable", _arduino->params().state_period_enable);
  AppParams::saveParam(this, "Arduino", "turn_angle", _arduino->params().turn_angle);
  AppParams::saveParam(this, "Arduino", "turn_angle_enable", _arduino->params().turn_angle_enable);
  AppParams::saveParam(this, "Arduino", "turn_count", _arduino->params().turn_count);
  AppParams::saveParam(this, "Arduino", "turn_count_enable", _arduino->params().turn_count_enable);
  AppParams::saveParam(this, "Arduino", "current_voltage", _arduino->params().current_voltage);
  
  
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
//  pullusb::fres *result = pullusb::request(handle/*, max_data*/);
  MUTEX1.unlock();
  
//  emit new_data(result/*, &max_data*/);

  
  libusb_release_interface(handle, 0); // отпускаем интерфейс 0
  libusb_close(handle);  // закрываем устройство
  libusb_exit(ctx);  // завершаем работу с библиотекой
  
}

void MainWindow::on_bnCycle_clicked()
{
  if(ui->listGraphs->count() == 0) {
    QMessageBox::information(this, "Info", "Необходимо добавить хотя бы один график");
    on_bnAddGraph_clicked();
    return;
  }
  
  ui->bnCycle->setEnabled(false);
  ui->bnOneShot->setEnabled(false);
  ui->frame->setEnabled(false);
  QApplication::processEvents();
  
  // параметры окна графиков
  _chart_params = _chart->params();
  _chart_params.x_tick_period = ui->spinTimer->value();
  _chart->setParams(_chart_params);
  
  if(!_dev)
  {
    /** синхронизация с Arduino **/
    if(ui->gbSynchronizeArduino->isChecked()) {
      
      if(!_arduino->start()) {
        emit newState(false);
        return;
      }
    }
    
    svidev::DeviceInfo dinfo;
//    dinfo.
    svidev::SupportedDevices devtype;
    
#ifdef NO_USB_DEVICE  
  devtype = svidev::NoDevice;
#else
    
   _device = new SvMAX35101Evaluate(dinfo);
//   _device
   
#endif
    
    _dev = new SvMAX35101Evaluate(dinfo);
    connect(_dev, SIGNAL(new_data(svidev::MeasuredData )), this, SLOT(new_data(svidev::MeasuredData )));
    _dev->start(ui->spinTimer->value());
    
    emit newState(true);    

  }
  else
  {
    /** синхронизация с Arduino **/
    if(ui->gbSynchronizeArduino->isChecked()) {
      _arduino->stop();
    }

    /** ВНИМАНИЕ здесь вызывается деструктор ~SvPullUsb() **/  
    delete _dev; 
    _dev = nullptr;
    
#ifndef NO_USB_DEVICE
 
#endif
 
      emit newState(false);      

  }
}

void MainWindow::stateChanged(bool state)
{
  if(state) {
    ui->bnCycle->setText("Стоп");
    ui->bnCycle->setStyleSheet("background-color: tomato");
   
  }
  else {
   
    ui->bnCycle->setText("Старт");
    ui->bnCycle->setStyleSheet("background-color: palegreen;");
    
    on_bnSaveToFile_clicked(false);
  }
  
  ui->frame->setEnabled(!state);
  ui->bnOneShot->setEnabled(!state);
  ui->bnSaveToFile->setEnabled(state);
  ui->bnCycle->setEnabled(true);
  QApplication::processEvents();
  
}

void MainWindow::new_data(svidev::MeasuredData data)
{
  _dev->mutex.lock();

  /** вычисляем значения **/
  _calcs[svgraph::giTOFdiff] = data.tof1 - data.tof2;
  _calcs[svgraph::giVsnd] = 2 * L / ((data.tof1 + data.tof2) / 1000000000); // определяем скорость звука в среде, м/с.
  _calcs[svgraph::giVpot] = 3 * _calcs.value(svgraph::giVsnd) * (_calcs.value(svgraph::giTOFdiff) / (data.tof1 + data.tof2)); // определяем скорость потока, м/с.
  _calcs[svgraph::gitAvg] = (data.tof1 + data.tof2) / 2;
  _calcs[svgraph::giTemperature] = _arduino->currentTemperature();
  _calcs[svgraph::giTurnByMinute] = _arduino->currentTurnByMinute();
  _calcs[svgraph::giAngleBySecond] = _arduino->currentAngleBySecond();
  
  for(int i = 0; i < _chart->graphList().count(); i++) {
    
    svgraph::GraphIDs graph_id = _chart->graphList()[i];
    
    _chart->appendData(graph_id, _calcs[graph_id]);
    
    if(_file)
        _file->write((const char*)&_calcs[graph_id], sizeof(qreal));
    
  }
      
  _chart->customplot()->replot();

  if(ui->checkLog->isChecked())
    ui->textLog->append(QString("%1%2\tHit Up Avg: %3\tHit Down Avg: %4\tTOF diff: %5\tVpot: %6\tVsnd: %7")
                        .arg(_tick_count++)
                        .arg(QTime::currentTime().toString("mm:ss.zzz"))
                        .arg(data.tof1).arg(data.tof2).arg(_calcs.value(svgraph::giTOFdiff), 0, 'f', 6)
                        .arg(_calcs.value(svgraph::giVpot), 0, 'f', 3)
                        .arg(_calcs.value(svgraph::giVsnd), 0, 'f', 4));
    
  _dev->mutex.unlock();
  
}

void MainWindow::on_bnSaveToFile_clicked(bool checked)
{
    if(checked) {
        ui->bnSaveToFile->setText("Stop saving");
        ui->bnSaveToFile->setStyleSheet("background-color: tomato");

        svfnt::SvRE re(QDateTime::currentDateTime());
        re.relist << qMakePair(svfnt::RE_EXT, FILE_EXT);
        
        QString s = svfnt::get_file_path(ui->editSaveFilePath->text(), ui->editSaveFileNameTemplate->text(), re);
        if(s.isEmpty()) {
            QMessageBox::critical(0, "Error", "Неверный путь или имя файла для сохранения", QMessageBox::Ok);
            ui->bnSaveToFile->setChecked(false);
            return;
        }

//        s += ("." + FILE_EXT);

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
          /* заголовок файла - сигнатура и кол-во графиков */
          FileHeader file_head;
          
          file_head.graph_count = _chart->graphCount();
          file_head.start_x = _tick_count;
          _file->write((const char*)&file_head, sizeof(FileHeader));
          
          /* параметры графиков */
          for(int i = 0; i < _chart->graphList().count(); i++) {
            
            svgraph::GraphIDs graph_id = _chart->graphList()[i];
            
            GraphHeader graph_head;
            graph_head.graph_id = graph_id;
           
            graph_head.line_color = static_cast<quint32>(_chart->graphParams(graph_id).line_color.rgb());
            graph_head.line_style = _chart->graphParams(graph_id).line_style;
            graph_head.line_width = _chart->graphParams(graph_id).line_width;
            
            _file->write((const char*)&graph_head, sizeof(GraphHeader));
            
          }
        }

        MUTEX1.unlock();

        /* затем выводим сообщение об ошибке и выходим */
        if(!b) {
          log << svlog::Time << svlog::Critical << s << svlog::endl;
          ui->bnSaveToFile->setChecked(false);
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
        ui->bnSaveToFile->setStyleSheet("");
        
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
  svfnt::SvRE re(QDateTime::currentDateTime());
  re.relist << qMakePair(svfnt::RE_EXT, FILE_EXT);
  
  QDir dir(svfnt::get_folder_path(ui->editSaveFilePath->text(), re));
  if(!dir.exists())
    dir.setPath(QDir::currentPath());
  
  QString fn = QFileDialog::getOpenFileName(this, tr("Открыть файл"), dir.path(), "frs file (*.frs);;all files (*.*)");
  
  if(fn.isEmpty()) return;
  
  QFile f(fn);
  if(!f.open(QIODevice::ReadOnly)) {
    QMessageBox::critical(this, "Error", f.errorString(), QMessageBox::Ok);
    return;
  }
  
  /* читаем параметры */
  FileHeader header;
  f.read((char*)(&header), sizeof(FileHeader));

  int graph_cnt = header.graph_count;
  quint32 start_x = header.start_x;
  
  //  QByteArray b = f.read(15);
//  if(b != FileHeader.signature) {
//    f.close();
//    return;
//  }
  
//  f.read((char*)&graph_cnt, sizeof(int));
  
  /* создаем окно для вывода графиков */
  svchart::ChartParams p = _chart->params();
  svchart::SvChartWidget *chart = new svchart::SvChartWidget(p);
  
  /* добавляем графики */
  QList<svgraph::GraphIDs> graph_ids;
  for(int i = 0; i < graph_cnt; i++) {
    GraphHeader grah_head;
    f.read((char*)&grah_head, sizeof(GraphHeader));
    
    svgraph::GraphParams graph_params;
    graph_params.type = static_cast<svgraph::GraphIDs>(grah_head.graph_id);
    graph_params.line_color = QColor::fromRgb(static_cast<QRgb>(grah_head.line_color));
    graph_params.line_style = grah_head.line_style;
    graph_params.line_width = grah_head.line_width;
    
    chart->addGraph(graph_params.type, graph_params);
    
    graph_ids.append(graph_params.type);
  }
  
  /* начинаем читать данные графиков */
  int i = 0;
  while(!f.atEnd()) {
    qreal y;
   
    f.read((char*)&y, sizeof(qreal));
    
    chart->appendData(graph_ids[i++], y);
    
    i = i == graph_ids.count() ? 0 : i;
    
  }
  
  f.close();

  chart->show();
  chart->customplot()->replot();
  
}

void MainWindow::_addGraphToList(int graph_id, svgraph::GraphParams &p)
{
  QListWidgetItem *wi = new QListWidgetItem(svgraph::GraphTypes.value(p.type), ui->listGraphs);
  wi->setData(Qt::DecorationRole, p.line_color);
  wi->setData(Qt::UserRole, graph_id);
}

void MainWindow::on_bnAddGraph_clicked()
{
  svgraph::SvGraphParamsDialog* chDlg = new svgraph::SvGraphParamsDialog();
  
  if(chDlg->exec() == QDialog::Accepted)
  {
    svgraph::GraphParams p = chDlg->graph_params;
    
    svgraph::GraphIDs graph_id = p.type;
    
    /* если такой график уже есть, то ничего не добавляем */
    if(_chart->findGraph(graph_id)) 
      QMessageBox::information(this, "Info", "This type grap is already present", QMessageBox::Ok);
    
    else {
      _chart->addGraph(graph_id, p);
      _addGraphToList(graph_id, p);
    }
    
    ui->listGraphs->setCurrentItem(ui->listGraphs->findItems(svgraph::GraphTypes.value(graph_id), Qt::MatchExactly).first());
    
  }
  
  chDlg->~SvGraphParamsDialog();
  
  
}


void MainWindow::on_bnEditGraph_clicked()
{
 
  svgraph::GraphIDs graph_id = static_cast<svgraph::GraphIDs>(ui->listGraphs->currentIndex().data(Qt::UserRole).toInt());
  
  svgraph::GraphParams p = _chart->graphParams(graph_id);
  
  svgraph::SvGraphParamsDialog* chDlg = new svgraph::SvGraphParamsDialog(0, &p);
  
  if(chDlg->exec() == QDialog::Accepted)
  {
    p = chDlg->graph_params;
    _chart->setGraphParams(graph_id, p);
    
    ui->listGraphs->currentItem()->setData(Qt::DecorationRole, p.line_color);
    
  }
  
  chDlg->~SvGraphParamsDialog();
  
}

void MainWindow::on_bnRemoveGraph_clicked()
{
    svgraph::GraphIDs graph_id = static_cast<svgraph::GraphIDs> (ui->listGraphs->currentIndex().data(Qt::UserRole).toInt());
    
    _chart->removeGraph(graph_id);
    ui->listGraphs->takeItem(ui->listGraphs->currentRow());
    
    ui->bnEditGraph->setEnabled(ui->listGraphs->count() > 0);
    ui->bnRemoveGraph->setEnabled(ui->listGraphs->count() > 0);
}

void MainWindow::on_listGraphs_currentRowChanged(int currentRow)
{
  ui->bnEditGraph->setEnabled(ui->listGraphs->count() > 0);
  ui->bnRemoveGraph->setEnabled(ui->listGraphs->count() > 0);
}

void MainWindow::on_listGraphs_doubleClicked(const QModelIndex &index)
{
    on_bnEditGraph_clicked();
}

void MainWindow::on_spinTimer_editingFinished()
{
  svarduinomax::SvArduinoWidgetParams ap = _arduino->params();
  ap.state_period = ui->spinTimer->value();
  _arduino->setParams(ap);
}

void MainWindow::on_bnSaveBmp_clicked()
{
  svfnt::SvRE re(QDateTime::currentDateTime());
  re.relist << qMakePair(svfnt::RE_EXT, QString("bmp"));
  
  QString s = svfnt::get_file_path(ui->editSaveFilePath->text(), ui->editSaveFileNameTemplate->text(), re);
  if(s.isEmpty()) {
    log << svlog::Time << svlog::Critical << "Неверный путь или имя файла для сохранения" << svlog::endl;
    return;
  }

//  s += ".bmp";
  
    if(_chart->customplot()->saveBmp(s))
      log << svlog::Time << svlog::Info
          << QString("Файл %1 успешно сохранен.").arg(svfnt::replace_re(ui->editSaveFileNameTemplate->text(), re))
          << svlog::endl;
    else
      log << svlog::Time << svlog::Info
          << QString("Не удалось сохранить файл %1.").arg(svfnt::replace_re(ui->editSaveFileNameTemplate->text(), re))
          << svlog::endl;
}

void MainWindow::addNewDevice()
{
  // выбираем тип устройства
  svidev::SupportedDevices dev_type = svidev::VirtualDevice;
  
  SELECTDEVICETYPE_UI = new svidev::SvSelectDeviceTypeDialog();
  
  if(SELECTDEVICETYPE_UI->exec() == QDialog::Accepted)
    dev_type = SELECTDEVICETYPE_UI->type_id;
  
  else return;
  
  if(dev_type == svidev::VirtualDevice) {
    log << svlog::Critical << svlog::Time
        << QString("Нельзя добавить\"%1\"").arg(svidev::SupportedDevicesNames.value(dev_type))
        << svlog::endl;
  }
  
  switch (dev_type) {
    case svidev::MAX35101EV:
      
      break;
    default:
      break;
  }
  
  return result;
}
