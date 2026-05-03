#include "linearwavesettingswindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QColorDialog>
#include <QComboBox>

LinearWaveSettingsWindow::LinearWaveSettingsWindow(Core::LinearWaveSettings* s, const QString &title, QWidget *parent)
    : QWidget(parent), settings(s)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(new QLabel("<b>" + title + "</b>"));

    auto addSlider = [&](const QString& name, int min, int max, int val, QSlider*& slider, QLabel*& lbl) {
        QHBoxLayout* h = new QHBoxLayout();
        h->addWidget(new QLabel(name));
        lbl = new QLabel(QString::number(val));
        h->addWidget(lbl);
        slider = new QSlider(Qt::Horizontal);
        slider->setRange(min, max);
        slider->setValue(val);
        mainLayout->addLayout(h);
        mainLayout->addWidget(slider);
    };

    addSlider("Amplitude", 1, 100, settings->amplitude * 100, amplitudeSlider, lblAmplitude);
    connect(amplitudeSlider, &QSlider::valueChanged, this, &LinearWaveSettingsWindow::onAmplitudeChanged);

    addSlider("Wave Number", 1, 100, settings->waveNumber * 10, waveNumberSlider, lblWaveNumber);
    connect(waveNumberSlider, &QSlider::valueChanged, this, &LinearWaveSettingsWindow::onWaveNumberChanged);

    addSlider("Speed Factor", 1, 10, settings->speedFactor * 5, speedFactorSlider, lblSpeedFactor);
    connect(speedFactorSlider, &QSlider::valueChanged, this, &LinearWaveSettingsWindow::onSpeedFactorChanged);

    addSlider("Active Grid Size", 4, 40, settings->activeGridSize, gridSlider, lblGridSize);
    gridSlider->setSingleStep(2);
    connect(gridSlider, &QSlider::valueChanged, this, &LinearWaveSettingsWindow::onGridSizeChanged);
    onGridSizeChanged(settings->activeGridSize);

    mainLayout->addWidget(new QLabel("<b>Direction</b>"));
    directionComboBox = new QComboBox();
    directionComboBox->addItem("X+ Axis");
    directionComboBox->addItem("X- Axis");
    directionComboBox->addItem("Y+ Axis");
    directionComboBox->addItem("Y- Axis");
    directionComboBox->addItem("Z+ Axis");
    directionComboBox->addItem("Z- Axis");

    directionComboBox->setCurrentIndex(static_cast<int>(settings->direction));
    connect(directionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LinearWaveSettingsWindow::onDirectionChanged);
    mainLayout->addWidget(directionComboBox);

    QHBoxLayout* colLayout = new QHBoxLayout();
    colLayout->addWidget(new QLabel("Wave Color"));
    btnWaveCol = new QPushButton();
    btnWaveCol->setFixedSize(50, 25);
    colLayout->addWidget(btnWaveCol);
    mainLayout->addLayout(colLayout);

    connect(btnWaveCol, &QPushButton::clicked, this, &LinearWaveSettingsWindow::onWaveColorClicked);
    updateColorBtn(btnWaveCol, settings->waveColor);

    mainLayout->addStretch();
}

void LinearWaveSettingsWindow::onAmplitudeChanged(int val) {
    settings->amplitude = val / 100.0;
    lblAmplitude->setText(QString::number(val));
}

void LinearWaveSettingsWindow::onWaveNumberChanged(int val) {
    settings->waveNumber = val / 10.0;
    lblWaveNumber->setText(QString::number(val));
}

void LinearWaveSettingsWindow::onSpeedFactorChanged(int val) {
    settings->speedFactor = val / 5.0;
    lblSpeedFactor->setText(QString::number(val));
}

void LinearWaveSettingsWindow::onDirectionChanged(int index) {
    if (index >= 0 && index <= 5) {
        settings->direction = static_cast<Core::_3D_AxisDirection>(index);
    }
}

void LinearWaveSettingsWindow::onWaveColorClicked() {
    QColor c = QColorDialog::getColor(settings->waveColor, this);
    if (c.isValid()) {
        settings->waveColor = c;
        updateColorBtn(btnWaveCol, c);
    }
}

void LinearWaveSettingsWindow::updateColorBtn(QPushButton* btn, const QColor& c) {
    btn->setStyleSheet(QString("background-color: %1; border: 1px solid gray;").arg(c.name()));
}

void LinearWaveSettingsWindow::onGridSizeChanged(int val) {
    int snapped = (val % 2 == 0) ? val : val + 1;
    if (gridSlider->value() != snapped) {
        gridSlider->blockSignals(true);
        gridSlider->setValue(snapped);
        gridSlider->blockSignals(false);
    }
    settings->activeGridSize = snapped;
    lblGridSize->setText(QString::number(snapped) + " x " + QString::number(snapped));
}
