#include "sv_arduino_max.h"
#include "ui_engine_control.h"

svarduinomax::SvArduinoWidget::SvArduinoWidget(SvArduinoWidgetParams params,
                                               QTextEdit *logWidget,
                                               QWidget *parent) : 
  QWidget(parent),
  ui(new Ui::SvArduinoWidgetUi)
{
  ui->setupUi(this);

  ui->editCmd->setVisible(false);
  
  _log = svlog::SvLog(logWidget, this);
  
  setParams(params);
  
  _client = new svtcp::SvTcpClient(_params.ip, _params.port, logWidget, svtcp::LogInData | svtcp::LogOutData, parent);
  
  connect(this, SIGNAL(newState(bool)), this, SLOT(stateChanged(bool)));
  
  if(_params.state_period_enable) {
    _state_timer = new QTimer(this);
    _state_timer->setSingleShot(false);
    connect(_state_timer, SIGNAL(timeout()), this, SLOT(pullSensors()));
  }
  
}

void svarduinomax::SvArduinoWidget::setParams(svarduinomax::SvArduinoWidgetParams params)
{ 
  _params = params;
  
  ui->editIp->setText(_params.ip);
  ui->spinPort->setValue(_params.port);
  ui->rbClockwise->setChecked(_params.spin_clockwise);
  ui->rbContraClockwise->setChecked(!_params.spin_clockwise);
  ui->sliderEnginePw->setValue(_params.engine_pw);
  ui->gbTurnAngle->setChecked(_params.turn_angle_enable);
  ui->spinTurnAngle->setValue(_params.turn_angle);
  ui->gbTurnCount->setChecked(_params.turn_count_enable);
  ui->spinTurnCount->setValue(_params.turn_count);
  ui->gbState->setChecked(_params.state_period_enable);
  ui->spinStatePeriod->setValue(_params.state_period);
  ui->spinVoltage->setValue(_params.current_voltage);
}

void svarduinomax::SvArduinoWidget::on_bnStart_clicked()
{
  if(!_current_state) 
    start();
}

void svarduinomax::SvArduinoWidget::on_bnStop_clicked()
{
    stop();    
}

bool svarduinomax::SvArduinoWidget::start()
{
  foreach (QWidget* wdg, this->findChildren<QWidget *>())
    wdg->setEnabled(false);
  
  QApplication::processEvents();
  
  try {
  
    _client->setIp(_params.ip);
    _client->setPort(_params.port);  
    
    QString cmd = "";
    cmd = QString("SET:ANGLE:%1;").arg(_params.turn_angle_enable ? QString::number(_params.turn_angle) : "OFF");
    cmd += QString("SET:TURN:%1;").arg(_params.turn_count_enable ? QString::number(_params.turn_count) : "OFF");
    cmd += QString("SET:ENGINE:%1;").arg(int(255 * _params.engine_pw/100));
    cmd += QString("SET:DIRECTION:%1;").arg(_params.spin_clockwise ? "CLOCKWISE" : "BACKWISE");
    cmd += QString("START");
    
#ifndef NO_ARDUINO

    if(_client->connectToHost() != svtcp::SOCKET_OK) _exception.raise(_client->lastError());
    _client->sendData(cmd, 2000);
    
    if(_client->response()->status != svtcp::SOCKET_OK)
      _exception.raise(_client->lastError());
    
    if(_client->response()->data.contains(QByteArray("UNKNOWN_COMMAND")))   
      _exception.raise(QString("Неверная команда: %1").arg(QString(_client->response()->data).replace("UNKNOWN_COMMAND:", "")));
    
#endif
    
  }
  
  catch (SvException &e) {
    emit newState(false);
    _log << svlog::Time << svlog::Critical << e.err << svlog::endl;
    return false;
  }
  
  _last_time = time(NULL);
  _last_turn = 0;
  _last_angle = 0;
  _current_turn_by_minute = 0;
  _current_angle_by_second = 0;
  
  _state_timer->setInterval(_params.state_period); // * 1000);
  _state_timer->start();
  
  emit newState(true);    
  return true;
  
}

bool svarduinomax::SvArduinoWidget::stop()
{
  foreach (QWidget* wdg, this->findChildren<QWidget *>())
    wdg->setEnabled(false);
  
  QApplication::processEvents();
  
  _state_timer->stop();
  
  while(_is_pulling) QApplication::processEvents();
  
  try {

#ifndef NO_ARDUINO
    
    if(!_client || !_client->connected()) 
      return false;
    
    _client->sendData(QString("STOP"));
    _client->disconnectFromHost();
    
#endif
    
  }
  
  catch (SvException &e) {
    _log << svlog::Time << svlog::Error << e.err << svlog::endl;
  }
  
//  _mux.unlock();
    
  emit newState(false);
  
  return true;
  
}

void svarduinomax::SvArduinoWidget::pullSensors()
{
  try {
  
#ifdef NO_ARDUINO
    
    QStringList l = QString("CURRENT:TEMP:20;CURRENT:ENCODER:%1").arg(ui->sliderEnginePw->maximum() - _params.engine_pw + rand()%2 + 100).split(';');
    
#else

    if(!_client || !_client->connected()) 
      return;
    
    _is_pulling = true;
    
    int flags = _client->logFlags();
    _client->setFlags(svtcp::NoLog);
    
    QString cmd = QString("SET:ENGINERT:%1;").arg(int(255 * _params.engine_pw/100));
//    cmd += QString("SET:DIRECTION:%1;").arg(_params.spin_clockwise ? "CLOCKWISE" : "BACKWISE");
    cmd += "STATE";
    
    _client->sendData(QString(cmd), 1000);
    _client->setFlags(flags);
    
    if(_client->response()->status != svtcp::SOCKET_OK)
      _exception.raise(_client->lastError());
    
    QStringList l = QString(_client->response()->data).split(';');
    
    _is_pulling = false;
    
#endif
    
    
    time_t cur_time = time(NULL);
    
    for(QString state: l) {
      bool ok;
      state = state.trimmed();
      
      if(state.startsWith("STOPPED"))
        _exception.raise("Двигатель остановлен");
      
      if(state.startsWith("CURRENT:TEMP:")) {
        QStringList v = state.split(':');
        _current_temperature = static_cast<QString>(v.last()).toFloat(&ok);
      }
      
//      else if(state.startsWith("CURRENT:ANGLE:")) {
//        QStringList v = state.split(':');
//        _current_angle = static_cast<QString>(v.last()).toUInt(&ok);
        
////        qDebug() << _current_angle << _last_angle << (cur_time - _last_time);
        
//        if(_last_angle) {
//          _current_angle_by_second = (_current_angle - _last_angle) / (cur_time - _last_time);
//          _current_turn_by_minute = (((_current_angle - _last_angle) / (cur_time - _last_time)) * 60) / 360;
//        }
        
//        _last_angle = _current_angle;
//        _last_time = cur_time;
//      }
      
//      else if(state.startsWith("CURRENT:TURN:")) {
//        QStringList v = state.split(':');
//        _current_turn = static_cast<QString>(v.last()).toUInt(&ok);
//      }
      
      else if(state.startsWith("CURRENT:ENCODER:")) {
        QStringList v = state.split(':');
        quint32 _encoder = static_cast<QString>(v.last()).toUInt(&ok);

        if(!ok) continue;
          
        _current_angle_by_second = (1000.0 / qreal(_encoder)) * 18.0;
        
#ifdef NO_ARDUINO
        _current_angle_by_second /= 200;
#endif
          
          _current_turn_by_minute =  _current_angle_by_second * 60 / 360;
          _current_encoder = _encoder / 100.0;
//        }
        
//        _last_encoder = _current_encoder;
//        _last_time = cur_time;
      }
    }
  
  }
  
  catch (SvException &e) {
    _log << svlog::Time << svlog::Error << e.err << svlog::endl;
    _state_timer->stop();
    _mux.unlock();
    return;
  }
  
  ui->textLog->setText(QString("Текущее состояние:\n  температура: %1\n  угол/сек.: %2\n  оборот/мин: %3\n  энкодер: %4")
                       .arg(_current_temperature).arg(_current_angle_by_second).arg(_current_turn_by_minute).arg(_current_encoder * 100));
  
//  _log << svlog::Time << svlog::Info << "Темп." << _current_temperature
//       << "Угол" << _current_angle << svlog::endl;
  
}

void svarduinomax::SvArduinoWidget::stateChanged(bool state)
{
  _current_state = state;
  
  foreach (QWidget* wdg, this->findChildren<QWidget *>())
    wdg->setEnabled(!_current_state);
  
  ui->gbSpinSpeed->setEnabled(true);
  ui->sliderEnginePw->setEnabled(true);

  if(_state_timer) {
    _state_timer->setInterval(_params.state_period); // * 1000);
    if(_current_state) _state_timer->start();
    else _state_timer->stop();
  }
  
  ui->bnStop->setEnabled(true);
  ui->bnSendCmd->setEnabled(true);
  ui->editCmd->setEnabled(true);
  
  QApplication::processEvents();
    
}

void svarduinomax::SvArduinoWidget::on_rbClockwise_clicked(bool checked)
{
    _params.spin_clockwise = checked;
}

void svarduinomax::SvArduinoWidget::on_rbContraClockwise_clicked(bool checked)
{
    _params.spin_clockwise = !checked;
}

void svarduinomax::SvArduinoWidget::on_sliderEnginePw_valueChanged(int value)
{
    _params.engine_pw = value;
}

void svarduinomax::SvArduinoWidget::on_gbTurnAngle_toggled(bool arg1)
{
  _params.turn_angle_enable = arg1;
}

void svarduinomax::SvArduinoWidget::on_spinTurnAngle_valueChanged(int arg1)
{
    _params.turn_angle = arg1;
}

void svarduinomax::SvArduinoWidget::on_gbTurnCount_clicked(bool checked)
{
    _params.turn_count_enable = checked;
}

void svarduinomax::SvArduinoWidget::on_spinTurnCount_valueChanged(int arg1)
{
    _params.turn_count = arg1;
}

void svarduinomax::SvArduinoWidget::on_gbState_clicked(bool checked)
{
    _params.state_period_enable = checked;
}

void svarduinomax::SvArduinoWidget::on_spinStatePeriod_valueChanged(int arg1)
{
    _params.state_period = arg1;
}

void svarduinomax::SvArduinoWidget::on_bnSendCmd_clicked()
{
  ui->editCmd->setVisible(true);
  ui->bnSendCmd->setVisible(false);
}

void svarduinomax::SvArduinoWidget::on_editCmd_returnPressed()
{
  _client->setIp(_params.ip);
  _client->setPort(_params.port);
  
  _client->connectToHost();
  if(!_client->connected()) return;
  _client->sendData(ui->editCmd->text(), 1000);
  if(_client->response()->data.contains(QByteArray("UNKNOWN_COMMAND"))) {
    _log << svlog::Time << svlog::Critical << QString("Неверная команда: %1")
            .arg(QString(_client->response()->data).replace("UNKNOWN_COMMAND:", "")) << svlog::endl;
    
  }
  
  _client->disconnectFromHost();
}

void svarduinomax::SvArduinoWidget::on_bnApply_clicked()
{
    
}

void svarduinomax::SvArduinoWidget::on_editIp_textChanged(const QString &arg1)
{
    _params.ip = arg1;
}

void svarduinomax::SvArduinoWidget::on_spinPort_valueChanged(int arg1)
{
    _params.port = arg1;
}

void svarduinomax::SvArduinoWidget::on_editCmd_editingFinished()
{
  ui->editCmd->setVisible(false);
  ui->bnSendCmd->setVisible(true);
}

