#include "wavefountain3dsettingswindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QColorDialog>
#include <QComboBox>

WaveFountain3DSettingsWindow::WaveFountain3DSettingsWindow(Core::WaveFountain3DSettings* s, const QString &title, QWidget *parent)
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
    connect(amplitudeSlider, &QSlider::valueChanged, this, &WaveFountain3DSettingsWindow::onAmplitudeChanged);

    addSlider("Wave Number", 1, 100, settings->waveNumber * 10, waveNumberSlider, lblWaveNumber);
    connect(waveNumberSlider, &QSlider::valueChanged, this, &WaveFountain3DSettingsWindow::onWaveNumberChanged);

    addSlider("Speed Factor", 1, 10, settings->speedFactor * 5, speedFactorSlider, lblSpeedFactor);
    connect(speedFactorSlider, &QSlider::valueChanged, this, &WaveFountain3DSettingsWindow::onSpeedFactorChanged);

    addSlider("Active Grid Size", 4, 40, settings->activeGridSize, gridSlider, lblGridSize);
    gridSlider->setSingleStep(2);
    connect(gridSlider, &QSlider::valueChanged, this, &WaveFountain3DSettingsWindow::onGridSizeChanged);
    onGridSizeChanged(settings->activeGridSize);

    mainLayout->addWidget(new QLabel("<b>Flow Direction</b>"));
    directionComboBox = new QComboBox();
    directionComboBox->addItem("Outward (+)");
    directionComboBox->addItem("Inward (-)");
    directionComboBox->setCurrentIndex(settings->direction % 2);
    connect(directionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WaveFountain3DSettingsWindow::onDirectionChanged);
    mainLayout->addWidget(directionComboBox);

    QHBoxLayout* colLayout = new QHBoxLayout();
    colLayout->addWidget(new QLabel("Water Color"));
    btnWaterCol = new QPushButton();
    btnWaterCol->setFixedSize(50, 25);
    colLayout->addWidget(btnWaterCol);
    mainLayout->addLayout(colLayout);
    connect(btnWaterCol, &QPushButton::clicked, this, &WaveFountain3DSettingsWindow::onWaterColorClicked);
    updateColorBtn(btnWaterCol, settings->waterColor);

    mainLayout->addStretch();
}

void WaveFountain3DSettingsWindow::onAmplitudeChanged(int val) { settings->amplitude = val / 100.0; lblAmplitude->setText(QString::number(val)); }
void WaveFountain3DSettingsWindow::onWaveNumberChanged(int val) { settings->waveNumber = val / 10.0; lblWaveNumber->setText(QString::number(val)); }
void WaveFountain3DSettingsWindow::onSpeedFactorChanged(int val) { settings->speedFactor = val / 5.0; lblSpeedFactor->setText(QString::number(val)); }
void WaveFountain3DSettingsWindow::onDirectionChanged(int index) { settings->direction = static_cast<Core::_3D_AxisDirection>(index); }
void WaveFountain3DSettingsWindow::onWaterColorClicked() { QColor c = QColorDialog::getColor(settings->waterColor, this); if (c.isValid()) { settings->waterColor = c; updateColorBtn(btnWaterCol, c); } }
void WaveFountain3DSettingsWindow::updateColorBtn(QPushButton* btn, const QColor& c) { btn->setStyleSheet(QString("background-color: %1; border: 1px solid gray;").arg(c.name())); }

void WaveFountain3DSettingsWindow::onGridSizeChanged(int val) {
    int snapped = (val % 2 == 0) ? val : val + 1;
    if (gridSlider->value() != snapped) {
        gridSlider->blockSignals(true);
        gridSlider->setValue(snapped);
        gridSlider->blockSignals(false);
    }
    settings->activeGridSize = snapped;
    lblGridSize->setText(QString::number(snapped) + " x " + QString::number(snapped));
}
