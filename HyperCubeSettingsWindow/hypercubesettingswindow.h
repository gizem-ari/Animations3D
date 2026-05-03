#ifndef HYPERCUBESETTINGSWINDOW_H
#define HYPERCUBESETTINGSWINDOW_H

#include <QWidget>
#include "core.h"

class QSlider;
class QPushButton;
class QCheckBox;
class QLabel;
class QComboBox;

class HyperCubeSettingsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit HyperCubeSettingsWindow(Core::HyperCubeSettings* settings,
                                     const QString &title,
                                     QWidget *parent = nullptr);

private slots:
    void onSizeChanged(int val);
    void onThicknessChanged(int val);
    void onSpeedFactorChanged(int val);
    void onDirectionChanged(int index);
    void onWireframeChanged(int state);
    void onFaceColorClicked(); // Edge color silindi

private:
    Core::HyperCubeSettings* settings;

    QSlider *sizeSlider, *thickSlider;
    QSlider *speedFactorSlider;
    QComboBox *directionComboBox;

    QPushButton *btnFaceCol; // btnEdgeCol silindi
    QCheckBox *chkWireframe;
    QLabel *lblSize, *lblThick, *lblSpeedFactor;

    void updateColorBtn(QPushButton* btn, const QColor& c);
};

#endif // HYPERCUBESETTINGSWINDOW_H
