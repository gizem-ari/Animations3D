#ifndef WAVEFOUNTAIN3DSETTINGSWINDOW_H
#define WAVEFOUNTAIN3DSETTINGSWINDOW_H

#include <QWidget>
#include "core.h"

class QSlider;
class QPushButton;
class QLabel;
class QComboBox;

class WaveFountain3DSettingsWindow : public QWidget
{
    Q_OBJECT
public:
    explicit WaveFountain3DSettingsWindow(Core::WaveFountain3DSettings* settings, const QString &title, QWidget *parent = nullptr);

private slots:
    void onAmplitudeChanged(int val);
    void onWaveNumberChanged(int val);
    void onSpeedFactorChanged(int val);
    void onDirectionChanged(int index);
    void onWaterColorClicked();
    void onGridSizeChanged(int val);

private:
    Core::WaveFountain3DSettings* settings;
    QSlider *amplitudeSlider, *waveNumberSlider, *speedFactorSlider, *gridSlider;
    QComboBox *directionComboBox;
    QPushButton *btnWaterCol;
    QLabel *lblAmplitude, *lblWaveNumber, *lblSpeedFactor, *lblGridSize;
    void updateColorBtn(QPushButton* btn, const QColor& c);
};

#endif
