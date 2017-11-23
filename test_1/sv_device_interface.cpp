#include "sv_device_interface.h"

extern SvSQLITE *SQLITE;
svidev::SvSelectDeviceType *SELECTDEVICETYPE_UI;


QSqlError svidev::open_db(QString dbFileName)
{
  /* читаем БД (план полета) */
  SQLITE = new SvSQLITE(0, dbFileName);
  
  QSqlError err = SQLITE->connectToDB();
  
  if(err.type() != QSqlError::NoError)
    return err;
  
  /// проверяем присутствие таблицы devices (в случае когда был создан новый файл)
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  err = SQLITE->execSQL("SELECT name FROM sqlite_master WHERE type='table' AND name='devices'", q);
  
  if(err.type() == QSqlError::NoError) {
     
    /// если таблицы нет, то создаем ее
    if(!q->next()) {
      
      err = SQLITE->execSQL(SQL_CREATE_TABLE_DEVICES);
      
      // добавляем виртуальное устройство
      if(err.type() == QSqlError::NoError) {
        
        err = SQLITE->execSQL(QString(SQL_NEW_DEVICE) 
                                        .arg(int(svidev::VirtualDevice))
                                        .arg(svidev::SupportedDevicesNames.value(svidev::VirtualDevice))
                                        .arg(-1)
                                        .arg(-1)
                                        .arg(-1)
                                        .arg("")
                                        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
                                        .arg(""));
      }
    }
  }
  
  q->finish();
  delete q;

  return err; 
  
}

QSqlError svidev::fill_device_list(QComboBox *cb)
{
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  QSqlError err = SQLITE->execSQL(SQL_SELECT_DEVICE_LIST, q);
  
  if(err.type() == QSqlError::NoError) {
  
    cb->clear();
    
    while(q->next())
      cb->addItem(q->value("device_name").toString(), q->value("id").toInt());
    
  }
 
  q->finish();
  delete q;
  
  return err;
  
}

QSqlError svidev::fill_device_info(int dbid, svidev::DeviceInfo &dinfo)
{
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  QSqlError err = SQLITE->execSQL(QString(SQL_SELECT_DEVICE_BY_ID).arg(dbid), q);
  
  if(err.type() == QSqlError::NoError) {
    
    q->first();
    
    dinfo.dbid = dbid;
    dinfo.deviceType = static_cast<dev_t>(q->value("device_type").toUInt());
    dinfo.idProduct = q->value("product_id").toUInt();
    dinfo.idVendor = q->value("vendor_id").toUInt();
    dinfo.iManufacturer = q->value("manufacturer_id").toUInt();
    dinfo.name = q->value("device_name").toString();
    dinfo.port = q->value("port_name").toUInt();
                       
  } 
  
  q->finish();
  delete q;
  
  return err;
  
}


/** ---------  ------------ **/
svidev::SvSelectDeviceType::SvSelectDeviceType(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SvSelectDeviceTypeDialog)
{
  ui->setupUi(this);
  
  for(svidev::SupportedDevices key: svidev::SupportedDevicesNames.keys())
    ui->cbDeviceType->addItem(svidev::SupportedDevicesNames.value(key), int(key));
  
  connect(ui->bnOk, SIGNAL(pressed()), this, SLOT(accept()));
  connect(ui->bnCancel, SIGNAL(pressed()), this, SLOT(reject()));
  
  setModal(true);
  show();
  
}

void svidev::SvSelectDeviceType::accept()
{
  type_id = static_cast<svidev::SupportedDevices>(ui->cbDeviceType->currentData().toInt());
  
  QDialog::accept();
  
}
