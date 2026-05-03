#include "pendulumwavesettingswindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QColorDialog>
#include <QCheckBox>
#include <QComboBox>

PendulumWaveSettingsWindow::PendulumWaveSettingsWindow(Core::PendulumWaveSettings* s, Core::PendulumWaveDynamicParams* dp, QWidget *parent)
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

    addSlider("Base Freq", 1, 100, 0.01, settings->baseFrequency, [this](double v){ settings->baseFrequency = v; });
    addSlider("Freq Step", 0, 50, 0.005, settings->frequencyStep, [this](double v){ settings->frequencyStep = v; });
    addSlider("Amplitude", 1, 100, 0.01, settings->amplitude, [this](double v){ settings->amplitude = v; });
    addSlider("Sphere Size", 1, 50, 0.01, settings->sphereSize, [this](double v){ settings->sphereSize = v; });
    addSlider("Speed", 1, 50, 0.1, settings->speedFactor, [this](double v){ settings->speedFactor = v; });

    directionCombo = new QComboBox();
    directionCombo->addItems({"X+", "X-", "Y+", "Y-", "Z+", "Z-"});
    directionCombo->setCurrentIndex((int)settings->direction);
    connect(directionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int i){ settings->direction = (Core::_3D_AxisDirection)i; });
    mainLayout->addWidget(new QLabel("Array Direction:"));
    mainLayout->addWidget(directionCombo);

    btnColor = new QPushButton("Pick Color");
    connect(btnColor, &QPushButton::clicked, this, [this](){
        QColor c = QColorDialog::getColor(settings->color, this);
        if(c.isValid()) settings->color = c;
    });
    mainLayout->addWidget(btnColor);

    chkRainbow = new QCheckBox("Rainbow Mode");
    chkRainbow->setChecked(settings->rainbowMode);
    connect(chkRainbow, &QCheckBox::toggled, this, [this](bool t){ settings->rainbowMode = t; });
    mainLayout->addWidget(chkRainbow);

    chkFlatMode = new QCheckBox("Flat Mode (Cascade Start)");
    chkFlatMode->setChecked(settings->flatMode);
    connect(chkFlatMode, &QCheckBox::toggled, this, [this](bool t){
        settings->flatMode = t;
        if(t) {
            dynParams->internalTime = 0.0;
        }
    });
    mainLayout->addWidget(chkFlatMode);

    mainLayout->addStretch();
}
