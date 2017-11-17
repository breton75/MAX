#ifndef SV_SELECT_DEVICE_TYPE_DIALOG_H
#define SV_SELECT_DEVICE_TYPE_DIALOG_H

#include <QDialog>

#include "../../svlib/sv_sqlite.h"
#include "../../svlib/sv_log.h"

namespace Ui {
class SvSelectDeviceTypeDialog;
}

class SvSelectDeviceTypeDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit SvSelectDeviceTypeDialog(QWidget *parent = 0);
  ~SvSelectDeviceTypeDialog();
  
private:
  Ui::SvSelectDeviceTypeDialog *ui;
};

#endif // SV_SELECT_DEVICE_TYPE_DIALOG_H
