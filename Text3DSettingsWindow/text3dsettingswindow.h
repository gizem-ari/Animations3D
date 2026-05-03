#ifndef TEXT3DSETTINGSWINDOW_H
#define TEXT3DSETTINGSWINDOW_H

#include <QMainWindow>
#include <core.h>

class QGridLayout;
class QWidget;
class QLabel;
class QDoubleSpinBox;
class QSpinBox;
class QScrollArea;
class QLineEdit;
class QComboBox;
class QSlider;
class QCheckBox;
class QPushButton;

class Text3DSettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit Text3DSettingsWindow(Core::Text3DSettings *s, const QString &windowLabel, QWidget *parent = nullptr);
    ~Text3DSettingsWindow();

    void setParameters(Core::Text3DSettings *new_s, const QString &new_windowLabel);
    void updateFontSizeUI(int newSize);

private:
    void setupUI();
    void applyStyles();
    void setupConnections();
    void updateUIFromSettings();
    void updateButtonColor(QPushButton *button, const QColor &color);

    Core::Text3DSettings *settings;

    QScrollArea *scrollArea;
    QWidget *scrollAreaContentWidget;
    QGridLayout *layout;
    QLabel *titleLabel;

    QLineEdit *textEdit;
    QComboBox *fontFamilyComboBox;
    QComboBox *fontWeightComboBox;
    QComboBox *scrollDirectionComboBox;
    QComboBox *renderModeComboBox;
    QComboBox *oscillationStyleComboBox;

    QSpinBox *fontSizeSpinBox;
    QDoubleSpinBox *scaleSpinBox;
    QDoubleSpinBox *depthSpinBox;
    QDoubleSpinBox *colorChangeIntervalSpinBox;
    QDoubleSpinBox *oscillationAmountSpinBox;

    QSlider *speedSlider;
    QLabel *speedValueLabel;

    QCheckBox *autoColorFlowCheckBox;
    QCheckBox *rainbowModeCheckBox;
    QCheckBox *fitOnScreenCheckBox;
    QPushButton *btnTextColor;
    QPushButton *btnBackgroundColor;

private slots:
    void onTextChanged(const QString &value);
    void onFontFamilyChanged(const QString &value);
    void onFontWeightChanged(int index);
    void onFontSizeChanged(int value);
    void onScrollDirectionChanged(int index);
    void onRenderModeChanged(int index);
    void onOscillationStyleChanged(int index);
    void onScaleChanged(double value);
    void onDepthChanged(double value);
    void onColorChangeIntervalChanged(double value);
    void onOscillationAmountChanged(double value);
    void onSpeedChanged(int value);

    void onAutoColorFlowToggled(bool checked);
    void onRainbowModeToggled(bool checked);
    void onFitOnScreenToggled(bool checked);
    void onBtnTextColorClicked();
    void onBtnBackgroundColorClicked();
};

#endif
