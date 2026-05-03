#include "kineticrainsettingswindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QColorDialog>
#include <QCheckBox>
#include <QComboBox>

KineticRainSettingsWindow::KineticRainSettingsWindow(Core::KineticRainSettings* s, Core::KineticRainDynamicParams* dp, QWidget *parent)
    : QWidget(parent), settings(s), dynParams(dp)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);

    auto addSlider = [&](const QString& name, int min, int max, double multiplier, double currentVal, auto memberFunc) {
        QHBoxLayout* h = new QHBoxLayout();
        h->addWidget(new QLabel(name));
        QLabel* valLbl = new QLabel(QString::number(currentVal));
        h->addWidget(valLbl);
        QSlider* slider = new QSlider(Qt::Horizontal);
        slider->setRange(min, max);
        slider->setValue(currentVal / multiplier);
        connect(slider, &QSlider::valueChanged, this, [=](int v) {
            double finalVal = v * multiplier;
            memberFunc(finalVal);
            valLbl->setText(QString::number(finalVal, 'f', 2));
        });
        mainLayout->addLayout(h);
        mainLayout->addWidget(slider);
    };

    addSlider("Drop Size", 1, 50, 0.01, settings->dropSize, [this](double v){ settings->dropSize = v; });
    addSlider("Wave Freq", 1, 50, 0.05, settings->waveFrequency, [this](double v){ settings->waveFrequency = v; });
    addSlider("Amplitude", 1, 100, 0.01, settings->amplitude, [this](double v){ settings->amplitude = v; });
    addSlider("Speed", 1, 50, 0.1, settings->speedFactor, [this](double v){ settings->speedFactor = v; });

    directionCombo = new QComboBox();
    directionCombo->addItems({"X+", "X-", "Y+", "Y-", "Z+", "Z-"});
    directionCombo->setCurrentIndex((int)settings->direction);
    connect(directionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int i){ settings->direction = (Core::_3D_AxisDirection)i; });
    mainLayout->addWidget(new QLabel("Gravity Axis:"));
    mainLayout->addWidget(directionCombo);

    btnColor = new QPushButton("Drop Color");
    connect(btnColor, &QPushButton::clicked, this, [this](){
        QColor c = QColorDialog::getColor(settings->dropColor, this);
        if(c.isValid()) settings->dropColor = c;
    });
    mainLayout->addWidget(btnColor);

    chkRippleMode = new QCheckBox("Ripple Mode (Center Out)");
    chkRippleMode->setChecked(settings->rippleMode);
    connect(chkRippleMode, &QCheckBox::toggled, this, [this](bool t){ settings->rippleMode = t; });
    mainLayout->addWidget(chkRippleMode);

    chkRainbowMode = new QCheckBox("Rainbow Mode");
    chkRainbowMode->setChecked(settings->rainbowMode);
    connect(chkRainbowMode, &QCheckBox::toggled, this, [this](bool t){ settings->rainbowMode = t; });
    mainLayout->addWidget(chkRainbowMode);

    mainLayout->addStretch();
}
