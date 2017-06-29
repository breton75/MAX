#include "sv_graphparamsdialog.h"
#include "ui_sv_graphparamsdialog.h"

SvGraphParamsDialog::SvGraphParamsDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SvGraphParamsDialog)
{
  ui->setupUi(this);
  
  ui->cbLineStyle->addItem(tr("Solid"), static_cast<int>(Qt::SolidLine));
  ui->cbLineStyle->addItem(tr("Dash"), static_cast<int>(Qt::DashLine));
  ui->cbLineStyle->addItem(tr("Dot"), static_cast<int>(Qt::DotLine));
  ui->cbLineStyle->addItem(tr("Dash Dot"), static_cast<int>(Qt::DashDotLine));
  ui->cbLineStyle->addItem(tr("Dash Dot Dot"), static_cast<int>(Qt::DashDotDotLine));
  
  QStringList colorNames = QColor::colorNames();

  for (int i = 0; i < colorNames.size(); ++i) {
      QColor color(colorNames[i]);

      ui->cbLineColor->insertItem(i, colorNames[i]);
      ui->cbLineColor->setItemData(i, color, Qt::DecorationRole);
  }
  
  
  foreach (int key, GraphTypes.keys()) {
    ui->cbGraphType->addItem(GraphTypes.value(key), QVariant(key));
  }
  
}

SvGraphParamsDialog::~SvGraphParamsDialog()
{
  delete ui;
}


void SvGraphParamsDialog::accept()
{
  qDebug() << "dddd";
  QDialog::accept();
}
