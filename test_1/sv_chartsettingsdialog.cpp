#include "sv_chartsettingsdialog.h"
#include "ui_sv_chartsettingsdialog.h"

SvChartSettingsDialog::SvChartSettingsDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SvChartSettingsDialog)
{
  ui->setupUi(this);
}

SvChartSettingsDialog::~SvChartSettingsDialog()
{
  delete ui;
}
