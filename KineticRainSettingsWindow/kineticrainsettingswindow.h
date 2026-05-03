#ifndef KINETICRAINSETTINGSWINDOW_H
#define KINETICRAINSETTINGSWINDOW_H

#include <QWidget>
#include "core.h"

class QComboBox;
class QPushButton;
class QCheckBox;

class KineticRainSettingsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit KineticRainSettingsWindow(Core::KineticRainSettings* settings, Core::KineticRainDynamicParams* dynParams, QWidget *parent = nullptr);

private:
    Core::KineticRainSettings* settings;
    Core::KineticRainDynamicParams* dynParams;
    QComboBox* directionCombo;
    QPushButton* btnColor;
    QCheckBox* chkRippleMode;
    QCheckBox* chkRainbowMode;
};

#endif
