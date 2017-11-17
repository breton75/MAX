#include "sv_device_interface.h"


SvSQLITE *SQLITE;
SvSelectDeviceTypeDialog *SELECTDEVICETYPE_UI;

QSqlError openDb(QString dbFileName)
{
  /* читаем БД (план полета) */
  SQLITE = new SvSQLITE(this, dbFileName);
  
  return SQLITE->connectToDB();
  
//  if(err.type() != QSqlError::NoError) {
//    qDebug() << err.databaseText();
//  }
}

QSqlError fillDeviceList(QComboBox *cb)
{
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  
  QSqlError err = SQLITE->execSQL(QString("select * from devices"), q);
  
  if(err.type() == QSqlError::NoError) {
  
    cb->clear();
    
    while(q->next())
      cb->addItem(q->value("device_name").toString(), q->value("device_type").toInt());
    
  }
 
  q->finish();
  delete q;
  
  return err;
  
}

QString fillDeviceInfo(int dbid, svidev::DeviceInfo &dinfo)
{
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  
  QSqlError err = SQLITE->execSQL(QString("select * from devices where dbid = %1").arg(dbid), q);
  
  if(err.type() == QSqlError::NoError) {
    
    q->first();
    
    dinfo.dbid = dbid;
    dinfo.deviceType = q->value("device_type");
    dinfo.idProduct = q->value("product_id");
    dinfo.idVendor = q->value("vendor_id");
    dinfo.iManufacturer = q->value("manufacturer_id");
    dinfo.name = q->value("device_name");
    dinfo.port = q->value("port");
                       
  } 
  
  q->finish();
  delete q;
  
  return err;
  
}



/** ---------------  ---------------- **/

SvDeviceEditor::SvDeviceEditor(QTextEdit *logWidget, int dbid, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SvDeviceEditorDialog)
{
  ui->setupUi(this);
  
  _log = svlog::SvLog(logWidget, this);
  
  _showMode = dbid == -1 ? smNew : smEdit;
  
  ui->cbDeviceType->clear();
  for(svidev::SupportedDevices key: svidev::SupportedDevicesNames.keys())    
    ui->cbDeviceType->addItem(svidev::SupportedDevicesNames.value(key), int(key));
  
  if(_showMode == smEdit) {
    
    QSqlQuery* q = new QSqlQuery(SQLITE->db);
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_DEVICE).arg(dbid), q).type()) {
      
      q->finish();
      delete q;
      return;
      
    }
   
    if(q->next()) {
      
      dinfo.dbid = dbid;
      dinfo.deviceType = q->value("device_type").toUInt();
      dinfo.idProduct = q->value("product_id").toUInt();
      dinfo.idVendor = q->value("vendor_id").toUInt();
      dinfo.iManufacturer = q->value("manufacturer_id").toUInt();
      dinfo.port = q->value("port").toUInt();
      
    }
    
    q->finish();
    delete q;
    
    ui->cbDeviceType->setCurrentIndex(ui->cbDeviceType->findData(int(dinfo.deviceType)));
    ui->editID->setText(QString::number(dinfo.dbid));
  }
  
//  ui->dateTimeEdit->setDateTime(t_date_time);
//  ui->editUID->setText(t_uid);
//  ui->textDescription->setText(t_description);
  
  connect(ui->cbDeviceType, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrentConnectionType(int)));
  connect(ui->bnSave, SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui->bnCancel, SIGNAL(clicked()), this, SLOT(reject()));
  
  this->setModal(true);
  this->show();
}

SvDeviceEditor::~SvDeviceEditor()
{
  delete ui;
}

void SvDeviceEditor::accept()
{

  bool result = false;
  
  switch (_showMode) {
    
    case smNew: {
      
      QSqlError err = SQLITE->execSQL(QString(SQL_INSERT_DEVICE)
                                      .arg(dinfo.deviceType)
                                      .arg(dinfo.idVendor)
                                      .arg(dinfo.idProduct)
                                      .arg(dinfo.iManufacturer)
                                      .arg(dinfo.port)
                                      .arg(QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss"))
                                      .arg(QString("%1_%2%3")));
      
      if(QSqlError::NoError != err.type()) {
        
        _log << svlog::Critical << svlog::Time
             << "Ошибка при добавлении нового устройства:\n"
             << err.databaseText() << svlog::endl;
        break;
      }
      
      QSqlQuery* q = new QSqlQuery(SQLITE->db);
      err = SQLITE->execSQL(QString("select id from devices order by id desc limit 1"), q);
      
      if(QSqlError::NoError != err.type()) {
        
        q->finish();
        delete q;
        
        _log << svlog::Critical << svlog::Time
             << "Ошибка при добавлении нового устройства:\n"
             << err.databaseText() << svlog::endl;
        
        break;
        
      }

      q->first();
      dinfo.dbid = q->value("id").toInt();
      
      q->finish();
      delete q;

      result = true;
      break;
      
    }
      
//    case smEdit:
//    {
//      if(QSqlError::NoError != SQLITE->execSQL(QString("update plan set uid='%1', lon=%2, lat=%3, date_time='%4', description='%5' where id=%6")
//                                             .arg(t_uid)
//                                             .arg(t_lon)
//                                             .arg(t_lat)
//                                             .arg(t_date_time.toString("dd/MM/yyyy hh:mm:ss"))
//                                             .arg(t_description)
//                                             .arg(t_id)).type())
//      {
//        QMessageBox::critical(this, "Редактирование", "Не удалось обновить запись в журнале местных буев", QMessageBox::Ok);
//        break;
//      }      
      
//      result = true;
//      break;
//    }
  }

  if(result) QDialog::accept();
  else QDialog::reject();
  
}

void SvDeviceEditor::setCurrentConnectionType(int index)
{
  svidev::SupportedDevices dt = static_cast<svidev::SupportedDevices>(ui->cbDeviceType->currentData().toInt());
  
  switch (dt) {
    case svidev::NoDevice:
      _curConnectionType = "VIRTUAL";
      break;
      
    case svidev::MAX35101EV:
      _curConnectionType = "USB";
      break;
      
    case svidev::TDC1000_TDC7200EVM:
    case svidev::AMEBuoy:
      _curConnectionType = "COM";
      break;
      
    default:
      _curConnectionType = "";
      break;
  }
}


/** ---------  ------------ **/
SvSelectDeviceTypeDialog::SvSelectDeviceTypeDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SvSelectDeviceTypeDialog)
{
  ui->setupUi(this);
  
  for(svidev::SupportedDevices key: svidev::SupportedDevicesNames.keys())
    ui->cbDeviceType->addItem(svidev::SupportedDevicesNames.value(key), int(key));
  
  connect(bnSelect, SIGNAL(pressed()), this, SLOT(accept()));
  connect(bnCancel, SIGNAL(pressed()), this, SLOT(reject()));
  
  setModal(true);
  show();
  
}

void SvSelectDeviceTypeDialog::accept()
{
  type_id = static_cast<svidev::SupportedDevices>(ui->cbDeviceType->currentData().toInt());
  
  QDialog::accept();
  
}
