#ifndef SV_CHARTSETTINGSDIALOG_H
#define SV_CHARTSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SvChartSettingsDialog;
}

class SvChartSettingsDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit SvChartSettingsDialog(QWidget *parent = 0);
  ~SvChartSettingsDialog();
  
private:
  Ui::SvChartSettingsDialog *ui;
};

#endif // SV_CHARTSETTINGSDIALOG_H
