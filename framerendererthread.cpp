#include "framerendererthread.h"
#include "animationalgorithms.h"
#include "qpainter.h"
#include <cstdlib>

FrameRendererThread::FrameRendererThread(QObject *parent)
    : QThread(parent)
    , m_running(true)
    , m_size(800, 600)
{
    ledColors.resize(1000);
}

FrameRendererThread::~FrameRendererThread()
{
    stop();
    wait();
}

void FrameRendererThread::stop()
{
    QMutexLocker locker(&m_mutex);
    m_running = false;
}

void FrameRendererThread::run()
{
    while (true)
    {
        {
            QMutexLocker locker(&m_mutex);
            if (!m_running) break;
        }

        QElapsedTimer timer;
        timer.start();

        low = 0.01;
        mid = 0.01;
        high = 0.01;

        if (ledColors.size() != ledCoords.size()) ledColors.resize(ledCoords.size());
        QImage frame;

        switch (currentType) {
        case Core::AnimationType::StaticCube:
            if (staticSettings && staticParams) {
                frame = AnimationAlgorithms::CreateStaticCube(m_size, staticSettings, staticParams, ledCoords, ledColors, &reaction, low, mid, high);
            }
            break;
        case Core::AnimationType::HyperCube:
            if (hyperSettings && hyperParams) {
                frame = AnimationAlgorithms::CreateHyperCube(m_size, hyperSettings, hyperParams, ledCoords, ledColors, &reaction, low, mid, high);
            }
            break;
        case Core::AnimationType::Text3D:
            if (text3DSettings && text3DParams) {
                int oldFontSize = text3DSettings->fontSize;

                frame = AnimationAlgorithms::CreateText3DFrame(m_size, text3DSettings, text3DParams, low, mid, high);

                if (text3DSettings->fontSize != oldFontSize) {
                    emit fontSizeAutoChanged(text3DSettings->fontSize);
                }

                ledColors.fill(text3DSettings->backgroundColor);
            }
            break;
        case Core::AnimationType::SolidText3D:
            if (solidText3DSettings && solidText3DParams) {
                frame = AnimationAlgorithms::CreateSolidText3D(m_size, solidText3DSettings, solidText3DParams, ledCoords, ledColors, &reaction, low, mid, high);
            }
            break;
        case Core::AnimationType::LinearWave:
            if(linearWaveSettings && linearWaveParams){
                frame = AnimationAlgorithms::CreateLinearWave(m_size, linearWaveSettings, linearWaveParams, ledCoords, ledColors, &reaction, low, mid, high);
            }
            break;
        case Core::AnimationType::WaveFountain3D:
            if(waveFountainSettings && waveFountainParams){
                frame = AnimationAlgorithms::CreateWaveFountain3D(m_size, waveFountainSettings, waveFountainParams, ledCoords, ledColors, &reaction, low, mid, high);
            }
            break;
        case Core::AnimationType::PendulumWave:
            if (pendulumSettings && pendulumParams) {
                frame = AnimationAlgorithms::CreatePendulumWave(m_size, pendulumSettings, pendulumParams, ledCoords, ledColors, &reaction,  low, mid, high);
            }
            break;
        case Core::AnimationType::KineticRain:
            if (kineticRainSettings && kineticRainParams) {
                frame = AnimationAlgorithms::CreateKineticRain(m_size, kineticRainSettings, kineticRainParams, ledCoords, ledColors, &reaction, low, mid, high);
            }
            break;
        default:
            frame = QImage(m_size, QImage::Format_ARGB32_Premultiplied);
            frame.fill(Qt::black);
        }

        if (!frame.isNull()) {
            emit frameReady(frame, ledColors);
        }

        int elapsed = timer.elapsed();
        if (elapsed < 33) msleep(33 - elapsed);
    }
}

void FrameRendererThread::setRenderSize(const QSize &size)
{
    QMutexLocker locker(&m_mutex);
    m_size = size;
}
