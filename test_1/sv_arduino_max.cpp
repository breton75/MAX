#include "sv_arduino_max.h"
#include "ui_engine_control.h"

svarduinomax::SvArduinoWidget::SvArduinoWidget(SvArduinoWidgetParams params,
                                               QTextEdit *logWidget,
                                               QWidget *parent) : 
  QWidget(parent),
  ui(new Ui::SvArduinoWidgetUi)
{
  _params = params;
  
  ui->setupUi(this);

  ui->editCmd->setVisible(false);
  
  _log = svlog::SvLog(logWidget, this);
  
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
  
  _client = new svtcp::SvTcpClient(_params.ip, _params.port, logWidget, svtcp::LogInData | svtcp::LogOutData, parent);
  
  connect(this, SIGNAL(newState(bool)), this, SLOT(stateChanged(bool)));
  
  if(_params.state_period_enable) {
    _state_timer = new QTimer(this);
    _state_timer->setSingleShot(false);
    connect(_state_timer, SIGNAL(timeout()), this, SLOT(pullSensors()));
  }
  
}

void svarduinomax::SvArduinoWidget::on_bnStart_clicked()
{
  if(!_started) 
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
  
  bool result = true;
  try {
  
    _client->setIp(_params.ip);
    _client->setPort(_params.port);  
    
    QString cmd = "";
    cmd = QString("SET:ANGLE:%1;").arg(_params.turn_angle_enable ? QString::number(_params.turn_angle) : "OFF");
    cmd += QString("SET:TURN:%1;").arg(_params.turn_count_enable ? QString::number(_params.turn_count) : "OFF");
    cmd += QString("SET:ENGINE:%1;").arg(int(255 * _params.engine_pw/100));
    cmd += QString("SET:DIRECTION:%1;").arg(_params.spin_clockwise ? "CLOCKWISE" : "ANTICLOCKWISE");
    cmd += QString("START");
    
    if(_client->connectToHost() != svtcp::SOCKET_OK) _exception.raise(_client->lastError());
    _client->sendData(cmd, 2000);
    if(_client->response()->data.contains(QByteArray("UNKNOWN_COMMAND")))   
      _exception.raise(QString("Неверная команда: %1").arg(QString(_client->response()->data).replace("UNKNOWN_COMMAND:", "")));
    
  }
  
  catch (SvException &e) {
    result = false;
    _log << svlog::Time << svlog::Critical << e.err << svlog::endl;
  }
  
//  _client->disconnectFromHost();
  
  emit newState(true);    
  return result;
  
}

bool svarduinomax::SvArduinoWidget::stop()
{
  foreach (QWidget* wdg, this->findChildren<QWidget *>())
    wdg->setEnabled(false);
  
  QApplication::processEvents();
  try {
  
    if(!_client->connected()) {
      if(_client->connectToHost() != svtcp::SOCKET_OK)
        _exception.raise(_client->lastError());
    }
    
    _client->sendData(QString("STOP"), 2000);
    
  }
    
  catch (SvException &e) {
    _log << svlog::Time << svlog::Critical << e.err << svlog::endl;
  }
  
  _client->disconnectFromHost();
    
  emit newState(false);
  
  return true;
  
}

void svarduinomax::SvArduinoWidget::pullSensors()
{
  try {
  
    _client->setIp(_params.ip);
    _client->setPort(_params.port);  
    
    if(!_client->connected()) {
      if(_client->connectToHost() != svtcp::SOCKET_OK)
        _exception.raise(_client->lastError());
    }
    
    int flags = _client->logFlags();
    _client->setFlags(svtcp::NoLog);
    _client->sendData(QString("STATE"), 1000);
    _client->setFlags(flags);
    
    bool ok;
    QStringList l = QString(_client->response()->data).split(';');
    
    for(QString state: l) {
      state = state.trimmed();
      
      if(state.startsWith("STOPPED")) {
        stop();
      }
      
      if(state.startsWith("CURRENT:TEMP:")) {
        QStringList v = state.split(':');
        _current_temperature = static_cast<QString>(v.last()).toFloat(&ok);
      }
      
      else if(state.startsWith("CURRENT:ANGLE:")) {
        QStringList v = state.split(':');
        _current_angle = static_cast<QString>(v.last()).toUInt(&ok);
      }
      
      else if(state.startsWith("CURRENT:TURN:")) {
        QStringList v = state.split(':');
        _current_turn = static_cast<QString>(v.last()).toUInt(&ok);
      }
    }
  }
  
  catch (SvException &e) {
    _log << svlog::Time << svlog::Critical << e.err << svlog::endl;
    return;
  }
  
  ui->textLog->setText(QString("Текущее состояние:\n  температура: %1\n  угол: %2\n  оборот: %3")
                       .arg(_current_temperature).arg(_current_angle).arg(_current_turn));
  
//  _log << svlog::Time << svlog::Info << "Темп." << _current_temperature
//       << "Угол" << _current_angle << svlog::endl;
  
}

void svarduinomax::SvArduinoWidget::stateChanged(bool started)
{
  _started = started;
  
  foreach (QWidget* wdg, this->findChildren<QWidget *>())
    wdg->setEnabled(!_started);
  
  ui->bnStop->setEnabled(true);
  ui->bnSendCmd->setEnabled(true);
  ui->editCmd->setEnabled(true);
  
  if(_state_timer) {
    _state_timer->setInterval(_params.state_period * 1000);
    if(_started) _state_timer->start();
    else _state_timer->stop();
  }
  
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

