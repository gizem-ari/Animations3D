#include "staticcubesettingswindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QColorDialog>
#include <QCheckBox>
#include <QComboBox>

StaticCubeSettingsWindow::StaticCubeSettingsWindow(Core::StaticCubeSettings* s, Core::StaticCubeDynamicParams* p, QWidget *parent)
    : QWidget(parent), settings(s), params(p)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);

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

    addSlider("Size", 1, 50, settings->size * 10, sizeSlider, lblSize);
    connect(sizeSlider, &QSlider::valueChanged, this, &StaticCubeSettingsWindow::onSizeChanged);

    addSlider("Thickness", 1, 200, settings->thickness * 1000, thickSlider, lblThick);
    connect(thickSlider, &QSlider::valueChanged, this, &StaticCubeSettingsWindow::onThicknessChanged);

    // --- YÖN SEÇİMİ (COMBOBOX) ---
    mainLayout->addWidget(new QLabel("<b>Axis Direction</b>"));
    directionComboBox = new QComboBox();
    directionComboBox->addItem("Z+ (Blue Front)");
    directionComboBox->addItem("Z- (Blue Back)");
    directionComboBox->addItem("X+ (Red Front)");
    directionComboBox->addItem("X- (Red Back)");
    directionComboBox->addItem("Y+ (Green Top)");
    directionComboBox->addItem("Y- (Green Bottom)");

    // Mevcut ayarı seç
    directionComboBox->setCurrentIndex(static_cast<int>(settings->direction));
    connect(directionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StaticCubeSettingsWindow::onDirectionChanged);
    mainLayout->addWidget(directionComboBox);

    mainLayout->addWidget(new QLabel("<b>Axis Colors</b>"));

    auto addColBtn = [&](const QString& txt, QPushButton*& btn) {
        QHBoxLayout* h = new QHBoxLayout();
        h->addWidget(new QLabel(txt));
        btn = new QPushButton();
        btn->setFixedSize(50, 25);
        h->addWidget(btn);
        mainLayout->addLayout(h);
    };

    addColBtn("X Face (Red)", btnColX);
    connect(btnColX, &QPushButton::clicked, this, &StaticCubeSettingsWindow::pickColorX);
    updateColorBtn(btnColX, settings->xColor);

    addColBtn("Y Face (Green)", btnColY);
    connect(btnColY, &QPushButton::clicked, this, &StaticCubeSettingsWindow::pickColorY);
    updateColorBtn(btnColY, settings->yColor);

    addColBtn("Z Face (Blue)", btnColZ);
    connect(btnColZ, &QPushButton::clicked, this, &StaticCubeSettingsWindow::pickColorZ);
    updateColorBtn(btnColZ, settings->zColor);

    chkWireframe = new QCheckBox("Wireframe Only");
    chkWireframe->setChecked(settings->wireframeOnly);
    mainLayout->addWidget(chkWireframe);
    connect(chkWireframe, &QCheckBox::stateChanged, this, &StaticCubeSettingsWindow::onWireframeChanged);

    mainLayout->addStretch();
}

void StaticCubeSettingsWindow::onSizeChanged(int val) {
    settings->size = val / 10.0;
    lblSize->setText(QString::number(settings->size));
}
void StaticCubeSettingsWindow::onThicknessChanged(int val) {
    settings->thickness = val / 1000.0;
    lblThick->setText(QString::number(settings->thickness));
}
void StaticCubeSettingsWindow::onDirectionChanged(int index) {
    if(index >= 0 && index <= 5) {
        settings->direction = static_cast<Core::_3D_AxisDirection>(index);
    }
}
void StaticCubeSettingsWindow::onWireframeChanged(int state) {
    settings->wireframeOnly = (state == Qt::Checked);
}
void StaticCubeSettingsWindow::pickColorX() {
    QColor c = QColorDialog::getColor(settings->xColor, this);
    if(c.isValid()) { settings->xColor = c; updateColorBtn(btnColX, c); }
}
void StaticCubeSettingsWindow::pickColorY() {
    QColor c = QColorDialog::getColor(settings->yColor, this);
    if(c.isValid()) { settings->yColor = c; updateColorBtn(btnColY, c); }
}
void StaticCubeSettingsWindow::pickColorZ() {
    QColor c = QColorDialog::getColor(settings->zColor, this);
    if(c.isValid()) { settings->zColor = c; updateColorBtn(btnColZ, c); }
}
void StaticCubeSettingsWindow::updateColorBtn(QPushButton* btn, const QColor& c) {
    btn->setStyleSheet(QString("background-color: %1; border: 1px solid gray;").arg(c.name()));
}
