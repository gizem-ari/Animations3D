#ifndef ANIMATIONALGORITHMS_H
#define ANIMATIONALGORITHMS_H
#include "core.h"
#include "qpixmap.h"
#include <QImage>
#include <QVector>
#include <QVector3D>
#include <QColor>

class AnimationAlgorithms
{
public:
    AnimationAlgorithms();

    static QImage CreateStaticCube(QSize size,Core::StaticCubeSettings* settings,Core::StaticCubeDynamicParams* dynParams,const QVector<QVector3D>& ledCoords,QVector<QColor>& ledColors,Core::ReactionSettings* reactionSettings,double low, double mid, double high);
    static QImage CreateHyperCube(QSize size, Core::HyperCubeSettings* settings,Core::HyperCubeDynamicParams* dynParams,const QVector<QVector3D>& ledCoords,QVector<QColor>& ledColors,Core::ReactionSettings* reactionSettings,double low, double mid, double high);
    static QImage CreateSolidText3D(QSize size, Core::SolidText3DSettings* settings, Core::SolidText3DDynamicParams* dynParams, const QVector<QVector3D>& ledCoords, QVector<QColor>& ledColors, Core::ReactionSettings* reactionSettings, double low, double mid, double high);
    static QImage CreateLinearWave(QSize size, Core::LinearWaveSettings* settings, Core::LinearWaveDynamicParams* dynParams, const QVector<QVector3D>& ledCoords, QVector<QColor>& ledColors, Core::ReactionSettings* reactionSettings, double low, double mid, double high);
    static QImage CreateWaveFountain3D(QSize size, Core::WaveFountain3DSettings* settings, Core::WaveFountain3DDynamicParams* dynParams, const QVector<QVector3D>& ledCoords, QVector<QColor>& ledColors, Core::ReactionSettings* reactionSettings, double low, double mid, double high);
    static QImage CreatePendulumWave(QSize size, Core::PendulumWaveSettings* settings, Core::PendulumWaveDynamicParams* dynParams, const QVector<QVector3D>& ledCoords, QVector<QColor>& ledColors, Core::ReactionSettings* reactionSettings, double low, double mid, double high);
    static QImage CreateKineticRain(QSize size, Core::KineticRainSettings* settings, Core::KineticRainDynamicParams* dynParams, const QVector<QVector3D>& ledCoords, QVector<QColor>& ledColors, Core::ReactionSettings* reactionSettings, double low, double mid, double high);
    //2d ekranda
    static QImage CreateText3DFrame(QSize size, Core::Text3DSettings* settings, Core::Text3DDynamicParams* dynParams, double low, double mid, double high);


};

#endif // ANIMATIONALGORITHMS_H
