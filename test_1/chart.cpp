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

#include "chart.h"
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCore/QTime>
#include <QtCore/QDebug>
#include <QtWidgets/QGesture>

Chart::Chart(ChartParams &params, QGraphicsItem *parent, Qt::WindowFlags wFlags):
    QChart(QChart::ChartTypeCartesian, parent, wFlags),
    m_series(0),
    up_series(0),
    down_series(0),
    m_step(0),
    m_x(0),
    m_y(0),
    _params(params)
{
  m_series = new QLineSeries(this);
  up_series = new QLineSeries(this);
  down_series = new QLineSeries(this);

    QPen red(_params.line_color);
    red.setWidth(_params.line_width);
    m_series->setPen(red);
    m_series->append(m_x, m_y);

    QPen green(QColor(0, 0, 0, 255));
    up_series->setPen(green);
    up_series->append(m_x, m_y);

    QPen velvet(QColor(255, 0, 255, 255));
    velvet.setStyle(Qt::DashLine);
    down_series->setPen(velvet);
    down_series->append(m_x, m_y);
    axX = new QValueAxis;
    axX->setRange(0, _params.x_range);
    axX->setLabelFormat("%g");
    axX->setTitleText("Отсчеты");
    axX->setTickCount(_params.x_tick_count);
    axX->applyNiceNumbers();
    
    axY = new QValueAxis;
    axY->setRange(-_params.y_range, _params.y_range);
    axY->setTitleText("");
    axY->setTickCount(_params.y_tick_count);
    
    addSeries(m_series);

    if(_params.show_TOF){
        addSeries(up_series);
        addSeries(down_series);
    }

//    createDefaultAxes();
    setAxisX(axX, m_series);
    setAxisY(axY, m_series);

    setAxisX(axX, up_series);
    setAxisY(axY, up_series);

    setAxisX(axX, down_series);
    setAxisY(axY, down_series);
    
//    m_axis->setTickCount(20);
//    axX()->setRange(0, 200);
//    axY()->setRange(-5, 5);

}

Chart::~Chart()
{

}

bool Chart::sceneEvent(QEvent *event)
{
    if (event->type() == QEvent::Gesture)
        return gestureEvent(static_cast<QGestureEvent *>(event));
    return QChart::event(event);
}

bool Chart::gestureEvent(QGestureEvent *event)
{
    if (QGesture *gesture = event->gesture(Qt::PanGesture)) {
        QPanGesture *pan = static_cast<QPanGesture *>(gesture);
        QChart::scroll(-(pan->delta().x()), pan->delta().y());
    }

    if (QGesture *gesture = event->gesture(Qt::PinchGesture)) {
        QPinchGesture *pinch = static_cast<QPinchGesture *>(gesture);
        if (pinch->changeFlags() & QPinchGesture::ScaleFactorChanged)
            QChart::zoom(pinch->scaleFactor());
    }

    return true;
}
