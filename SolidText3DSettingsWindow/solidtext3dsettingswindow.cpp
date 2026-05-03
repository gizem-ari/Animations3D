#include "solidtext3dsettingswindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QFontComboBox>
#include <QSlider>
#include <QPushButton>
#include <QColorDialog>
#include <QTimer>
#include <QScrollArea>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>

SolidText3DSettingsWindow::SolidText3DSettingsWindow(Core::SolidText3DSettings* s, Core::SolidText3DDynamicParams* dp, const QString &title, QWidget *parent)
    : QWidget(parent), settings(s), dynParams(dp)
{
    setupUI(title);
    applyStyles();

    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &SolidText3DSettingsWindow::checkThreadUpdates);
    updateTimer->start(100);
}

void SolidText3DSettingsWindow::setupUI(const QString &title)
{
    QVBoxLayout *baseLayout = new QVBoxLayout(this);
    baseLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    baseLayout->addWidget(scrollArea);

    scrollAreaContentWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(scrollAreaContentWidget);
    scrollArea->setWidget(scrollAreaContentWidget);

    titleLabel = new QLabel(title);
    mainLayout->addWidget(titleLabel);

    QHBoxLayout* hText = new QHBoxLayout();
    hText->addWidget(new QLabel("Text:"));
    txtInput = new QLineEdit(settings->text);
    hText->addWidget(txtInput);
    mainLayout->addLayout(hText);

    QHBoxLayout* hFont = new QHBoxLayout();
    hFont->addWidget(new QLabel("Font:"));
    fontCombo = new QFontComboBox();
    fontCombo->setCurrentFont(QFont(settings->fontFamily));
    hFont->addWidget(fontCombo);
    mainLayout->addLayout(hFont);

    QHBoxLayout* hSize = new QHBoxLayout();
    hSize->addWidget(new QLabel("Font Size:"));
    fontSizeSpin = new QSpinBox();
    fontSizeSpin->setRange(10, 300);
    fontSizeSpin->setValue(settings->fontSize);
    fontSizeSpin->setEnabled(!settings->autoScale);
    hSize->addWidget(fontSizeSpin);

    autoScaleCheck = new QCheckBox("Auto Scale");
    autoScaleCheck->setChecked(settings->autoScale);
    hSize->addWidget(autoScaleCheck);
    mainLayout->addLayout(hSize);

    QHBoxLayout* hDynSize = new QHBoxLayout();
    hDynSize->addWidget(new QLabel("Calculated Size:"));
    lblDynamicSize = new QLabel(QString::number(dynParams->calculatedFontSize));
    hDynSize->addWidget(lblDynamicSize);
    mainLayout->addLayout(hDynSize);

    QHBoxLayout* hThick = new QHBoxLayout();
    hThick->addWidget(new QLabel("Thickness:"));
    lblThick = new QLabel(QString::number(settings->thickness * 100));
    hThick->addWidget(lblThick);
    mainLayout->addLayout(hThick);
    thickSlider = new QSlider(Qt::Horizontal);
    thickSlider->setRange(1, 100);
    thickSlider->setValue(settings->thickness * 100);
    mainLayout->addWidget(thickSlider);

    QHBoxLayout* hAxis = new QHBoxLayout();
    hAxis->addWidget(new QLabel("Rotation Direction:"));
    axisCombo = new QComboBox();
    axisCombo->addItems({"X+", "X-", "Y+", "Y-", "Z+", "Z-"});
    axisCombo->setCurrentIndex(static_cast<int>(settings->direction));
    hAxis->addWidget(axisCombo);
    resetDirCheck = new QCheckBox("Reset on Change");
    resetDirCheck->setChecked(settings->resetOnDirectionChange);
    hAxis->addWidget(resetDirCheck);
    mainLayout->addLayout(hAxis);

    QHBoxLayout* hSpeed = new QHBoxLayout();
    hSpeed->addWidget(new QLabel("Speed Factor (0 = Flat):"));
    lblSpeed = new QLabel(QString::number(settings->speedFactor, 'f', 1));
    hSpeed->addWidget(lblSpeed);
    mainLayout->addLayout(hSpeed);
    speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setRange(0, 50);
    speedSlider->setValue(settings->speedFactor * 10);
    mainLayout->addWidget(speedSlider);

    QHBoxLayout* hGizmo = new QHBoxLayout();
    gizmoCheck = new QCheckBox("Show 3D Gizmo Reference");
    gizmoCheck->setChecked(settings->showGizmo);
    hGizmo->addWidget(gizmoCheck);
    mainLayout->addLayout(hGizmo);

    QHBoxLayout* hColors = new QHBoxLayout();
    btnColorFront = new QPushButton("Front Color");
    btnColorBack = new QPushButton("Back Color");
    updateColorBtn(btnColorFront, settings->colorFront);
    updateColorBtn(btnColorBack, settings->colorBack);
    hColors->addWidget(btnColorFront);
    hColors->addWidget(btnColorBack);
    mainLayout->addLayout(hColors);

    connect(txtInput, &QLineEdit::textChanged, this, &SolidText3DSettingsWindow::onTextChanged);
    connect(fontCombo, &QFontComboBox::currentFontChanged, this, &SolidText3DSettingsWindow::onFontChanged);
    connect(fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SolidText3DSettingsWindow::onFontSizeChanged);
    connect(autoScaleCheck, &QCheckBox::toggled, this, &SolidText3DSettingsWindow::onAutoScaleToggled);
    connect(thickSlider, &QSlider::valueChanged, this, &SolidText3DSettingsWindow::onThicknessChanged);
    connect(axisCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SolidText3DSettingsWindow::onDirectionChanged);
    connect(speedSlider, &QSlider::valueChanged, this, &SolidText3DSettingsWindow::onSpeedFactorChanged);
    connect(gizmoCheck, &QCheckBox::toggled, this, &SolidText3DSettingsWindow::onGizmoToggled);
    connect(btnColorFront, &QPushButton::clicked, this, &SolidText3DSettingsWindow::onColorFrontClicked);
    connect(btnColorBack, &QPushButton::clicked, this, &SolidText3DSettingsWindow::onColorBackClicked);
    connect(resetDirCheck, &QCheckBox::toggled, this, &SolidText3DSettingsWindow::onResetDirToggled);

    mainLayout->addStretch();
}

void SolidText3DSettingsWindow::applyStyles()
{
    scrollAreaContentWidget->setObjectName("scrollAreaContentWidget");
    titleLabel->setObjectName("titleLabel");
    QString globalStyleSheet = QLatin1String(R"(
        QMainWindow, QWidget#scrollAreaContentWidget { background-color: rgba(40, 40, 40, 255); }
        QScrollArea { border: none; background: transparent; }
        QScrollBar:vertical { background: rgba(10,10,10,255); border: 1px solid gray; border-radius: 3px; width: 16px; margin: 0; }
        QScrollBar::handle:vertical { background-color: rgba(240,130,42,255); min-height: 20px; border-radius: 3px; }
        QLabel#titleLabel { color: rgb(240, 130, 32); background: transparent; font-size: 13pt; font-weight: bold; qproperty-alignment: 'AlignLeft'; }
        QLabel { background: transparent; color: white; font-weight: bold; }
        QLineEdit, QComboBox { background: rgb(0, 26, 30); color: rgb(250, 140, 42); font-weight: bold; border: 1px solid rgb(100, 100, 100); border-radius: 3px; min-height: 20px; }
        QAbstractSpinBox { background: rgb(0, 26, 30); color: rgb(250, 140, 42); font-weight: bold; border: 1px solid rgb(100, 100, 100); border-radius: 3px; min-height: 20px; min-width: 60px; }
        QAbstractSpinBox QLineEdit { background-color: transparent; color: rgb(250, 140, 42); padding-right: 19px; alignment: AlignRight; }
        QAbstractSpinBox::up-button, QAbstractSpinBox::down-button { background: rgb(0,0,0); border: none; width: 15px; border-radius: 2px; subcontrol-origin: border; }
        QAbstractSpinBox::up-button { subcontrol-position: top right; right: 1px; top: 1px; }
        QAbstractSpinBox::down-button { subcontrol-position: bottom right; right: 1px; bottom: 1px; }
        QAbstractSpinBox::up-button:hover, QAbstractSpinBox::down-button:hover { background: rgb(255, 150, 50); }
        QSlider::groove:horizontal { border: 1px solid #999999; height: 6px; background: rgb(0, 26, 30); border-radius: 3px; }
        QSlider::handle:horizontal { background: rgb(250, 140, 42); width: 14px; margin: -4px 0; border-radius: 7px; }
        QCheckBox { color: white; font-weight: bold; }
        QCheckBox::indicator { width: 15px; height: 15px; border: 1px solid #777; border-radius: 3px; background: rgb(0, 26, 30); }
        QCheckBox::indicator:checked { background: rgb(250, 140, 42); }
    )");
    this->setStyleSheet(globalStyleSheet);
}

void SolidText3DSettingsWindow::onTextChanged(const QString &text) { settings->text = text; }
void SolidText3DSettingsWindow::onFontChanged(const QFont &font) { settings->fontFamily = font.family(); }
void SolidText3DSettingsWindow::onFontSizeChanged(int val) { settings->fontSize = val; }
void SolidText3DSettingsWindow::onAutoScaleToggled(bool checked) { settings->autoScale = checked; fontSizeSpin->setEnabled(!checked); }
void SolidText3DSettingsWindow::onThicknessChanged(int val) { settings->thickness = val / 100.0; lblThick->setText(QString::number(val)); }
void SolidText3DSettingsWindow::onDirectionChanged(int index) { settings->direction = static_cast<Core::_3D_AxisDirection>(index); }
void SolidText3DSettingsWindow::onSpeedFactorChanged(int val) { settings->speedFactor = val / 10.0; lblSpeed->setText(QString::number(settings->speedFactor, 'f', 1)); }
void SolidText3DSettingsWindow::onGizmoToggled(bool checked) { settings->showGizmo = checked; }

void SolidText3DSettingsWindow::onColorFrontClicked() {
    QColor c = QColorDialog::getColor(settings->colorFront, this, "Select Front Color");
    if(c.isValid()) { settings->colorFront = c; updateColorBtn(btnColorFront, c); }
}

void SolidText3DSettingsWindow::onColorBackClicked() {
    QColor c = QColorDialog::getColor(settings->colorBack, this, "Select Back Color");
    if(c.isValid()) { settings->colorBack = c; updateColorBtn(btnColorBack, c); }
}

void SolidText3DSettingsWindow::updateColorBtn(QPushButton* btn, const QColor& c) {
    QString style = QString("background-color: %1; color: %2; font-weight: bold; border-radius: 3px; padding: 5px;")
    .arg(c.name())
        .arg(c.lightness() < 128 ? "white" : "black");
    btn->setStyleSheet(style);
}

void SolidText3DSettingsWindow::checkThreadUpdates() {
    QString currentSizeStr = QString::number(dynParams->calculatedFontSize);
    if (lblDynamicSize->text() != currentSizeStr) {
        lblDynamicSize->setText(currentSizeStr);
    }
}
void SolidText3DSettingsWindow::onResetDirToggled(bool checked) { settings->resetOnDirectionChange = checked; }
