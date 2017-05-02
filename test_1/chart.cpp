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

Chart::Chart(QGraphicsItem *parent, Qt::WindowFlags wFlags):
    QChart(QChart::ChartTypeCartesian, parent, wFlags),
    m_series(0),
    m_axis(new QValueAxis),
    m_step(0),
    m_x(0),
    m_y(0)
{
    m_series = new QSplineSeries(this);
    QPen green(Qt::red);
    green.setWidth(2);
    m_series->setPen(green);
    m_series->append(m_x, m_y);

    addSeries(m_series);
    createDefaultAxes();
    setAxisX(m_axis, m_series);
    m_axis->setTickCount(20);
    axisX()->setRange(0, 200);
    axisY()->setRange(-100, 100);

}

Chart::~Chart()
{

}

void Chart::handleTimeout()
{
    qreal x = plotArea().width() / m_axis->tickCount();
    qreal y = (m_axis->max() - m_axis->min()) / m_axis->tickCount();
    m_x += 1;
    m_y = qrand() % 5 - 2.5;
    m_series->append(m_x, m_y);
//    scroll(x, 0);
    if (m_x == 100)
        m_timer.stop();
}

//bool Chart::sceneEvent(QEvent *event)
//{
//  qDebug() << event->type();
//    if (event->type() == QEvent::Gesture)
//        return gestureEvent(static_cast<QGestureEvent *>(event));
//    return QChart::event(event);
//}

//bool Chart::gestureEvent(QGestureEvent *event)
//{
//    if (QGesture *gesture = event->gesture(Qt::PanGesture)) {
//        QPanGesture *pan = static_cast<QPanGesture *>(gesture);
//        QChart::scroll(-(pan->delta().x()), pan->delta().y());
//    }

//    if (QGesture *gesture = event->gesture(Qt::PinchGesture)) {
//        QPinchGesture *pinch = static_cast<QPinchGesture *>(gesture);
//        if (pinch->changeFlags() & QPinchGesture::ScaleFactorChanged)
//            QChart::zoom(pinch->scaleFactor());
//    }

//    return true;
//}
