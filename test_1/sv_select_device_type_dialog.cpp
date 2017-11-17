#include "sv_select_device_type_dialog.h"
#include "ui_sv_select_device_type_dialog.h"

extern SvSQLITE *SQLITE;

SvSelectDeviceTypeDialog::SvSelectDeviceTypeDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SvSelectDeviceTypeDialog)
{
  ui->setupUi(this);
}

SvSelectDeviceTypeDialog::~SvSelectDeviceTypeDialog()
{
  delete ui;
}
