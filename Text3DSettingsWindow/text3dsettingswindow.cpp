#include "text3dsettingswindow.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QScrollArea>
#include <QLineEdit>
#include <QComboBox>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QColorDialog>
#include <QFontDatabase>

Text3DSettingsWindow::Text3DSettingsWindow(Core::Text3DSettings *s, const QString &windowLabel, QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Text 3D Settings");

    setupUI();
    applyStyles();
    setParameters(s, windowLabel);
    setupConnections();
}
Text3DSettingsWindow::~Text3DSettingsWindow() {}

void Text3DSettingsWindow::setParameters(Core::Text3DSettings *new_s, const QString &new_windowLabel)
{
    this->settings = new_s;
    this->titleLabel->setText(new_windowLabel);
    updateUIFromSettings();
}

void Text3DSettingsWindow::updateUIFromSettings()
{
    if (!settings) return;

    QSignalBlocker blocker(this);

    textEdit->setText(settings->text);
    fontFamilyComboBox->setCurrentText(settings->fontFamily);

    int weightIndex = fontWeightComboBox->findData(settings->fontWeight);
    if (weightIndex >= 0) fontWeightComboBox->setCurrentIndex(weightIndex);

    fontSizeSpinBox->setValue(settings->fontSize);
    scrollDirectionComboBox->setCurrentIndex(static_cast<int>(settings->scrollDirection));

    int renderIdx = renderModeComboBox->findData(settings->renderMode);
    if (renderIdx >= 0) renderModeComboBox->setCurrentIndex(renderIdx);

    int oscIdx = oscillationStyleComboBox->findData(settings->oscillationStyle);
    if (oscIdx >= 0) oscillationStyleComboBox->setCurrentIndex(oscIdx);

    scaleSpinBox->setValue(settings->scale);
    depthSpinBox->setValue(settings->depth);
    colorChangeIntervalSpinBox->setValue(settings->colorChangeInterval);
    oscillationAmountSpinBox->setValue(settings->oscillationAmount);

    speedSlider->setValue(settings->speedFactor);
    speedValueLabel->setText(QString::number(settings->speedFactor));

    autoColorFlowCheckBox->setChecked(settings->autoColorFlow);
    rainbowModeCheckBox->setChecked(settings->rainbowMode);
    fitOnScreenCheckBox->setChecked(settings->fitOnScreen);

    updateButtonColor(btnTextColor, settings->textColor);
    updateButtonColor(btnBackgroundColor, settings->backgroundColor);

    btnTextColor->setEnabled(!settings->autoColorFlow && !settings->rainbowMode);
    colorChangeIntervalSpinBox->setEnabled(settings->autoColorFlow || settings->rainbowMode);
}

void Text3DSettingsWindow::updateButtonColor(QPushButton *button, const QColor &color)
{
    button->setStyleSheet(QString("background-color: %1; border: 1px solid #777; border-radius: 3px; min-height: 20px;").arg(color.name()));
}

void Text3DSettingsWindow::setupUI()
{
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setMinimumWidth(350); // Minimum genişliği daraltarak esneklik sağladık
    setCentralWidget(scrollArea);

    scrollAreaContentWidget = new QWidget();
    scrollArea->setWidget(scrollAreaContentWidget);

    layout = new QGridLayout(scrollAreaContentWidget);
    layout->setHorizontalSpacing(10);
    layout->setVerticalSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);

    titleLabel = new QLabel();
    layout->addWidget(titleLabel, 0, 0, 1, 2);

    int row = 1;
    int col = 0;

    // YENİ YERLEŞİM: Sıkışmayı önlemek için 3 yerine 2 Sütunlu Grid Algoritması kullanıyoruz
    auto addParameterGroup = [&](const QString &labelText, QWidget *widget, int colSpan = 1) {
        QVBoxLayout *vbox = new QVBoxLayout();
        vbox->setSpacing(3);
        if(!labelText.isEmpty()) {
            QLabel *lbl = new QLabel(labelText);
            lbl->setWordWrap(true); // Gerekirse yazıyı alt satıra indir
            vbox->addWidget(lbl);
        }
        vbox->addWidget(widget);

        layout->addLayout(vbox, row, col, 1, colSpan);
        col += colSpan;
        if (col >= 2) { // 2 sütun dolduğunda alt satıra geç
            col = 0;
            row++;
        }
    };

    // TAM SATIR (Span 2)
    textEdit = new QLineEdit();
    addParameterGroup("Display Text:", textEdit, 2);

    // YARI SATIR (Span 1)
    fontFamilyComboBox = new QComboBox();
    fontFamilyComboBox->addItems(QFontDatabase::families());
    addParameterGroup("Font Family:", fontFamilyComboBox, 1);

    fontWeightComboBox = new QComboBox();
    fontWeightComboBox->addItem("Light", QFont::Light);
    fontWeightComboBox->addItem("Normal", QFont::Normal);
    fontWeightComboBox->addItem("Bold", QFont::Bold);
    fontWeightComboBox->addItem("Black", QFont::Black);
    addParameterGroup("Font Weight:", fontWeightComboBox, 1);

    fontSizeSpinBox = new QSpinBox();
    fontSizeSpinBox->setRange(10, 300);
    addParameterGroup("Font Size:", fontSizeSpinBox, 1);

    fitOnScreenCheckBox = new QCheckBox("Fit / Contain");
    addParameterGroup("", fitOnScreenCheckBox, 1);

    // TAM SATIR (Span 2) - Uzun yazılı olduğu için
    scrollDirectionComboBox = new QComboBox();
    scrollDirectionComboBox->addItem("Left", Core::ScrollLeft);
    scrollDirectionComboBox->addItem("Right", Core::ScrollRight);
    scrollDirectionComboBox->addItem("Up", Core::ScrollUp);
    scrollDirectionComboBox->addItem("Down", Core::ScrollDown);
    scrollDirectionComboBox->addItem("Diag TL to BR", Core::ScrollDiagTL_BR);
    scrollDirectionComboBox->addItem("Diag TR to BL", Core::ScrollDiagTR_BL);
    scrollDirectionComboBox->addItem("Diag BL to TR", Core::ScrollDiagBL_TR);
    scrollDirectionComboBox->addItem("Diag BR to TL", Core::ScrollDiagBR_TL);
    scrollDirectionComboBox->addItem("Fly-By", Core::ScrollFlyBy);
    scrollDirectionComboBox->addItem("Abyss", Core::ScrollAbyss);
    scrollDirectionComboBox->addItem("Star Wars", Core::ScrollStarWars);
    scrollDirectionComboBox->addItem("3D Orbit", Core::ScrollOrbit);
    scrollDirectionComboBox->addItem("Tornado", Core::ScrollTornado);
    scrollDirectionComboBox->addItem("Z-Bounce", Core::ScrollZBounce);
    scrollDirectionComboBox->addItem("Wave (Sine)", Core::ScrollSineWave);
    scrollDirectionComboBox->addItem("Bouncing DVD", Core::ScrollBouncingDVD);
    scrollDirectionComboBox->addItem("Pendulum Swing", Core::ScrollPendulum);
    addParameterGroup("Direction:", scrollDirectionComboBox, 2);

    // TAM SATIR (Span 2)
    renderModeComboBox = new QComboBox();
    renderModeComboBox->addItem("Solid", Core::Solid);
    renderModeComboBox->addItem("Wireframe", Core::Wireframe);
    renderModeComboBox->addItem("Layered (2D)", Core::Layered);
    addParameterGroup("Render Mode:", renderModeComboBox, 2);

    // DİĞER KONTROLLER YARI SATIR (Span 1)
    speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setRange(1, 10);
    speedValueLabel = new QLabel("5");
    speedValueLabel->setFixedWidth(20);
    speedValueLabel->setAlignment(Qt::AlignCenter);
    QWidget *speedContainer = new QWidget();
    QHBoxLayout *speedLayout = new QHBoxLayout(speedContainer);
    speedLayout->setContentsMargins(0, 0, 0, 0);
    speedLayout->addWidget(speedSlider);
    speedLayout->addWidget(speedValueLabel);
    addParameterGroup("Speed Factor:", speedContainer, 1);

    oscillationStyleComboBox = new QComboBox();
    oscillationStyleComboBox->addItem("None", Core::OscillationNone);
    oscillationStyleComboBox->addItem("Gentle", Core::OscillationGentle);
    oscillationStyleComboBox->addItem("Dynamic", Core::OscillationDynamic);
    oscillationStyleComboBox->addItem("Chaotic", Core::OscillationChaotic);
    addParameterGroup("Oscillation:", oscillationStyleComboBox, 1);

    oscillationAmountSpinBox = new QDoubleSpinBox();
    addParameterGroup("Osc. Amount:", oscillationAmountSpinBox, 1);

    scaleSpinBox = new QDoubleSpinBox();
    addParameterGroup("Overall Scale:", scaleSpinBox, 1);

    depthSpinBox = new QDoubleSpinBox();
    addParameterGroup("3D Depth:", depthSpinBox, 1);

    colorChangeIntervalSpinBox = new QDoubleSpinBox();
    addParameterGroup("Color Interval (s):", colorChangeIntervalSpinBox, 1);

    autoColorFlowCheckBox = new QCheckBox("Auto Color Flow");
    addParameterGroup("", autoColorFlowCheckBox, 1);

    rainbowModeCheckBox = new QCheckBox("Rainbow RGB Flow");
    addParameterGroup("", rainbowModeCheckBox, 1);

    btnTextColor = new QPushButton();
    addParameterGroup("Static Color:", btnTextColor, 1);

    btnBackgroundColor = new QPushButton();
    addParameterGroup("Background Color:", btnBackgroundColor, 1);

    if (col != 0) { row++; }
    layout->setRowStretch(row, 1);

    scaleSpinBox->setRange(0.1, 10.0); scaleSpinBox->setSingleStep(0.1);
    depthSpinBox->setRange(1.0, 500.0); depthSpinBox->setSingleStep(1.0);
    colorChangeIntervalSpinBox->setRange(0.1, 60.0); colorChangeIntervalSpinBox->setSingleStep(0.5);
    oscillationAmountSpinBox->setRange(0.0, 5.0); oscillationAmountSpinBox->setSingleStep(0.1);
}

void Text3DSettingsWindow::applyStyles()
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

void Text3DSettingsWindow::setupConnections()
{
    connect(textEdit, &QLineEdit::textChanged, this, &Text3DSettingsWindow::onTextChanged);
    connect(fontFamilyComboBox, &QComboBox::currentTextChanged, this, &Text3DSettingsWindow::onFontFamilyChanged);
    connect(fontWeightComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Text3DSettingsWindow::onFontWeightChanged);
    connect(fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Text3DSettingsWindow::onFontSizeChanged);
    connect(scrollDirectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Text3DSettingsWindow::onScrollDirectionChanged);
    connect(renderModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Text3DSettingsWindow::onRenderModeChanged);
    connect(oscillationStyleComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Text3DSettingsWindow::onOscillationStyleChanged);
    connect(scaleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Text3DSettingsWindow::onScaleChanged);
    connect(depthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Text3DSettingsWindow::onDepthChanged);
    connect(colorChangeIntervalSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Text3DSettingsWindow::onColorChangeIntervalChanged);
    connect(oscillationAmountSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Text3DSettingsWindow::onOscillationAmountChanged);
    connect(speedSlider, &QSlider::valueChanged, this, &Text3DSettingsWindow::onSpeedChanged);
    connect(autoColorFlowCheckBox, &QCheckBox::toggled, this, &Text3DSettingsWindow::onAutoColorFlowToggled);
    connect(rainbowModeCheckBox, &QCheckBox::toggled, this, &Text3DSettingsWindow::onRainbowModeToggled);
    connect(fitOnScreenCheckBox, &QCheckBox::toggled, this, &Text3DSettingsWindow::onFitOnScreenToggled);
    connect(btnTextColor, &QPushButton::clicked, this, &Text3DSettingsWindow::onBtnTextColorClicked);
    connect(btnBackgroundColor, &QPushButton::clicked, this, &Text3DSettingsWindow::onBtnBackgroundColorClicked);
}

void Text3DSettingsWindow::onTextChanged(const QString &value) { if(settings) settings->text = value; }
void Text3DSettingsWindow::onFontFamilyChanged(const QString &value) { if(settings) settings->fontFamily = value; }
void Text3DSettingsWindow::onFontWeightChanged(int index) { if(settings) settings->fontWeight = fontWeightComboBox->itemData(index).toInt(); }
void Text3DSettingsWindow::onFontSizeChanged(int value) { if(settings) settings->fontSize = value; }
void Text3DSettingsWindow::onScrollDirectionChanged(int index) { if(settings) settings->scrollDirection = static_cast<Core::TextScrollDirection>(scrollDirectionComboBox->itemData(index).toInt()); }
void Text3DSettingsWindow::onRenderModeChanged(int index) {
    if(!settings) return;
    Core::TextRenderMode newMode = static_cast<Core::TextRenderMode>(renderModeComboBox->itemData(index).toInt());
    settings->renderMode = newMode;
}
void Text3DSettingsWindow::onOscillationStyleChanged(int index) { if(settings) settings->oscillationStyle = static_cast<Core::TextOscillationStyle>(oscillationStyleComboBox->itemData(index).toInt()); }
void Text3DSettingsWindow::onScaleChanged(double value) { if(settings) settings->scale = value; }
void Text3DSettingsWindow::onDepthChanged(double value) { if(settings) settings->depth = value; }
void Text3DSettingsWindow::onColorChangeIntervalChanged(double value) { if(settings) settings->colorChangeInterval = value; }
void Text3DSettingsWindow::onOscillationAmountChanged(double value) { if(settings) settings->oscillationAmount = value; }
void Text3DSettingsWindow::onSpeedChanged(int value) {
    if(settings) settings->speedFactor = value;
    speedValueLabel->setText(QString::number(value));
}

void Text3DSettingsWindow::onAutoColorFlowToggled(bool checked) {
    if(settings) settings->autoColorFlow = checked;
    btnTextColor->setEnabled(!settings->autoColorFlow && !settings->rainbowMode);
    colorChangeIntervalSpinBox->setEnabled(settings->autoColorFlow || settings->rainbowMode);
}

void Text3DSettingsWindow::onRainbowModeToggled(bool checked) {
    if(settings) settings->rainbowMode = checked;
    btnTextColor->setEnabled(!settings->autoColorFlow && !settings->rainbowMode);
    colorChangeIntervalSpinBox->setEnabled(settings->autoColorFlow || settings->rainbowMode);
}

void Text3DSettingsWindow::onFitOnScreenToggled(bool checked) {
    if(settings) settings->fitOnScreen = checked;
}

void Text3DSettingsWindow::onBtnTextColorClicked() {
    if(!settings) return;
    QColor color = QColorDialog::getColor(settings->textColor, this, "Select Static Text Color");
    if(color.isValid()) {
        settings->textColor = color;
        updateButtonColor(btnTextColor, color);
    }
}

void Text3DSettingsWindow::onBtnBackgroundColorClicked() {
    if(!settings) return;
    QColor color = QColorDialog::getColor(settings->backgroundColor, this, "Select Background Color");
    if(color.isValid()) {
        settings->backgroundColor = color;
        updateButtonColor(btnBackgroundColor, color);
    }
}
void Text3DSettingsWindow::updateFontSizeUI(int newSize)
{
    QSignalBlocker blocker(fontSizeSpinBox);
    fontSizeSpinBox->setValue(newSize);
}
