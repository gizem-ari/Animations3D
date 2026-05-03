#ifndef SOLIDTEXT3DSETTINGSWINDOW_H
#define SOLIDTEXT3DSETTINGSWINDOW_H

#include <QWidget>
#include "core.h"

class QLineEdit;
class QFontComboBox;
class QSlider;
class QPushButton;
class QLabel;
class QTimer;
class QScrollArea;
class QComboBox;
class QCheckBox;
class QSpinBox;

class SolidText3DSettingsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SolidText3DSettingsWindow(Core::SolidText3DSettings* settings, Core::SolidText3DDynamicParams* dynParams, const QString &title, QWidget *parent = nullptr);

private slots:
    void onTextChanged(const QString &text);
    void onFontChanged(const QFont &font);
    void onFontSizeChanged(int val);
    void onAutoScaleToggled(bool checked);
    void onThicknessChanged(int val);
    void onDirectionChanged(int index);
    void onSpeedFactorChanged(int val);
    void onGizmoToggled(bool checked);
    void onResetDirToggled(bool checked);
    void onColorFrontClicked();
    void onColorBackClicked();
    void checkThreadUpdates();

private:
    void setupUI(const QString &title);
    void applyStyles();
    void updateColorBtn(QPushButton* btn, const QColor& c);

    Core::SolidText3DSettings* settings;
    Core::SolidText3DDynamicParams* dynParams;

    QWidget* scrollAreaContentWidget;
    QLabel* titleLabel;

    QLineEdit *txtInput;
    QFontComboBox *fontCombo;
    QSpinBox *fontSizeSpin;
    QCheckBox *autoScaleCheck;
    QSlider *thickSlider;
    QComboBox *axisCombo;
    QSlider *speedSlider;
    QCheckBox *gizmoCheck;
    QCheckBox *resetDirCheck;
    QPushButton *btnColorFront, *btnColorBack;
    QLabel *lblDynamicSize, *lblThick, *lblSpeed;
    QTimer *updateTimer;
};

#endif
