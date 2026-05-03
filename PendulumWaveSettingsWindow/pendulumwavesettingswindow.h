#ifndef PENDULUMWAVESETTINGSWINDOW_H
#define PENDULUMWAVESETTINGSWINDOW_H

#include <QWidget>
#include "core.h"

class QComboBox;
class QPushButton;
class QCheckBox;

class PendulumWaveSettingsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PendulumWaveSettingsWindow(Core::PendulumWaveSettings* settings, Core::PendulumWaveDynamicParams* dynParams, QWidget *parent = nullptr);

private:
    Core::PendulumWaveSettings* settings;
    Core::PendulumWaveDynamicParams* dynParams;
    QComboBox* directionCombo;
    QPushButton* btnColor;
    QCheckBox* chkRainbow;
    QCheckBox* chkFlatMode;
};

#endif
