#ifndef SV_GRAPHPARAMSDIALOG_H
#define SV_GRAPHPARAMSDIALOG_H

#include <QDialog>
#include <QColor>
#include <QStringList>
#include <QMap>
#include <QDebug>

namespace Ui {
class SvGraphParamsDialog;
}

/* режимы отображения */
const QMap<int, QString> GraphTypes = {{0, "Скорость потока м/с."},
               {1, "TOF diff нс."},
               {2, "(t1 + t2) / 2"},
               {3, "Скорость звука (м/с)."}};


class SvGraphParamsDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit SvGraphParamsDialog(QWidget *parent = 0);
  ~SvGraphParamsDialog();
  
private slots:
//  void on_buttonBox_accepted();
  
  void accept();
  
 
private:
  Ui::SvGraphParamsDialog *ui;
};

#endif // SV_GRAPHPARAMSDIALOG_H
