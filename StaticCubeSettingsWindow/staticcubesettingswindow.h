#ifndef STATICCUBESETTINGSWINDOW_H
#define STATICCUBESETTINGSWINDOW_H

#include <QWidget>
#include "core.h"

class QSlider;
class QPushButton;
class QCheckBox;
class QLabel;
class QComboBox;

class StaticCubeSettingsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit StaticCubeSettingsWindow(Core::StaticCubeSettings* settings,
                                      Core::StaticCubeDynamicParams* params,
                                      QWidget *parent = nullptr);

private slots:
    void onSizeChanged(int val);
    void onThicknessChanged(int val);
    void onDirectionChanged(int index); // YENİ SLOT
    void onWireframeChanged(int state);
    void pickColorX();
    void pickColorY();
    void pickColorZ();

private:
    Core::StaticCubeSettings* settings;
    Core::StaticCubeDynamicParams* params;

    QSlider *sizeSlider, *thickSlider;
    QComboBox *directionComboBox; // YENİ
    QPushButton *btnColX, *btnColY, *btnColZ;
    QCheckBox *chkWireframe;
    QLabel *lblSize, *lblThick;

    void updateColorBtn(QPushButton* btn, const QColor& c);
};

#endif // STATICCUBESETTINGSWINDOW_H
