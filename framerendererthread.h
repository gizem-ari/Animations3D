#ifndef FRAMERENDERERTHREAD_H
#define FRAMERENDERERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QImage>
#include <QVector>
#include <QColor>
#include <QElapsedTimer>
#include "core.h"

class FrameRendererThread : public QThread
{
    Q_OBJECT
public:
    explicit FrameRendererThread(QObject *parent = nullptr);
    ~FrameRendererThread();

    void stop();
    void setRenderSize(const QSize &size);
    QVector<QVector3D> ledCoords;
    QVector<QColor> ledColors;

    Core::AnimationType currentType = Core::AnimationType::StaticCube;

    Core::StaticCubeSettings* staticSettings = nullptr;
    Core::StaticCubeDynamicParams* staticParams = nullptr;
    Core::HyperCubeSettings* hyperSettings = nullptr;
    Core::HyperCubeDynamicParams* hyperParams = nullptr;

    Core::Text3DSettings* text3DSettings = nullptr;
    Core::Text3DDynamicParams* text3DParams = nullptr;

    Core::SolidText3DSettings* solidText3DSettings = nullptr;
    Core::SolidText3DDynamicParams* solidText3DParams = nullptr;

    Core::LinearWaveSettings* linearWaveSettings = nullptr;
    Core::LinearWaveDynamicParams* linearWaveParams = nullptr;

    Core::WaveFountain3DSettings* waveFountainSettings;
    Core::WaveFountain3DDynamicParams* waveFountainParams;

    Core::PendulumWaveSettings* pendulumSettings = nullptr;
    Core::PendulumWaveDynamicParams* pendulumParams = nullptr;

    Core::KineticRainSettings* kineticRainSettings = nullptr;
    Core::KineticRainDynamicParams* kineticRainParams = nullptr;

    Core::ReactionSettings reaction;

signals:
    void frameReady(const QImage& previewImage, const QVector<QColor>& colors);
    void fontSizeAutoChanged(int newSize);
protected:
    void run() override;

private:
    bool m_running;
    QMutex m_mutex;
    QSize m_size;

    double low = 0.0;
    double mid = 0.0;
    double high = 0.0;
};

#endif // FRAMERENDERERTHREAD_H
