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

#include "sv_chartwidget.h"

svchart::SvChartWidget::SvChartWidget(ChartParams &params, QWidget *parent)
{
  this->setParent(parent);
  
  _params = params;
  
  setupUi();
  
  connect(_customplot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(rangeChanged(QCPRange)));
  
  on_bnResetChart_clicked();
  
//  spinXRange->setValue(_params.x_range);
  bnYAutoscale->setChecked(_params.y_autoscale);  
  cbXAutoScroll->setCurrentIndex(cbXAutoScroll->findData(QVariant(_params.x_autoscroll_type)));
  cbXMeasureUnit->setCurrentIndex(cbXMeasureUnit->findData(QVariant(_params.x_measure_unit))); 
    
  
}

svchart::SvChartWidget::~SvChartWidget()
{ 
  /* освобождаем память графиков */
  for(svgraph::GraphIDs gid: _graphs.keys()) {
    
    /* удаляем график (также здесь освобождается память графика) */
    _graphs.value(gid)->graph->clearData();
    _customplot->removeGraph(_graphs.value(gid)->graph);
   
    /* освобождаем данные разных ед. измерения */
//    for(ChartXMeasureUnitIDs muid: ChartXMeasureUnitList) 
//      delete _graphs.value(gid)->data.value(muid);
   
    /* освобождаем память структуры GRAPH */
    delete _graphs.value(gid);
    
  }
  
  close();
  deleteLater();
}

void svchart::SvChartWidget::setupUi()
{
//    this->setObjectName(QStringLiteral("SvChartWidget1"));
  
    vlayMain = new QVBoxLayout(this);
    vlayMain->setObjectName(QStringLiteral("vlayMain"));
    
    hlay1 = new QHBoxLayout();
    hlay1->setSpacing(6);
    hlay1->setObjectName(QStringLiteral("hlay1"));
    hlay1->setContentsMargins(4, -1, -1, -1);
    
    bnResetChart = new QPushButton(this);
    bnResetChart->setObjectName(QStringLiteral("bnResetChart"));
    bnResetChart->setMaximumSize(QSize(25, 16777215));
    
    QIcon icon;
    icon.addFile(QStringLiteral(":/Common/Icons/Basic/Refresh.ico"), QSize(), QIcon::Normal, QIcon::Off);
    bnResetChart->setIcon(icon);

    hlay1->addWidget(bnResetChart);

    frameXRange = new QFrame(this);
    frameXRange->setObjectName(QStringLiteral("frameXRange"));
    frameXRange->setFrameShape(QFrame::StyledPanel);
    frameXRange->setFrameShadow(QFrame::Raised);
    hlayXRange = new QHBoxLayout(frameXRange);
    hlayXRange->setObjectName(QStringLiteral("hlayXRange"));
    hlayXRange->setContentsMargins(4, 4, 4, 4);
    hspacer1 = new QSpacerItem(122, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hlayXRange->addItem(hspacer1);

    bnXRangeDown = new QPushButton(frameXRange);
    bnXRangeDown->setObjectName(QStringLiteral("bnXRangeDown"));
    bnXRangeDown->setMaximumSize(QSize(25, 16777215));
    QIcon icon1;
    icon1.addFile(QStringLiteral("../../Common/Icons/Basic/Zoom out.ico"), QSize(), QIcon::Normal, QIcon::Off);
    bnXRangeDown->setIcon(icon1);

    hlayXRange->addWidget(bnXRangeDown);

    hspacer2 = new QSpacerItem(123, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hlayXRange->addItem(hspacer2);

    bnXScrollToBegin = new QPushButton(frameXRange);
    bnXScrollToBegin->setObjectName(QStringLiteral("bnXScrollToBegin"));
    bnXScrollToBegin->setMaximumSize(QSize(25, 16777215));
    QIcon icon2_1;
    icon2_1.addFile(QStringLiteral(":/icons/arrow-left3.ico"), QSize(), QIcon::Normal, QIcon::Off);
    bnXScrollToBegin->setIcon(icon2_1);

    hlayXRange->addWidget(bnXScrollToBegin);
    
    bnXRangeActual = new QPushButton(frameXRange);
    bnXRangeActual->setObjectName(QStringLiteral("bnXRangeActual"));
    bnXRangeActual->setMaximumSize(QSize(25, 16777215));
    QIcon icon2;
    icon2.addFile(QStringLiteral(":/Common/Icons/Basic/Search.ico"), QSize(), QIcon::Normal, QIcon::Off);
    bnXRangeActual->setIcon(icon2);

    hlayXRange->addWidget(bnXRangeActual);
    
    bnXScrollToEnd = new QPushButton(frameXRange);
    bnXScrollToEnd->setObjectName(QStringLiteral("bnXScrollToEnd"));
    bnXScrollToEnd->setMaximumSize(QSize(25, 16777215));
    QIcon icon4_1;
    icon4_1.addFile(QStringLiteral(":/icons/arrow-right3.ico"), QSize(), QIcon::Normal, QIcon::Off);
    bnXScrollToEnd->setIcon(icon4_1);

    hlayXRange->addWidget(bnXScrollToEnd);

    hspacer3 = new QSpacerItem(122, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hlayXRange->addItem(hspacer3);

//    spinXRange = new QSpinBox(frameXRange);
//    spinXRange->setObjectName(QStringLiteral("spinXRange"));
//    spinXRange->setMinimum(10);
//    spinXRange->setMaximum(10000);

//    hlayXRange->addWidget(spinXRange);

//    bnXSetRange = new QPushButton(frameXRange);
//    bnXSetRange->setObjectName(QStringLiteral("bnXSetRange"));
//    bnXSetRange->setMaximumSize(QSize(25, 16777215));
//    QIcon icon3;
//    icon3.addFile(QStringLiteral(":/Common/Icons/Basic/Ok2.ico"), QSize(), QIcon::Normal, QIcon::Off);
//    bnXSetRange->setIcon(icon3);

//    hlayXRange->addWidget(bnXSetRange);

//    hspacer4 = new QSpacerItem(123, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

//    hlayXRange->addItem(hspacer4);

    bnXRangeUp = new QPushButton(frameXRange);
    bnXRangeUp->setObjectName(QStringLiteral("bnXRangeUp"));
    bnXRangeUp->setMaximumSize(QSize(25, 16777215));
    QIcon icon4;
    icon4.addFile(QStringLiteral(":/Common/Icons/Basic/Zoom in.ico"), QSize(), QIcon::Normal, QIcon::Off);
    bnXRangeUp->setIcon(icon4);

    hlayXRange->addWidget(bnXRangeUp);

    hspacer5 = new QSpacerItem(122, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hlayXRange->addItem(hspacer5);

    
    cbXAutoScroll = new QComboBox(frameXRange);
    cbXAutoScroll->setObjectName(QStringLiteral("cbXAutoScroll"));
    cbXAutoScroll->setMinimumSize(QSize(0, 0));
    cbXAutoScroll->setStyleSheet(QStringLiteral("color: rgb(0, 97, 152);"));

    cbXAutoScroll->clear();
    cbXAutoScroll->insertItem(0, ChartXAutoscrollTypes.value(svchart::xtNoAutoScroll), QVariant(svchart::xtNoAutoScroll));
    cbXAutoScroll->insertItem(1, ChartXAutoscrollTypes.value(svchart::xtTickScroll), QVariant(svchart::xtTickScroll));
    cbXAutoScroll->insertItem(2, ChartXAutoscrollTypes.value(svchart::xtHalfChartScroll), QVariant(svchart::xtHalfChartScroll));
    cbXAutoScroll->insertItem(3, ChartXAutoscrollTypes.value(svchart::xtChartScroll), QVariant(svchart::xtChartScroll));
    
    hlayXRange->addWidget(cbXAutoScroll);
    
    cbXMeasureUnit = new QComboBox(frameXRange);
    cbXMeasureUnit->setObjectName(QStringLiteral("cbXMeasureUnit"));
    cbXMeasureUnit->setMinimumSize(QSize(0, 0));
    cbXMeasureUnit->setStyleSheet(QStringLiteral("color: rgb(0, 97, 152);"));
    
    cbXMeasureUnit->clear();
    cbXMeasureUnit->insertItem(0, ChartXMeasureUnits.value(svchart::xmuTick), QVariant(svchart::xmuTick));
//    cbXMeasureUnit->insertItem(1, ChartXMeasureUnits.value(svchart::xmuMillisecond), QVariant(svchart::xmuMillisecond));
//    cbXMeasureUnit->insertItem(2, ChartXMeasureUnits.value(svchart::xmuSecond), QVariant(svchart::xmuSecond));

    hlayXRange->addWidget(cbXMeasureUnit);

    hlay1->addWidget(frameXRange);


    vlayMain->addLayout(hlay1);

    hlay2 = new QHBoxLayout();
    hlay2->setObjectName(QStringLiteral("hlay2"));
    frameYRange = new QFrame(this);
    frameYRange->setObjectName(QStringLiteral("frameYRange"));
    frameYRange->setFrameShape(QFrame::StyledPanel);
    frameYRange->setFrameShadow(QFrame::Raised);
    vlayYRange = new QVBoxLayout(frameYRange);
    vlayYRange->setSpacing(2);
    vlayYRange->setObjectName(QStringLiteral("vlayYRange"));
    vlayYRange->setContentsMargins(4, 4, 4, 4);
    bnYAutoscale = new QPushButton(frameYRange);
    bnYAutoscale->setObjectName(QStringLiteral("bnYAutoscale"));
    bnYAutoscale->setMaximumSize(QSize(25, 16777215));
    QIcon icon5;
    icon5.addFile(QStringLiteral(":/Common/Icons/Basic/Stats.ico"), QSize(), QIcon::Normal, QIcon::Off);
    bnYAutoscale->setIcon(icon5);
    bnYAutoscale->setCheckable(true);
  
    vlayYRange->addWidget(bnYAutoscale);

    vspacer1 = new QSpacerItem(20, 84, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vlayYRange->addItem(vspacer1);

    bnYRangeDown = new QPushButton(frameYRange);
    bnYRangeDown->setObjectName(QStringLiteral("bnYRangeDown"));
    bnYRangeDown->setMaximumSize(QSize(25, 16777215));
    bnYRangeDown->setIcon(icon4);

    vlayYRange->addWidget(bnYRangeDown);

    vspacer2 = new QSpacerItem(20, 85, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vlayYRange->addItem(vspacer2);

    bnYRangeActual = new QPushButton(frameYRange);
    bnYRangeActual->setObjectName(QStringLiteral("bnYRangeActual"));
    bnYRangeActual->setMaximumSize(QSize(25, 16777215));
    bnYRangeActual->setIcon(icon2);

    vlayYRange->addWidget(bnYRangeActual);

    vspacer3 = new QSpacerItem(20, 84, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vlayYRange->addItem(vspacer3);

    bnYRangeUp = new QPushButton(frameYRange);
    bnYRangeUp->setObjectName(QStringLiteral("bnYRangeUp"));
    bnYRangeUp->setMaximumSize(QSize(25, 16777215));
    QIcon icon6;
    icon6.addFile(QStringLiteral(":/Common/Icons/Basic/Zoom out.ico"), QSize(), QIcon::Normal, QIcon::Off);
    bnYRangeUp->setIcon(icon6);

    vlayYRange->addWidget(bnYRangeUp);

    vspacer4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vlayYRange->addItem(vspacer4);

    hlay2->addWidget(frameYRange);


    _customplot = new QCustomPlot(this);
    _customplot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _customplot->xAxis->setRange(0, _params.x_range, Qt::AlignLeft);
    
    _customplot->yAxis->setRange(0, 1, Qt::AlignCenter);
//    _customplot->axisRect()->setupFullAxesBox(true);
//    _customplot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    _customplot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    _customplot->legend->setVisible(true);
        
    hlay2->addWidget(_customplot);
    
    vlayMain->addLayout(hlay2);


//    retranslateUi(this);

    QMetaObject::connectSlotsByName(this);
} // setupUi

void svchart::SvChartWidget::setParams(svchart::ChartParams &params)
{ 
  _params = params;
  
  /* определяем коэффициенты для пересчета значений оси  */
//  _x_measure_unit_koeff[svchart::xmuTick] = 1;
//  _x_measure_unit_koeff[svchart::xmuMillisecond] = _params.x_tick_period;
}

void svchart::SvChartWidget::addGraph(svgraph::GraphIDs graph_id, svgraph::GraphParams &graphParams)
{
  /* если такой график уже есть, то ничего не добавляем и выходим */
  if(findGraph(graph_id))
    return;
    
  svchart::GRAPH* g = new svchart::GRAPH;
  g->params = graphParams;
  g->graph = _customplot->addGraph();

//  for(ChartXMeasureUnitIDs muid: ChartXMeasureUnitList) {
//    g->data.insert(muid, new QCPDataMap);
//  }
  
//  g->graph->setData(g->data.value(_params.x_measure_unit));
  
  _graphs.insert(graph_id, g);

  QPen pen(graphParams.line_color);
  pen.setStyle(Qt::PenStyle(graphParams.line_style));
  pen.setWidth(graphParams.line_width);
    
  _graphs.value(graph_id)->graph->setPen(pen);
  _graphs.value(graph_id)->graph->setName(svgraph::GraphTypes.value(graph_id));
  
}

void svchart::SvChartWidget::setGraphParams(svgraph::GraphIDs graph_id, svgraph::GraphParams &graphParams)
{
  _graphs.value(graph_id)->params = graphParams;
  
  QPen pen(graphParams.line_color);
  pen.setStyle(Qt::PenStyle(graphParams.line_style));
  pen.setWidth(graphParams.line_width);
    
  _graphs.value(graph_id)->graph->setPen(pen);
  
  _customplot->repaint();
  
}

void svchart::SvChartWidget::removeGraph(svgraph::GraphIDs graph_id)
{
  /* очищаем память */
  _graphs.value(graph_id)->graph->clearData();
  
  /* освобождаем данные разных ед. измерения */
//  for(ChartXMeasureUnitIDs muid: ChartXMeasureUnitList) 
//     delete _graphs.value(graph_id)->data.value(muid);
  
  /* удаляем график (также здесь освобождается память графика) */
  _customplot->removeGraph(_graphs.value(graph_id)->graph);
  
  /* освобождаем память структуры GRAPH */
  delete _graphs.value(graph_id);
  
  /* удаляем запись о графике из списка графиков */
  _graphs.remove(graph_id);
  
  _customplot->replot();
  
}

void svchart::SvChartWidget::appendData(svgraph::GraphIDs graph_id, double y)
{
  mutex.lock();
  
  quint32 tick = _graphs.value(graph_id)->graph->data()->count();
  
  /* вычисляем текущее значение x для различных ед. измерения */
//  _current_x[svchart::xmuTick] = _graphs.value(graph_id)->graph->data()->count();
//  _current_x[svchart::xmuMillisecond] = _graphs.value(graph_id)->graph->data()->count() * _params.x_tick_period;
//  x.insert(svchart::xmuSecond, current_tick * _params.x_tick_period / 1000);
  
  /* раскидываем вычисленные значения в соответствующий массив данных */
//  for(svchart::ChartXMeasureUnitIDs muid: _current_x.keys())
//    _graphs.value(graph_id)->data.value(muid)->insert(_current_x.value(muid), QCPData(_current_x.value(muid), y));
  
  /* записываем значение для текущей ед. измерения */
  _graphs.value(graph_id)->graph->data()->insert(tick, QCPData(tick, y));

  mutex.unlock();
  
  /* сдвигаем график если надо */
  if(_params.x_autoscroll_type != xtNoAutoScroll) {
    
    if(tick > _customplot->xAxis->range().upper) {
      qreal dx;
      
      switch (_params.x_autoscroll_type) {
        case svchart::xtTickScroll:
          dx = 1;
          break;
          
        case svchart::xtHalfChartScroll:
          dx = _customplot->xAxis->range().size() / 2;
          break;
          
        case svchart::xtChartScroll:
          dx = _customplot->xAxis->range().size() - 1;
          break;
      }
      _customplot->xAxis->moveRange(dx);
    }
  }
  
     
  setMaxMinY(y);
  
  if(_params.y_autoscale)
    setActualYRange();
}

void svchart::SvChartWidget::insertData(svgraph::GraphIDs graph_id, QCPData xy)
{
  _graphs.value(graph_id)->graph->data()->insert(xy.key, xy);
  
  setMaxMinY(xy.value);
  
  if(_params.y_autoscale)
    setActualYRange();
}

void svchart::SvChartWidget::on_cbXAutoScroll_currentIndexChanged(int index)
{
  _params.x_autoscroll_type = static_cast<svchart::ChartXAutoscrollTypeIDs>(cbXAutoScroll->itemData(index).toInt());
}

//void svchart::SvChartWidget::on_cbXMeasureUnit_currentIndexChanged(int index)
//{
//  svchart::ChartXMeasureUnitIDs last_measure_unit = _params.x_measure_unit;
//  _params.x_measure_unit = static_cast<svchart::ChartXMeasureUnitIDs>(cbXMeasureUnit->itemData(index).toInt());

//  mutex.lock();

//  /* копируем соответствующий единице измерения массив данных */
//  for(svchart::GRAPH* g: _graphs.values()) {
//    g->graph->clearData();
//    g->graph->setData(g->data.value(_params.x_measure_unit), true);
//  }
 
//  _customplot->xAxis->setRange(_customplot->xAxis->range().lower / _x_measure_unit_koeff.value(last_measure_unit) * _x_measure_unit_koeff.value(_params.x_measure_unit),
//                               _customplot->xAxis->range().upper / _x_measure_unit_koeff.value(last_measure_unit) * _x_measure_unit_koeff.value(_params.x_measure_unit));
  
//  _customplot->replot();
  
//  mutex.unlock();

//}

void svchart::SvChartWidget::on_bnXRangeUp_clicked()
{
  mutex.lock();
  _customplot->xAxis->setRange(_customplot->xAxis->range().lower, _customplot->xAxis->range().upper * 1.25);
  _customplot->replot();
  mutex.unlock();
}

void svchart::SvChartWidget::on_bnXRangeDown_clicked()
{
  mutex.lock();

  if(_customplot->xAxis->range().lower >= _customplot->xAxis->range().upper / 1.25)
    return;
  
  _customplot->xAxis->setRange(_customplot->xAxis->range().lower, _customplot->xAxis->range().upper / 1.25);
  _customplot->replot();
  
  mutex.unlock();
}

void svchart::SvChartWidget::on_bnXRangeActual_clicked()
{
  mutex.lock();
  _customplot->xAxis->setRange(0, _customplot->graph()->data()->count(), Qt::AlignLeft);
  _customplot->replot();  
  mutex.unlock();
}

//void svchart::SvChartWidget::on_bnXSetRange_clicked()
//{
//  _params.x_range = spinXRange->value();
//  _customplot->xAxis->setRangeUpper(_params.x_range);
//  _customplot->replot(QCustomPlot::rpQueued);
//}

//void svchart::SvChartWidget::setXRange(double lower, double upper)
//{
//  _customplot->xAxis->setRange(lower, upper); // _params.x_range, Qt::AlignLeft);
//  _customplot->replot();
//}

void svchart::SvChartWidget::on_bnXScrollToBegin_clicked()
{
  _customplot->xAxis->setRange(0, _customplot->xAxis->range().size(), Qt::AlignLeft);
  _customplot->replot();
}

void svchart::SvChartWidget::on_bnXScrollToEnd_clicked()
{
  double upper = _customplot->graph()->data()->count(); // * _x_measure_unit_koeff.value(_params.x_measure_unit);

  _customplot->xAxis->setRange(upper - _customplot->xAxis->range().size(), upper);
  _customplot->replot();
}

void svchart::SvChartWidget::on_bnYRangeUp_clicked()
{
  _customplot->yAxis->setRangeUpper(_customplot->yAxis->range().upper * 1.25);
  _customplot->yAxis->setRangeLower(_customplot->yAxis->range().lower * 1.25);
  _customplot->replot(QCustomPlot::rpQueued);
}

void svchart::SvChartWidget::on_bnYRangeDown_clicked()
{
  _customplot->yAxis->setRangeUpper(_customplot->yAxis->range().upper * 0.75);
  _customplot->yAxis->setRangeLower(_customplot->yAxis->range().lower * 0.75);
  _customplot->replot(QCustomPlot::rpQueued);
}

void svchart::SvChartWidget::on_bnYRangeActual_clicked()
{
  setActualYRange();
}

void svchart::SvChartWidget::setActualYRange()
{
  _customplot->yAxis->setRange(_y_min, _y_max);
  _customplot->replot(QCustomPlot::rpQueued);
}

void svchart::SvChartWidget::on_bnResetChart_clicked()
{
  for(svchart::GRAPH* g: _graphs.values()) {
    
    g->graph->clearData();
    
//    for(ChartXMeasureUnitIDs muid: ChartXMeasureUnitList) {
//      delete g->data.value(muid);
//      g->data[muid] = new QCPDataMap;
//    }
  }
        
  _customplot->xAxis->setRange(0, _params.x_range, Qt::AlignLeft);
  
  _y_max = -1000000000;
  _y_min =  1000000000;
  
  emit onReset();
  
  _customplot->replot();
  
}

void svchart::SvChartWidget::on_bnYAutoscale_clicked(bool checked)
{
  if(checked)
    on_bnYRangeActual_clicked();
  
  _params.y_autoscale = checked;
  
}

void svchart::SvChartWidget::rangeChanged(QCPRange range)
{
  _params.x_range = range.size();
  
  _customplot->xAxis->setLabel(QString("%1. Текущий диапазон: %2")
                               .arg(ChartXMeasureUnits.value(_params.x_measure_unit))
                               .arg(_customplot->xAxis->range().size()));
}
