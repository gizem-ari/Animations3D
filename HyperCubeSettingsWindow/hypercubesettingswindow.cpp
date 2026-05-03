#include "hypercubesettingswindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QColorDialog>
#include <QCheckBox>
#include <QComboBox>

HyperCubeSettingsWindow::HyperCubeSettingsWindow(Core::HyperCubeSettings* s, const QString &title, QWidget *parent)
    : QWidget(parent), settings(s)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
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

    // Size: 1 - 50
    addSlider("Size", 1, 50, settings->size * 10, sizeSlider, lblSize);
    connect(sizeSlider, &QSlider::valueChanged, this, &HyperCubeSettingsWindow::onSizeChanged);

    // Thickness: 1 - 100
    addSlider("Thickness", 1, 100, settings->thickness * 1000, thickSlider, lblThick);
    connect(thickSlider, &QSlider::valueChanged, this, &HyperCubeSettingsWindow::onThicknessChanged);

    // Speed Factor: 1 - 10 (Arka planda 5 değeri = 1.0x hız)
    addSlider("Speed Factor", 1, 10, settings->speedFactor * 5, speedFactorSlider, lblSpeedFactor);
    connect(speedFactorSlider, &QSlider::valueChanged, this, &HyperCubeSettingsWindow::onSpeedFactorChanged);

    mainLayout->addWidget(new QLabel("<b>Dominant Axis</b>"));
    directionComboBox = new QComboBox();
    directionComboBox->addItem("Z+ Axis");
    directionComboBox->addItem("Z- Axis");
    directionComboBox->addItem("X+ Axis");
    directionComboBox->addItem("X- Axis");
    directionComboBox->addItem("Y+ Axis");
    directionComboBox->addItem("Y- Axis");

    directionComboBox->setCurrentIndex(static_cast<int>(settings->direction));
    connect(directionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &HyperCubeSettingsWindow::onDirectionChanged);
    mainLayout->addWidget(directionComboBox);

    mainLayout->addWidget(new QLabel("<b>Colors</b>"));
    auto addColBtn = [&](const QString& txt, QPushButton*& btn) {
        QHBoxLayout* h = new QHBoxLayout();
        h->addWidget(new QLabel(txt));
        btn = new QPushButton();
        btn->setFixedSize(50, 25);
        h->addWidget(btn);
        mainLayout->addLayout(h);
    };

    // Edge Color butonu arayüzden tamamen kaldırıldı.

    addColBtn("Face Color", btnFaceCol);
    connect(btnFaceCol, &QPushButton::clicked, this, &HyperCubeSettingsWindow::onFaceColorClicked);
    updateColorBtn(btnFaceCol, settings->faceColor);

    chkWireframe = new QCheckBox("Wireframe Only");
    chkWireframe->setChecked(settings->wireframeOnly);
    mainLayout->addWidget(chkWireframe);
    connect(chkWireframe, &QCheckBox::stateChanged, this, &HyperCubeSettingsWindow::onWireframeChanged);

    mainLayout->addStretch();
}

// SLOTS
void HyperCubeSettingsWindow::onSizeChanged(int val) {
    settings->size = val / 10.0;
    // ÇÖZÜM BURADA: Ekrana ayarlanan ondalıklı değeri değil, slider'ın saf tam sayısını yazdırıyoruz.
    lblSize->setText(QString::number(val));
}

void HyperCubeSettingsWindow::onThicknessChanged(int val) {
    settings->thickness = val / 1000.0;
    lblThick->setText(QString::number(val));
}

void HyperCubeSettingsWindow::onSpeedFactorChanged(int val) {
    settings->speedFactor = val / 5.0; // 1 = 0.2x (Çok Yavaş), 5 = 1.0x (Normal), 10 = 2.0x (Hızlı)
    lblSpeedFactor->setText(QString::number(val));
}

void HyperCubeSettingsWindow::onDirectionChanged(int index) {
    if(index >= 0 && index <= 5)
        settings->direction = static_cast<Core::_3D_AxisDirection>(index);
}

void HyperCubeSettingsWindow::onWireframeChanged(int state) {
    settings->wireframeOnly = (state == Qt::Checked);
}

void HyperCubeSettingsWindow::onFaceColorClicked() {
    QColor c = QColorDialog::getColor(settings->faceColor, this);
    if(c.isValid()) { settings->faceColor = c; updateColorBtn(btnFaceCol, c); }
}

void HyperCubeSettingsWindow::updateColorBtn(QPushButton* btn, const QColor& c) {
    btn->setStyleSheet(QString("background-color: %1; border: 1px solid gray;").arg(c.name()));
}
