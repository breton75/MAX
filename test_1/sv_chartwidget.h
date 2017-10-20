/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef SV_CHARTWIDGET_H
#define SV_CHARTWIDGET_H

#include <QtCore/QTimer>
#include <QWidget>
#include <QMutex>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QSpinBox>
#include <QtCore/QTime>
#include <QtCore/QDebug>

#include "qcustomplot.h"

#include "sv_graph.h"


namespace svchart {

  enum ChartXAutoscrollTypeIDs {
    xtNoAutoScroll = 0,
    xtTickScroll,
    xtHalfChartScroll,
    xtChartScroll
  };
  
  /* сдвиг по оси Х */
  const QMap<ChartXAutoscrollTypeIDs, QString> ChartXAutoscrollTypes = {
               {xtNoAutoScroll, QApplication::translate("Form", "\320\220\320\262\321\202\320\276\321\201\320\264\320\262\320\270\320\263: <\320\235\320\265\321\202>", Q_NULLPTR)},
               {xtTickScroll, QApplication::translate("Form", "\320\220\320\262\321\202\320\276\321\201\320\264\320\262\320\270\320\263: \320\235\320\260 \320\276\320\264\320\275\321\203 \321\202\320\276\321\207\320\272\321\203", Q_NULLPTR)},
               {xtHalfChartScroll, QApplication::translate("Form", "\320\220\320\262\321\202\320\276\321\201\320\264\320\262\320\270\320\263: \320\235\320\260 1/2 \320\263\321\200\320\260\321\204\320\270\320\272\320\260", Q_NULLPTR)},
               {xtChartScroll, QApplication::translate("Form", "\320\220\320\262\321\202\320\276\321\201\320\264\320\262\320\270\320\263: \320\235\320\260 \321\206\320\265\320\273\321\213\320\271 \320\263\321\200\320\260\321\204\320\270\320\272", Q_NULLPTR)}
  };


  enum ChartXMeasureUnitIDs {
    xmuTick,
    xmuMillisecond,
    xmuSecond
  };
  
  const QList<ChartXMeasureUnitIDs> ChartXMeasureUnitList = {xmuTick, xmuMillisecond, xmuSecond};
  
  const QMap<ChartXMeasureUnitIDs, QString> ChartXMeasureUnits = {
    {xmuTick, "Отсчеты"},
    {xmuMillisecond, "Миллисекунды"}
//    {xmuSecond, "Секунды"}    
  };
  
  struct ChartParams {
    int x_range = 300;
    int x_tick_count = 26;
    qreal y_range = 1.0;
//    int y_tick_count = 11;
    bool y_autoscale = false;
    svchart::ChartXAutoscrollTypeIDs x_autoscroll_type = xtTickScroll;
    svchart::ChartXMeasureUnitIDs x_measure_unit = xmuTick;
    int x_tick_period = 200;  // в миллисекундах
    
  };

  struct GRAPH {
    QCPGraph* graph;
    svgraph::GraphParams params;
    QMap<ChartXMeasureUnitIDs, QCPDataMap*> data;
//    quint64 tick_count = 0;
  };
  
  class SvChartWidget;
  
}


class svchart::SvChartWidget: public QWidget
{
    Q_OBJECT

public:
    SvChartWidget(svchart::ChartParams &params, QWidget *parent = 0);
    ~SvChartWidget();
    
    QCustomPlot *customplot() { return _customplot; }
    svchart::ChartParams chartParams() { return _params; }
    void setParams(svchart::ChartParams &params);
    
    
    void setActualYRange();
    
    void setMaxMinY(qreal y) { if(y > _y_max) _y_max = y * 1.01; 
                               if(y < _y_min) _y_min = y * 1.01; }
    
    void addGraph(svgraph::GraphIDs graph_id, svgraph::GraphParams &graphParams);
    
    void removeGraph(svgraph::GraphIDs graph_id);
    
    bool findGraph(svgraph::GraphIDs graph_id) { return _graphs.find(graph_id) != _graphs.end(); }
    
    void setGraphParams(svgraph::GraphIDs graph_id, svgraph::GraphParams &graphParams);
    
    QList<svgraph::GraphIDs> graphList() { return _graphs.keys(); }
    
    int graphCount() { return _customplot->graphCount(); }
    
    void appendData(svgraph::GraphIDs graph_id, double y);
    
    void insertData(svgraph::GraphIDs graph_id, QCPData xy);
    
    svgraph::GraphParams graphParams(svgraph::GraphIDs graph_id) { return _graphs.value(graph_id)->params; }
    
//    int pointCount() { return _customplot->graph()->data()->count(); }
    
//    quint64 tickCount() { return _tick_count; }
    
    QMutex mutex;
    
private:
    
    QCustomPlot *_customplot;
    QMap<svgraph::GraphIDs, svchart::GRAPH*> _graphs;
    svchart::ChartParams _params;
    
    qreal _y_max = -1000000000;
    qreal _y_min =  1000000000;
    
//    quint64 _tick_count = 0;
    
    /* коэффициенты для пересчета оси Х для разных ед. измерения */
    QMap<svchart::ChartXMeasureUnitIDs, qreal> _x_measure_unit_koeff = {
      {svchart::xmuTick, 0.0}, {svchart::xmuMillisecond, 0.0} };
    
    /* вычисляемые текущие значения Х для разных ед. измерения */
    QMap<svchart::ChartXMeasureUnitIDs, double> _current_x = {
      {svchart::xmuTick, 0.0}, {svchart::xmuMillisecond, 0.0}};
    
    /** виджеты **/
    void setupUi();
    
    QVBoxLayout *vlayMain;
    QHBoxLayout *hlay1;
    QPushButton *bnResetChart;
    QFrame *frameXRange;
    QHBoxLayout *hlayXRange;
    QSpacerItem *hspacer1;
    QPushButton *bnXRangeDown;
    QSpacerItem *hspacer2;
    QPushButton *bnXScrollToBegin;
    QPushButton *bnXRangeActual;
    QPushButton *bnXScrollToEnd;
    QSpacerItem *hspacer3;
    QSpinBox *spinXRange;
    QPushButton *bnXSetRange;
    QSpacerItem *hspacer4;
    QPushButton *bnXRangeUp;
    QSpacerItem *hspacer5;
    QComboBox *cbXAutoScroll;
    QComboBox *cbXMeasureUnit;
    QHBoxLayout *hlay2;
    QFrame *frameYRange;
    QVBoxLayout *vlayYRange;
    QPushButton *bnYAutoscale;
    QSpacerItem *vspacer1;
    QPushButton *bnYRangeDown;
    QSpacerItem *vspacer2;
    QPushButton *bnYRangeActual;
    QSpacerItem *vspacer3;
    QPushButton *bnYRangeUp;
    QSpacerItem *vspacer4;
    
private slots:
    void rangeChanged(QCPRange range);
    
    void on_bnXRangeUp_clicked();
    void on_bnXRangeDown_clicked();
    void on_bnXScrollToBegin_clicked();
    void on_bnXRangeActual_clicked();
    void on_bnXScrollToEnd_clicked();
//    void on_bnXSetRange_clicked();
    void on_bnYRangeUp_clicked();
    void on_bnYRangeDown_clicked();
    void on_bnYRangeActual_clicked();
    void on_bnResetChart_clicked();
    void on_bnYAutoscale_clicked(bool checked);
    void on_cbXAutoScroll_currentIndexChanged(int index);
    void on_cbXMeasureUnit_currentIndexChanged(int index);
    
//    void setXRange(double lower, double upper);
signals:
    void onReset();
    
};




//class svchart::Chart: public QChart
//{
//    Q_OBJECT
//public:
//    Chart(ChartParams &params, QGraphicsItem *parent = Q_NULLPTR, Qt::WindowFlags wFlags = 0);
//    virtual ~Chart();
    
//    QLineSeries *m_series;
//    QValueAxis *axX;
//    QValueAxis *axY;

//private:
//    QStringList m_titles;
//    qreal m_step;
//    qreal m_x;
//    qreal m_y;
    
//    svchart::ChartParams _params;
    
//protected:
//    bool sceneEvent(QEvent *event);

//private:
//    bool gestureEvent(QGestureEvent *event);
//};


#endif /* SV_CHARTWIDGET_H */
