#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../../svlib/sv_fnt.h"
//#include "../../svlib/sv_log.h"

QMutex MUTEX1;


MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  
  setWindowIcon(QIcon(":/icons/battery.ico"));
  setWindowTitle(QString("Измерение скорости потока v.%1").arg(APP_VERSION));
  
  ui->dockPlot->setWindowIcon(QIcon(":/icons/Chart2.ico"));
  ui->dockArduino->setWindowIcon(QIcon(":/icons/Gears.ico"));
  
  /* лог */
  log = svlog::SvLog(ui->textLog);
  
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
  ui->gbSynchronizeArduino->setChecked(AppParams::readParam(this, "General", "SynchronizeArduino", true).toBool());
  
  _chp.x_range = AppParams::readParam(this, "Chart", "x_range", 300).toInt();
  _chp.x_tick_count = AppParams::readParam(this, "Chart", "x_tick_count", 26).toInt();
  _chp.y_autoscale = AppParams::readParam(this, "Chart", "y_autoscale", false).toBool();
  _chp.y_range = AppParams::readParam(this, "Chart", "y_range", 5).toInt();
//  _chp.y_tick_count = AppParams::readParam(this, "Chart", "y_tick_count", 11).toInt();
//  _chp.line_color = QColor(AppParams::readParam(this, "Chart", "line_color", 0xFFFF0000).toUInt());
//  _chp.line_width = AppParams::readParam(this, "Chart", "line_width", 2).toInt();
//  _chp.show_TOF = ui->checkShowTOF->isChecked();

  _chart_w = new svchart::SvChartWidget(_chp, ui->dockPlot);
  ui->verticalLayout_7->addWidget(_chart_w);
//  _chart_w->show();
  
  /* параметры arduino */
  svarduinomax::SvArduinoWidgetParams ardp;
  ardp.ip = AppParams::readParam(this, "Arduino", "ip", "192.168.44.44").toString();
  ardp.port = AppParams::readParam(this, "Arduino", "port", 35580).toInt();
  ardp.spin_clockwise = AppParams::readParam(this, "Arduino", "spin_clockwise", true).toBool();
  ardp.engine_pw = AppParams::readParam(this, "Arduino", "engine_pw", 100).toInt();
  ardp.state_period = AppParams::readParam(this, "Arduino", "state_period", 1).toInt();
  ardp.state_period_enable = AppParams::readParam(this, "Arduino", "state_period_enable", true).toBool();
  ardp.turn_angle = AppParams::readParam(this, "Arduino", "turn_angle", 180).toInt();
  ardp.turn_angle_enable = AppParams::readParam(this, "Arduino", "turn_angle_enable", false).toBool();
  ardp.turn_count = AppParams::readParam(this, "Arduino", "turn_count", 1).toInt();
  ardp.turn_count_enable = AppParams::readParam(this, "Arduino", "turn_count_enable", false).toBool();
  ardp.current_voltage = AppParams::readParam(this, "Arduino", "current_voltage", 12).toInt();
//  ardp. = AppParams::readParam(this, "Arduino", "", );
  
  arduino = new svarduinomax::SvArduinoWidget(ardp, ui->textLog, 0);
  ui->verticalLayout_9->addWidget(arduino);
  
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

    _chart_w->addGraph(p.type, p);
    _addGraphToList(p.type, p);
    
  }
  
  if(ui->listGraphs->count() > 0)
    ui->listGraphs->setCurrentRow(0);
  
  /* готовим карту для вычисляемых значений */
  foreach (svgraph::GraphIDs key, svgraph::GraphTypes.keys())
    _calcs.insert(key, 0);

  connect(this, SIGNAL(newState(bool)), this, SLOT(stateChanged(bool)));
  
  
}

MainWindow::~MainWindow()
{
  /* завершаем работу с платой */
//  if(_thr)
//  {

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
  
//  AppParams::saveParam(this, "Chart", "ViewType", ui->cbViewType->currentIndex());
  AppParams::saveParam(this, "Chart", "Autoscale", _chart_w->chartParams().y_autoscale);
  AppParams::saveParam(this, "Chart", "x_range", _chart_w->chartParams().x_range);
//  AppParams::saveParam(this, "Chart", "ShowTOF", ui->checkShowTOF->isChecked());
  
  /* сохраняем список графиков */
  AppParams::saveParam(this, "Chart", "GraphCount", _chart_w->graphCount());
  
  for (int i = 0; i < _chart_w->graphList().count(); i++) {
    svgraph::GraphIDs graph_id = _chart_w->graphList()[i];
    AppParams::saveParam(this, QString("Graph_%1").arg(i), "TypeID", int(graph_id));
    AppParams::saveParam(this, QString("Graph_%1").arg(i), "LineColor", _chart_w->graphParams(graph_id).line_color.name());
    AppParams::saveParam(this, QString("Graph_%1").arg(i), "LineStyle", _chart_w->graphParams(graph_id).line_style);
    AppParams::saveParam(this, QString("Graph_%1").arg(i), "LineWidth", _chart_w->graphParams(graph_id).line_width);
    AppParams::saveParam(this, QString("Graph_%1").arg(i), "LineLegend", _chart_w->graphParams(graph_id).legend);
  }
  
  /* параметры arduino */
  AppParams::saveParam(this, "Arduino", "ip", arduino->params().ip);
  AppParams::saveParam(this, "Arduino", "port", arduino->params().port);
  AppParams::saveParam(this, "Arduino", "spin_clockwise", arduino->params().spin_clockwise);
  AppParams::saveParam(this, "Arduino", "engine_pw", arduino->params().engine_pw);
  AppParams::saveParam(this, "Arduino", "state_period", arduino->params().state_period);
  AppParams::saveParam(this, "Arduino", "state_period_enable", arduino->params().state_period_enable);
  AppParams::saveParam(this, "Arduino", "turn_angle", arduino->params().turn_angle);
  AppParams::saveParam(this, "Arduino", "turn_angle_enable", arduino->params().turn_angle_enable);
  AppParams::saveParam(this, "Arduino", "turn_count", arduino->params().turn_count);
  AppParams::saveParam(this, "Arduino", "turn_count_enable", arduino->params().turn_count_enable);
  AppParams::saveParam(this, "Arduino", "current_voltage", arduino->params().current_voltage);
  
  
  delete ui;
  
}

void MainWindow::on_bnGetDeviceList_clicked()
{
  _available_devices = QSerialPortInfo::availablePorts();
    
  for(QSerialPortInfo port: _available_devices)
    ui->cbDevices->addItem(port.portName());
  
}

void MainWindow::on_bnOneShot_clicked()
{
  
  _serial = new QSerialPort(_available_devices.at(ui->cbDevices->currentIndex()));
  
  if (!_serial)
  {
    log << svlog::Time << svlog::Critical << "Ошибка при создании объекта." 
        << _serial->errorString() << svlog::endl;;
    return;
  }
  
  if(!_serial->open(QIODevice::ReadWrite)) {
    log << svlog::Time << svlog::Critical
        << QString("Не удалось открыть порт %1").arg(_available_devices.at(ui->cbDevices->currentIndex()).portName())
        << svlog::endl;
    return;
  }
  
  TDC1000::qres res = TDC1000::pullTDC1000(_serial);
  
  if(res.result)
    emit new_data(res.answer);

  _serial->close();
  delete _serial;
  _serial = nullptr;
  
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
  
  /** синхронизация с Arduino **/
  if(ui->gbSynchronizeArduino->isChecked()) {
    
    if(!arduino->start()) {
      emit newState(false);
      return;
    }
  }
  
  
  if(!_tdc100thr)
  {
    _serial = new QSerialPort(_available_devices.at(ui->cbDevices->currentIndex()));
    
    if (!_serial)
    {
      log << svlog::Time << svlog::Critical << "Ошибка при создании устройства." 
          << _serial->errorString() << svlog::endl;;
      return;
    }
    
    if(!_serial->open(QIODevice::ReadWrite)) {
      log << svlog::Time << svlog::Critical
          << QString("Не удалось открыть порт %1").arg(_available_devices.at(ui->cbDevices->currentIndex()).portName())
          << svlog::endl;
      return;
    }
    
    _tdc100thr = new SvPullTDC1000(_serial, ui->spinTimer->value());
    connect(_tdc100thr, SIGNAL(newData(QByteArray&)), this, SLOT(new_data(QByteArray&)));
    _tdc100thr->start();
    
    emit newState(true);    

  }
  else
  {
    
    _tdc100thr->stop();
    _tdc100thr->deleteLater();
    delete _tdc100thr;
    _tdc100thr = nullptr;      
    
    if(_serial) {
      if(_serial->isOpen()) _serial->close();
      delete _serial;
      _serial = nullptr;
    }
    
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
    ui->bnCycle->setStyleSheet("");
    
    on_bnSaveToFile_clicked(false);
  }
  
  ui->frame->setEnabled(state);
  ui->bnOneShot->setEnabled(!state);
  ui->bnSaveToFile->setEnabled(state);
  ui->bnCycle->setEnabled(true);
  QApplication::processEvents();
  
}

void MainWindow::new_data(QByteArray &data)
{
  MUTEX1.lock();

  memcpy(&_tdc1000_data, data.data(), sizeof(TDC1000::TDC1000_ANSWER));
  
  /** ******************************* **/
  qreal t1 = qFromBigEndian<qint16>(_tdc1000_data.time1); // / 262.14;   // время пролета 1, в нс.
  qreal t2 = qFromBigEndian<qint16>(_tdc1000_data.time2); // / 262.14; // время пролета 2, в нс.
  
  qreal L = 0.0895; // расстояние между излучателями в метрах

  /** вычисляем значения **/
  _calcs[svgraph::giTOFdiff] = t1 - t2;
  _calcs[svgraph::giVsnd] = 2 * L / ((t1 + t2) / 1000000000); // определяем скорость звука в среде, м/с.
  _calcs[svgraph::giVpot] = 3 * _calcs.value(svgraph::giVsnd) * (_calcs.value(svgraph::giTOFdiff) / (t1 + t2)); // определяем скорость потока, м/с.
  _calcs[svgraph::gitAvg] = (t1 + t2) / 2;
  _calcs[svgraph::giTemperature] = arduino->currentTemperature();
  
  for(int i = 0; i < _chart_w->graphList().count(); i++) {
    
    svgraph::GraphIDs graph_id = _chart_w->graphList()[i];
    
    _chart_w->appendData(graph_id, _calcs[graph_id]);
    
    if(_file)
        _file->write((const char*)&_calcs[graph_id], sizeof(qreal));
    
  }
      
  _chart_w->customplot()->replot();

  if(ui->checkLog->isChecked()) {
    log.setSeparator('\t');
    log << _chart_w->pointCount() << svlog::TimeZZZ << svlog::Info
        << QString("t1: %1").arg(t1, 0, 'f', 3)
        << QString("t2: %1").arg(t2, 0, 'f', 3)
        << QString("TOF diff: %1").arg(_calcs.value(svgraph::giTOFdiff), 0, 'f', 3)
        << QString("Vpot: %1").arg(_calcs.value(svgraph::giVpot), 0, 'f', 3)
        << QString("Vsnd: %1").arg(_calcs.value(svgraph::giVsnd), 0, 'f', 3)
        << svlog::endl;
                        
    log.setSeparator(' ');
  }
    
  MUTEX1.unlock();
    
}


/** ******************************* **/
void SvPullTDC1000::run()
{
  _started = true;
  _finished = false;
  
  while(_started)
  {
    MUTEX1.lock();
    TDC1000::qres result = TDC1000::pullTDC1000(_serial);
    MUTEX1.unlock();
    
    if(result.result)
      emit new_data(result.answer); 
    
    msleep(_timeout);
    
  }
  
  _finished = true;
  
}

/** ****************************************** **/

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

        s += FILE_EXT;

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
          
          file_head.graph_count = _chart_w->graphCount();
          _file->write((const char*)&file_head, sizeof(FileHeader));
          
          /* параметры графиков */
          for(int i = 0; i < _chart_w->graphList().count(); i++) {
            
            svgraph::GraphIDs graph_id = _chart_w->graphList()[i];
            
            GraphHeader graph_head;
            graph_head.graph_id = graph_id;
           
            graph_head.line_color = static_cast<quint32>(_chart_w->graphParams(graph_id).line_color.rgb());
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
  
  QByteArray b = f.read(15);
//  if(b != FileHeader.signature) {
//    f.close();
//    return;
//  }
  
  /* создаем окно для вывода графиков */
  svchart::ChartParams p = _chart_w->chartParams();
  svchart::SvChartWidget *chart = new svchart::SvChartWidget(p);
  
  /* читаем количество графиков */
  int graph_cnt;
  f.read((char*)&graph_cnt, sizeof(int));
  
//  qDebug() << 
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
    if(_chart_w->findGraph(graph_id)) 
      QMessageBox::information(this, "Info", "This type grap is already present", QMessageBox::Ok);
    
    else {
      _chart_w->addGraph(graph_id, p);
      _addGraphToList(graph_id, p);
    }
    
    ui->listGraphs->setCurrentItem(ui->listGraphs->findItems(svgraph::GraphTypes.value(graph_id), Qt::MatchExactly).first());
    
  }
  
  chDlg->~SvGraphParamsDialog();
  
  
}


void MainWindow::on_bnEditGraph_clicked()
{
 
  svgraph::GraphIDs graph_id = static_cast<svgraph::GraphIDs>(ui->listGraphs->currentIndex().data(Qt::UserRole).toInt());
  
  svgraph::GraphParams p = _chart_w->graphParams(graph_id);
  
  svgraph::SvGraphParamsDialog* chDlg = new svgraph::SvGraphParamsDialog(0, &p);
  
  if(chDlg->exec() == QDialog::Accepted)
  {
    p = chDlg->graph_params;
    _chart_w->setGraphParams(graph_id, p);
    
    ui->listGraphs->currentItem()->setData(Qt::DecorationRole, p.line_color);
    
  }
  
  chDlg->~SvGraphParamsDialog();
  
}

void MainWindow::on_bnRemoveGraph_clicked()
{
    svgraph::GraphIDs graph_id = static_cast<svgraph::GraphIDs> (ui->listGraphs->currentIndex().data(Qt::UserRole).toInt());
    
    _chart_w->removeGraph(graph_id);
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
