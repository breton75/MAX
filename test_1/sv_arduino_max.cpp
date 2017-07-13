#include "sv_arduino_max.h"
#include "ui_engine_control.h"

svarduinomax::SvArduinoWidget::SvArduinoWidget(SvArduinoWidgetParams params, QWidget *parent) : 
  QWidget(parent)
{
  _params = params;
  
  setupUi();
  
  editIp->setText(_params.ip);
  spinPort->setValue(_params.port);
  rbClockwise->setChecked(_params.spin_clockwise);
  rbContraClockwise->setChecked(!_params.spin_clockwise);
  sliderSpinSpeed->setValue(_params.spin_speed);
  gbTurnAngle->setEnabled(_params.turn_angle_enable);
  spinTurnAngle->setValue(_params.turn_angle);
  gbTurnCount->setEnabled(_params.turn_count_enable);
  spinTurnCount->setValue(_params.turn_count);
  gbTemperature->setEnabled(_params.temperature_period_enable);
  spinTemperaturePeriod->setValue(_params.temperature_period);
  spinVoltage->setValue(_params.voltage);
  
  
}

void svarduinomax::SvArduinoWidget::setupUi()
{
    verticalLayout_7 = new QVBoxLayout(this);
    verticalLayout_7->setObjectName(QStringLiteral("verticalLayout_7"));
    gbMain = new QGroupBox(this);
    gbMain->setObjectName(QStringLiteral("gbMain"));
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(gbMain->sizePolicy().hasHeightForWidth());
    gbMain->setSizePolicy(sizePolicy);
    verticalLayout = new QVBoxLayout(gbMain);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    horizontalLayout_5 = new QHBoxLayout();
    horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
    gbNetworkParams = new QGroupBox(gbMain);
    gbNetworkParams->setObjectName(QStringLiteral("gbNetworkParams"));
    verticalLayout_4 = new QVBoxLayout(gbNetworkParams);
    verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
    horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
    lblIp = new QLabel(gbNetworkParams);
    lblIp->setObjectName(QStringLiteral("lblIp"));
    lblIp->setMinimumSize(QSize(70, 0));
    lblIp->setMaximumSize(QSize(70, 16777215));
    lblIp->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    horizontalLayout_2->addWidget(lblIp);

    editIp = new QLineEdit(gbNetworkParams);
    editIp->setObjectName(QStringLiteral("editIp"));

    horizontalLayout_2->addWidget(editIp);


    verticalLayout_4->addLayout(horizontalLayout_2);

    horizontalLayout_3 = new QHBoxLayout();
    horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
    lblPort = new QLabel(gbNetworkParams);
    lblPort->setObjectName(QStringLiteral("lblPort"));
    lblPort->setMinimumSize(QSize(70, 0));
    lblPort->setMaximumSize(QSize(70, 16777215));
    lblPort->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    horizontalLayout_3->addWidget(lblPort);

    spinPort = new QSpinBox(gbNetworkParams);
    spinPort->setObjectName(QStringLiteral("spinPort"));
    spinPort->setMinimum(1);
    spinPort->setMaximum(65535);
    spinPort->setValue(35580);

    horizontalLayout_3->addWidget(spinPort);


    verticalLayout_4->addLayout(horizontalLayout_3);


    horizontalLayout_5->addWidget(gbNetworkParams);

    bnStartStop = new QPushButton(gbMain);
    bnStartStop->setObjectName(QStringLiteral("bnStartStop"));
    QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Expanding);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(bnStartStop->sizePolicy().hasHeightForWidth());
    bnStartStop->setSizePolicy(sizePolicy1);
    bnStartStop->setCheckable(true);
   
    horizontalLayout_5->addWidget(bnStartStop);


    verticalLayout->addLayout(horizontalLayout_5);

    gbSpinDirection = new QGroupBox(gbMain);
    gbSpinDirection->setObjectName(QStringLiteral("gbSpinDirection"));
    verticalLayout_2 = new QVBoxLayout(gbSpinDirection);
    verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
    rbClockwise = new QRadioButton(gbSpinDirection);
    rbClockwise->setObjectName(QStringLiteral("rbClockwise"));
    rbClockwise->setChecked(true);

    verticalLayout_2->addWidget(rbClockwise);

    rbContraClockwise = new QRadioButton(gbSpinDirection);
    rbContraClockwise->setObjectName(QStringLiteral("rbContraClockwise"));

    verticalLayout_2->addWidget(rbContraClockwise);


    verticalLayout->addWidget(gbSpinDirection);

    gbSpinSpeed = new QGroupBox(gbMain);
    gbSpinSpeed->setObjectName(QStringLiteral("gbSpinSpeed"));
    verticalLayout_3 = new QVBoxLayout(gbSpinSpeed);
    verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
    sliderSpinSpeed = new QSlider(gbSpinSpeed);
    sliderSpinSpeed->setObjectName(QStringLiteral("sliderSpinSpeed"));
    sliderSpinSpeed->setMinimum(1);
    sliderSpinSpeed->setMaximum(100);
    sliderSpinSpeed->setSingleStep(10);
    sliderSpinSpeed->setValue(100);
    sliderSpinSpeed->setOrientation(Qt::Horizontal);
    sliderSpinSpeed->setTickPosition(QSlider::TicksBothSides);

    verticalLayout_3->addWidget(sliderSpinSpeed);


    verticalLayout->addWidget(gbSpinSpeed);

    gbTurnAngle = new QGroupBox(gbMain);
    gbTurnAngle->setObjectName(QStringLiteral("gbTurnAngle"));
    gbTurnAngle->setCheckable(true);
    gbTurnAngle->setChecked(false);
    verticalLayout_5 = new QVBoxLayout(gbTurnAngle);
    verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
    spinTurnAngle = new QSpinBox(gbTurnAngle);
    spinTurnAngle->setObjectName(QStringLiteral("spinTurnAngle"));
    spinTurnAngle->setMinimum(18);
    spinTurnAngle->setMaximum(180000);
    spinTurnAngle->setValue(180);

    verticalLayout_5->addWidget(spinTurnAngle);


    verticalLayout->addWidget(gbTurnAngle);

    gbTurnCount = new QGroupBox(gbMain);
    gbTurnCount->setObjectName(QStringLiteral("gbTurnCount"));
    gbTurnCount->setCheckable(true);
    gbTurnCount->setChecked(false);
    verticalLayout_6 = new QVBoxLayout(gbTurnCount);
    verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
    spinTurnCount = new QSpinBox(gbTurnCount);
    spinTurnCount->setObjectName(QStringLiteral("spinTurnCount"));
    spinTurnCount->setMinimum(1);
    spinTurnCount->setMaximum(180000);
    spinTurnCount->setValue(1);

    verticalLayout_6->addWidget(spinTurnCount);


    verticalLayout->addWidget(gbTurnCount);

    gbTemperature = new QGroupBox(gbMain);
    gbTemperature->setObjectName(QStringLiteral("gbTemperature"));
    QFont font;
    font.setBold(false);
    font.setWeight(50);
    gbTemperature->setFont(font);
    gbTemperature->setCheckable(true);
    verticalLayout_9 = new QVBoxLayout(gbTemperature);
    verticalLayout_9->setObjectName(QStringLiteral("verticalLayout_9"));
    spinTemperaturePeriod = new QSpinBox(gbTemperature);
    spinTemperaturePeriod->setObjectName(QStringLiteral("spinTemperaturePeriod"));
    QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(spinTemperaturePeriod->sizePolicy().hasHeightForWidth());
    spinTemperaturePeriod->setSizePolicy(sizePolicy2);
    spinTemperaturePeriod->setFont(font);
    spinTemperaturePeriod->setMinimum(1);
    spinTemperaturePeriod->setMaximum(60);
    spinTemperaturePeriod->setValue(1);

    verticalLayout_9->addWidget(spinTemperaturePeriod);


    verticalLayout->addWidget(gbTemperature);

    horizontalLayout_4 = new QHBoxLayout();
    horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
    lblVoltage = new QLabel(gbMain);
    lblVoltage->setObjectName(QStringLiteral("lblVoltage"));
    lblVoltage->setMinimumSize(QSize(70, 0));
    lblVoltage->setMaximumSize(QSize(70, 16777215));
    lblVoltage->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    horizontalLayout_4->addWidget(lblVoltage);

    spinVoltage = new QSpinBox(gbMain);
    spinVoltage->setObjectName(QStringLiteral("spinVoltage"));
    spinVoltage->setEnabled(false);
    spinVoltage->setStyleSheet(QStringLiteral("background-color: rgb(255, 255, 127);"));
    spinVoltage->setMinimum(5);
    spinVoltage->setMaximum(24);
    spinVoltage->setValue(12);

    horizontalLayout_4->addWidget(spinVoltage);


    verticalLayout->addLayout(horizontalLayout_4);


    verticalLayout_7->addWidget(gbMain);

    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
    bnSendCmd = new QPushButton(this);
    bnSendCmd->setObjectName(QStringLiteral("bnSendCmd"));

    horizontalLayout->addWidget(bnSendCmd);

    editCmd = new QLineEdit(this);
    editCmd->setObjectName(QStringLiteral("editCmd"));

    horizontalLayout->addWidget(editCmd);

    horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout->addItem(horizontalSpacer);

    bnApply = new QPushButton(this);
    bnApply->setObjectName(QStringLiteral("bnApply"));

    horizontalLayout->addWidget(bnApply);


    verticalLayout_7->addLayout(horizontalLayout);

    textLog = new QTextEdit(this);
    textLog->setObjectName(QStringLiteral("textLog"));

    verticalLayout_7->addWidget(textLog);


    QMetaObject::connectSlotsByName(this);
 // setupUi

// retranslateUi(QWidget *Form)
    gbMain->setTitle(QString());
    gbNetworkParams->setTitle(QApplication::translate("Form", "\320\241\320\265\321\202\320\265\320\262\321\213\320\265 \320\277\320\260\321\200\320\260\320\274\320\265\321\202\321\200\321\213", Q_NULLPTR));
    lblIp->setText(QApplication::translate("Form", "IP \320\260\320\264\321\200\320\265\321\201", Q_NULLPTR));
    editIp->setText(QString());
    editIp->setPlaceholderText(QApplication::translate("Form", "192.168.100.100", Q_NULLPTR));
    lblPort->setText(QApplication::translate("Form", "\320\237\320\276\321\200\321\202", Q_NULLPTR));
    spinPort->setSuffix(QString());
    bnStartStop->setText(QApplication::translate("Form", "\320\241\321\202\320\260\321\200\321\202", Q_NULLPTR));
    gbSpinDirection->setTitle(QApplication::translate("Form", "\320\235\320\260\320\277\321\200\320\260\320\262\320\273\320\265\320\275\320\270\320\265 \320\262\321\200\320\260\321\211\320\265\320\275\320\270\321\217", Q_NULLPTR));
    rbClockwise->setText(QApplication::translate("Form", "\320\237\320\276 \321\207\320\260\321\201\320\276\320\262\320\276\320\271 \321\201\321\202\321\200\320\265\320\273\320\272\320\265", Q_NULLPTR));
    rbContraClockwise->setText(QApplication::translate("Form", "\320\237\321\200\320\276\321\202\320\270\320\262 \321\207\320\260\321\201\320\276\320\262\320\276\320\271 \321\201\321\202\321\200\320\265\320\273\320\272\320\270", Q_NULLPTR));
    gbSpinSpeed->setTitle(QApplication::translate("Form", "\320\241\320\272\320\276\321\200\320\276\321\201\321\202\321\214 \320\262\321\200\320\260\321\211\320\265\320\275\320\270\321\217, %", Q_NULLPTR));
    gbTurnAngle->setTitle(QApplication::translate("Form", "\320\243\320\263\320\276\320\273 \320\277\320\276\320\262\320\276\321\200\320\276\321\202\320\260", Q_NULLPTR));
    spinTurnAngle->setSuffix(QApplication::translate("Form", " \320\263\321\200\320\260\320\264.", Q_NULLPTR));
    gbTurnCount->setTitle(QApplication::translate("Form", "\320\232\320\276\320\273\320\270\321\207\320\265\321\201\321\202\320\262\320\276 \320\276\320\261\320\276\321\200\320\276\321\202\320\276\320\262", Q_NULLPTR));
    spinTurnCount->setSuffix(QApplication::translate("Form", " \320\276\320\261\320\276\321\200.", Q_NULLPTR));
    gbTemperature->setTitle(QApplication::translate("Form", "\320\227\320\260\320\277\321\200\320\260\321\210\320\270\320\262\320\260\321\202\321\214 \321\202\320\265\320\274\320\277\320\265\321\200\320\260\321\202\321\203\321\200\321\203", Q_NULLPTR));
    spinTemperaturePeriod->setSuffix(QApplication::translate("Form", " \321\200\320\260\320\267 \320\262 \320\274\320\270\320\275\321\203\321\202\321\203", Q_NULLPTR));
    lblVoltage->setText(QApplication::translate("Form", "\320\235\320\260\320\277\321\200\321\217\320\266\320\265\320\275\320\270\320\265", Q_NULLPTR));
    spinVoltage->setSuffix(QApplication::translate("Form", " \320\262\320\276\320\273\321\214\321\202", Q_NULLPTR));
    bnSendCmd->setText(QApplication::translate("Form", "\320\236\321\202\320\277\321\200\320\260\320\262\320\270\321\202\321\214 \320\272\320\276\320\274\320\260\320\275\320\264\321\203", Q_NULLPTR));
    bnApply->setText(QApplication::translate("Form", "\320\237\321\200\320\270\320\274\320\265\320\275\320\270\321\202\321\214", Q_NULLPTR));
// retranslateUi

}

void svarduinomax::SvArduinoWidget::on_bnStartStop_clicked(bool checked)
{
  qDebug() << checked;
}

void svarduinomax::SvArduinoWidget::on_rbClockwise_clicked(bool checked)
{
    
}

void svarduinomax::SvArduinoWidget::on_rbContraClockwise_clicked(bool checked)
{
    
}

void svarduinomax::SvArduinoWidget::on_sliderSpinSpeed_valueChanged(int value)
{
    
}

void svarduinomax::SvArduinoWidget::on_gbTurnAngle_clicked(bool checked)
{
    
}

void svarduinomax::SvArduinoWidget::on_spinTurnAngle_valueChanged(int arg1)
{
    
}

void svarduinomax::SvArduinoWidget::on_gbTurnCount_clicked(bool checked)
{
    
}

void svarduinomax::SvArduinoWidget::on_spinTurnCount_valueChanged(int arg1)
{
    
}

void svarduinomax::SvArduinoWidget::on_gbTemperature_clicked(bool checked)
{
    
}

void svarduinomax::SvArduinoWidget::on_spinTemperaturePeriod_valueChanged(int arg1)
{
    
}

void svarduinomax::SvArduinoWidget::on_bnSendCmd_clicked()
{
    
}

void svarduinomax::SvArduinoWidget::on_editCmd_returnPressed()
{
    
}

void svarduinomax::SvArduinoWidget::on_bnApply_clicked()
{
    
}

void svarduinomax::SvArduinoWidget::on_editIp_textChanged(const QString &arg1)
{
    
}

void svarduinomax::SvArduinoWidget::on_spinPort_valueChanged(int arg1)
{
    
}