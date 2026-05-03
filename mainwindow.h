#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include "core.h"
#include "framerendererthread.h"
#include "simulationwidget.h"
#include "SettingsWindow/HyperCubeSettingsWindow/hypercubesettingswindow.h"
#include "SettingsWindow/StaticCubeSettingsWindow/staticcubesettingswindow.h"
#include "SettingsWindow/Text3DSettingsWindow/text3dsettingswindow.h"
#include "SettingsWindow/SolidText3DSettingsWindow/solidtext3dsettingswindow.h"

class QComboBox;
class QLabel;
class QGroupBox;
class QVBoxLayout;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // Thread'den gelen görüntüyü ekrana basar
    void updateDisplay(const QImage &img, const QVector<QColor> &colors);
    //void onFrameReady(const QImage &img, const QVector<QColor> &colors);
    void onAnimationChanged(int index);
    void onToggleSimulation();


private:
    FrameRendererThread *rendererThread;

    QComboBox *animSelector;
    QPushButton *btnToggleSim;
    QStackedWidget *viewStack;
    SimulationWidget *simWidget;
    QLabel *displayLabel;
    QGroupBox *settingsContainer;
    QVBoxLayout *settingsLayout;

    Core::AnimationParams m_params;
    Core::AnimationDynamicParams m_dynParams;

    QWidget *currentSettingsWidget = nullptr;
    bool isSimulationActive = false;

    void setupUI();
    void setupFakeLeds();
    void loadSettingsWidget(Core::AnimationType type);
};

#endif // MAINWINDOW_H
