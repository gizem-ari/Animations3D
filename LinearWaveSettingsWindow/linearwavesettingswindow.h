#ifndef LINEARWAVESETTINGSWINDOW_H
#define LINEARWAVESETTINGSWINDOW_H

#include <QWidget>
#include "core.h"

class QSlider;
class QPushButton;
class QLabel;
class QComboBox;

class LinearWaveSettingsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LinearWaveSettingsWindow(Core::LinearWaveSettings* settings, const QString &title, QWidget *parent = nullptr);

private slots:
    void onAmplitudeChanged(int val);
    void onWaveNumberChanged(int val);
    void onSpeedFactorChanged(int val);
    void onDirectionChanged(int index);
    void onWaveColorClicked();
    void onGridSizeChanged(int val);

private:
    Core::LinearWaveSettings* settings;

    QSlider *amplitudeSlider;
    QSlider *waveNumberSlider;
    QSlider *speedFactorSlider;
    QSlider *gridSlider;
    QComboBox *directionComboBox;
    QPushButton *btnWaveCol;

    QLabel *lblAmplitude;
    QLabel *lblWaveNumber;
    QLabel *lblSpeedFactor;
    QLabel *lblGridSize;

    void updateColorBtn(QPushButton* btn, const QColor& c);
};

#endif
