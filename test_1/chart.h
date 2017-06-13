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

#ifndef CHART_H
#define CHART_H

#include <QtCharts/QChart>
#include <QtCore/QTimer>

QT_CHARTS_BEGIN_NAMESPACE
class QSplineSeries;
class QLineSeries;
class QValueAxis;
QT_CHARTS_END_NAMESPACE


QT_BEGIN_NAMESPACE
class QGestureEvent;
QT_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

struct ChartParams{
  int x_range = 300;
  int x_tick_count = 26;
  qreal y_range = 1.0;
  int y_tick_count = 11;
  int line_width = 2;
  QColor line_color = Qt::red;
  bool show_TOF = false;
};

//![1]
class Chart: public QChart
{
    Q_OBJECT
public:
    Chart(ChartParams &params, QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~Chart();
    
    QLineSeries *m_series;
    QLineSeries *up_series;
    QLineSeries *down_series;
    QValueAxis *axX;
    QValueAxis *axY;

private:
    QTimer m_timer;
    QStringList m_titles;
    qreal m_step;
    qreal m_x;
    qreal m_y;
    
    ChartParams _params;
    
protected:
    bool sceneEvent(QEvent *event);

private:
    bool gestureEvent(QGestureEvent *event);
};
//![1]

#endif /* CHART_H */
