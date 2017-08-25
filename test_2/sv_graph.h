#ifndef SV_GRAPH_H
#define SV_GRAPH_H

#include <QDialog>
#include <QColor>
#include <QStringList>
#include <QMap>
#include <QDebug>


namespace Ui {
class SvGraphParamsDialog;
}

namespace svgraph {

  enum GraphIDs{
    giVpot = 0,
    giTOFdiff,
    gitAvg,
    giVsnd,
    giTemperature
  };

  /* режимы отображения */
  const QMap<GraphIDs, QString> GraphTypes = {{giVpot, "Скорость потока м/с."},
               {giTOFdiff, "TOF diff нс."},
               {gitAvg, "(t1 + t2) / 2"},
               {giVsnd, "Скорость звука (м/с)."},
               {giTemperature, "Температура"}};

  
  struct GraphParams {
    GraphIDs type = giVpot;  // "Скорость потока м/с."
    int line_width = 1;
    QColor line_color = Qt::red;
    int line_style = static_cast<int> (Qt::SolidLine);
    QString legend = "";
    
    GraphParams &operator= (GraphParams &p) {
      legend = p.legend;
      line_color = p.line_color;
      line_style = p.line_style;
      type = p.type;
      line_width = p.line_width;
      
      return *this;
    }
  };
  
  class SvGraphParamsDialog;
}


class svgraph::SvGraphParamsDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit SvGraphParamsDialog(QWidget *parent = 0, svgraph::GraphParams *params = nullptr);
  ~SvGraphParamsDialog();
  
  svgraph::GraphParams graph_params;
  
public slots:
  void accept() Q_DECL_OVERRIDE;
  
 
private slots:
  void on_cbLineColor_currentIndexChanged(const QString &arg1);
  
//  void on_buttonBox_clicked(QAbstractButton *button);
  
  void on_cbGraphType_currentIndexChanged(const QString &arg1);
  
private:
  Ui::SvGraphParamsDialog *ui;
};

#endif // SV_GRAPH_H
