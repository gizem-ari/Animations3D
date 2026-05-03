#include "mainwindow.h"
#include "staticcubesettingswindow.h"
#include "hypercubesettingswindow.h"
#include "text3dsettingswindow.h"
#include "solidtext3dsettingswindow.h"
#include "linearwavesettingswindow.h"
#include "wavefountain3dsettingswindow.h"
#include "pendulumwavesettingswindow.h"
#include "kineticrainsettingswindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include <QResizeEvent>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUI();

    rendererThread = new FrameRendererThread(this);
    setupFakeLeds();

    // Parametre pointerlarını thread'e bağla
    rendererThread->staticSettings = &m_params.staticCubeSettings;
    rendererThread->staticParams = &m_dynParams.staticCubeDynamicParams;
    rendererThread->hyperSettings = &m_params.hyperCubeSettings;
    rendererThread->hyperParams = &m_dynParams.hyperCubeDynamicParams;
    rendererThread->text3DSettings = &m_params.text3DSettings;
    rendererThread->text3DParams = &m_dynParams.text3DDynamicParams;
    rendererThread->solidText3DSettings = &m_params.solidText3DSettings;
    rendererThread->solidText3DParams = &m_dynParams.solidText3DDynamicParams;
    rendererThread->linearWaveSettings = &m_params.linearWaveSettings;
    rendererThread->linearWaveParams = &m_dynParams.linearWaveDynamicParams;
    rendererThread->waveFountainSettings = &m_params.waveFountainSettings;
    rendererThread->waveFountainParams = &m_dynParams.waveFountainDynamicParams;
    rendererThread->pendulumSettings = &m_params.pendulumWaveSettings;
    rendererThread->pendulumParams = &m_dynParams.pendulumWaveDynamicParams;
    rendererThread->kineticRainSettings = &m_params.kineticRainSettings;
    rendererThread->kineticRainParams = &m_dynParams.kineticRainDynamicParams;

    connect(rendererThread, &FrameRendererThread::frameReady, this, &MainWindow::updateDisplay);
    connect(rendererThread, &FrameRendererThread::fontSizeAutoChanged, this, [this](int newSize) {
        if (currentSettingsWidget) {
            Text3DSettingsWindow* textWindow = qobject_cast<Text3DSettingsWindow*>(currentSettingsWidget);
            if (textWindow) {
                textWindow->updateFontSizeUI(newSize);
            }
        }
    });
    rendererThread->start();
}

MainWindow::~MainWindow()
{
    rendererThread->stop();
    rendererThread->wait();
}

void MainWindow::setupUI()
{
    QWidget *central = new QWidget;
    setCentralWidget(central);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);

    // Sol Taraf: Animasyon ve Önizleme
    QWidget *leftWidget = new QWidget;
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);

    QHBoxLayout *topControls = new QHBoxLayout;
    topControls->addWidget(new QLabel("Animation:"));

    animSelector = new QComboBox;
    animSelector->addItem("Static Cube", static_cast<int>(Core::AnimationType::StaticCube));
    animSelector->addItem("Hyper Cube", static_cast<int>(Core::AnimationType::HyperCube));
    animSelector->addItem("Text 3D Frame", static_cast<int>(Core::AnimationType::Text3D));
    animSelector->addItem("Solid Text 3D", static_cast<int>(Core::AnimationType::SolidText3D));
    animSelector->addItem("Linear Wave", static_cast<int>(Core::AnimationType::LinearWave));
    animSelector->addItem("Wave Fountain", static_cast<int>(Core::AnimationType::WaveFountain3D));
    animSelector->addItem("Pendulum Wave", static_cast<int>(Core::AnimationType::PendulumWave));
    animSelector->addItem("Kinetic Rain", static_cast<int>(Core::AnimationType::KineticRain));

    connect(animSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onAnimationChanged);
    topControls->addWidget(animSelector);

    btnToggleSim = new QPushButton("Show Simulation");
    btnToggleSim->setCheckable(true);
    connect(btnToggleSim, &QPushButton::clicked, this, &MainWindow::onToggleSimulation);
    topControls->addWidget(btnToggleSim);
    topControls->addStretch();

    leftLayout->addLayout(topControls);
    viewStack = new QStackedWidget;

    // 1. Sayfa: Klasik Önizleme Resmi
    displayLabel = new QLabel;
    displayLabel->setMinimumSize(800, 600);
    displayLabel->setStyleSheet("background-color: black; border: 2px solid #333;");
    displayLabel->setAlignment(Qt::AlignCenter);
    viewStack->addWidget(displayLabel);

    // 2. Sayfa: Yeni Simülasyon Ekranı
    simWidget = new SimulationWidget;
    simWidget->setMinimumSize(800, 600);
    simWidget->setStyleSheet("border: 2px solid #444;");
    viewStack->addWidget(simWidget);

    leftLayout->addWidget(viewStack);

    // Sağ Taraf: Ayarlar
    settingsContainer = new QGroupBox("Animation Settings");
    settingsContainer->setMinimumWidth(400);
    settingsContainer->setMaximumWidth(480);
    settingsLayout = new QVBoxLayout(settingsContainer);

    mainLayout->addWidget(leftWidget);
    mainLayout->addWidget(settingsContainer);
}

void MainWindow::setupFakeLeds()
{
    rendererThread->ledCoords.clear();

    // Simülasyon Widget'ına göndermek için geçici vektör
    QVector<QVector3D> displayLeds;

    // --- 1. PRIZMA AYARLARI (18 x 18 x 30) ---
    int dimX = 40;
    int dimY = 40;
    int dimZ = 30; // Dikey yükseklik

    float spacing = 2.5f;

    // Merkezi (0,0,0) yapmak için ofset hesaplaması
    float offsetX = (dimX * spacing) / 2.0f;
    float offsetY = (dimY * spacing) / 2.0f;
    float offsetZ = (dimZ * spacing) / 2.0f;

    for(int x = 0; x < dimX; ++x) {
        for(int y = 0; y < dimY; ++y) {
            for(int z = 0; z < dimZ; ++z) {

                // A. 3D FİZİKSEL KOORDİNAT
                float physX = x * spacing - offsetX;
                float physY = y * spacing - offsetY;
                float physZ = z * spacing - offsetZ;

                // Y eksenini yükseklik olarak kullanıyoruz (Görsel tercih)
                QVector3D p(physX, physZ, physY);

                // Animasyon thread'i için (Veri)
                rendererThread->ledCoords.append(p);

                // Simülasyon ekranı için (Görsel)
                displayLeds.append(p);
            }
        }
    }

    rendererThread->ledColors.resize(rendererThread->ledCoords.size());
    rendererThread->ledColors.fill(Qt::black);

    // Verileri widget'a gönder
    simWidget->setLedData(displayLeds);
}

void MainWindow::onToggleSimulation()
{
    isSimulationActive = btnToggleSim->isChecked();
    if (isSimulationActive) {
        btnToggleSim->setText("Show Preview");
        viewStack->setCurrentWidget(simWidget);
    } else {
        btnToggleSim->setText("Show Simulation");
        viewStack->setCurrentWidget(displayLabel);
    }
}


void MainWindow::updateDisplay(const QImage &img, const QVector<QColor> &colors)
{
    // Hangi mod açıksa ona veri gönder
    if (isSimulationActive) {
        simWidget->updateLedColors(colors);
    } else {
        displayLabel->setPixmap(QPixmap::fromImage(img));
    }
}

void MainWindow::onAnimationChanged(int index)
{
    Core::AnimationType type = static_cast<Core::AnimationType>(animSelector->itemData(index).toInt());
    rendererThread->currentType = type;
    loadSettingsWidget(type);
}

void MainWindow::loadSettingsWidget(Core::AnimationType type)
{
    if(currentSettingsWidget) {
        settingsLayout->removeWidget(currentSettingsWidget);
        currentSettingsWidget->deleteLater();
        currentSettingsWidget = nullptr;
    }

    if (type == Core::AnimationType::StaticCube) {
        currentSettingsWidget = new StaticCubeSettingsWindow(&m_params.staticCubeSettings, &m_dynParams.staticCubeDynamicParams);
    }
    else if (type == Core::AnimationType::HyperCube) {
        auto w = new HyperCubeSettingsWindow(&m_params.hyperCubeSettings, "Hyper Cube");
        w->setWindowFlags(Qt::Widget);
        currentSettingsWidget = w;
    }
    else if (type == Core::AnimationType::Text3D) {
        auto w = new Text3DSettingsWindow(&m_params.text3DSettings, "Text 3D Options");
        w->setWindowFlags(Qt::Widget);
        currentSettingsWidget = w;
    }
    else if (type == Core::AnimationType::SolidText3D) {
        auto w = new SolidText3DSettingsWindow(&m_params.solidText3DSettings, &m_dynParams.solidText3DDynamicParams, "Solid Text 3D Options");
        w->setWindowFlags(Qt::Widget);
        currentSettingsWidget = w;
    }
    else if (type == Core::AnimationType::LinearWave) {
        auto w = new LinearWaveSettingsWindow(&m_params.linearWaveSettings, "Linear Wave Options");
        w->setWindowFlags(Qt::Widget);
        currentSettingsWidget = w;
    }
    else if (type == Core::AnimationType::WaveFountain3D) {
        auto w = new WaveFountain3DSettingsWindow(&m_params.waveFountainSettings, "Wave Fountain 3D Options");
        w->setWindowFlags(Qt::Widget);
        currentSettingsWidget = w;
    }
    else if (type == Core::AnimationType::PendulumWave) {
        auto w = new PendulumWaveSettingsWindow(&m_params.pendulumWaveSettings, &m_dynParams.pendulumWaveDynamicParams);
        w->setWindowFlags(Qt::Widget);
        currentSettingsWidget = w;
    }
    else if (type == Core::AnimationType::KineticRain) {
        auto w = new KineticRainSettingsWindow(&m_params.kineticRainSettings, &m_dynParams.kineticRainDynamicParams);
        w->setWindowFlags(Qt::Widget);
        currentSettingsWidget = w;
    }
    if (currentSettingsWidget) {
        settingsLayout->addWidget(currentSettingsWidget);
        currentSettingsWidget->show();
    }
}


void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (rendererThread && displayLabel) {
        rendererThread->setRenderSize(displayLabel->size());
    }
}
