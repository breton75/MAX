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

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCore/QTimer>
#include <QWidget>
#include <QMutex>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QSpinBox>

#include "qcustomplot.h"

QT_CHARTS_BEGIN_NAMESPACE
class QSplineSeries;
class QLineSeries;
class QValueAxis;
QT_CHARTS_END_NAMESPACE


QT_BEGIN_NAMESPACE
class QGestureEvent;
QT_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

namespace svchart {

  struct ChartParams {
    int x_range = 300;
    int x_tick_count = 26;
    qreal y_range = 1.0;
//    int y_tick_count = 11;
    bool y_autoscale = false;
  };
  
  struct GraphParams {
    int line_width = 1;
    QColor line_color = Qt::red;
    int line_style = 1;
    QString legend = "";
  };
  
//  class Chart;
  class SvChartWidget;
  
}

struct GRAPH {
  QCPGraph* graph;
  svchart::GraphParams params;
};

class svchart::SvChartWidget: public QWidget
{
    Q_OBJECT

public:
    SvChartWidget(svchart::ChartParams &params, Qt::WindowFlags wFlags = 0, QWidget *parent = 0);
    
    QCustomPlot *customplot() { return _customplot; }
    svchart::ChartParams chartParams() { return _params; }
    
    void setActualYRange();
    
    void setMaxMinY(qreal y) { if(y > _y_max) _y_max = y * 1.01; 
                               if(y < _y_min) _y_min = y * 1.01; }
    
    void addGraph(int graph_id, svchart::GraphParams &graphParams);
    
    void removeGraph(int graph_id);
    
    bool findGraph(int graph_id) { return _graphs.find(graph_id) != _graphs.end(); }
    
    void setGraphParams(int graph_id, svchart::GraphParams &graphParams);
    
    QList<int> graphList() { return _graphs.keys(); }
    
    int graphCount() { return _customplot->graphCount(); }
    
    void appendData(int graph_id, double y);
    
    void insertData(int graph_id, QCPData xy);
    
    svchart::GraphParams graphParams(int graph_id) { return _graphs.value(graph_id)->params; }
    
    int pointCount() { return _customplot->graph()->data()->count(); }
    
    QMutex mutex;
    
private:
    
    QCustomPlot *_customplot;
    QMap<int, GRAPH*> _graphs;
    svchart::ChartParams _params;
    
    qreal _y_max = -1000000000;
    qreal _y_min =  1000000000;
    
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
    QPushButton *bnXRangeActual;
    QSpacerItem *hspacer3;
    QSpinBox *spinXRange;
    QPushButton *bnXSetRange;
    QSpacerItem *hspacer4;
    QPushButton *bnXRangeUp;
    QSpacerItem *hspacer5;
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
    void on_bnXRangeUp_clicked();
    void on_bnXRangeDown_clicked();
    void on_bnXRangeActual_clicked();
    void on_bnXSetRange_clicked();
    void on_bnYRangeUp_clicked();
    void on_bnYRangeDown_clicked();
    void on_bnYRangeActual_clicked();
    void on_bnResetChart_clicked();
    void on_bnYAutoscale_clicked(bool checked);

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
