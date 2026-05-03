#ifndef CORE_H
#define CORE_H

#include "qcolor.h"
#include "qpainterpath.h"
#include "qpolygon.h"
#include <QVector>
#include <QVector3D>

class Core
{
public:
    Core();
    enum AnimationType {
        StaticCube,
        HyperCube,
        Text3D,
        SolidText3D,
        LinearWave,
        WaveFountain3D,
        PendulumWave,
        KineticRain
    };

    const QStringList animationNames = {
        "StaticCube",
        "HyperCube",
        "Text3D",
        "SolidText3D",
        "LinearWave",
        "WaveFountain3D",
        "PendulumWave",
        "KineticRain"
    };

    enum _3D_AxisDirection {
        _3D_X_Plus,
        _3D_X_Minus,
        _3D_Y_Plus,
        _3D_Y_Minus,
        _3D_Z_Plus,
        _3D_Z_Minus
    };

    struct StaticCubeSettings {
        QColor xColor = Qt::red;
        QColor yColor = Qt::green;
        QColor zColor = Qt::blue;
        QColor backgroundColor = Qt::black;
        double size = 1.0;
        double thickness = 0.1;
        bool wireframeOnly = false;
        _3D_AxisDirection direction = _3D_Z_Plus;
    };

    struct StaticCubeDynamicParams {
        double phase = 0.0;
        double rotation = 0.0;
    };

    struct HyperCubeSettings {
        QColor edgeColor = QColor(0, 0, 255);
        QColor faceColor = QColor(0, 0, 255, 40);
        QColor backgroundColor = Qt::black;
        double size = 1.5;
        double thickness = 0.1;
        double speedFactor = 1.0;
        bool wireframeOnly = false;
        _3D_AxisDirection direction = _3D_Z_Plus;
    };

    struct HyperCubeDynamicParams {
        double continuousRotation = 0.0; //sürekli artan ana açı
        double wobblePhase = 0.0; //sinüs fazı - salınım
    };

    enum TextScrollDirection {
        ScrollLeft,
        ScrollRight,
        ScrollUp,
        ScrollDown,
        ScrollDiagTL_BR,
        ScrollDiagTR_BL,
        ScrollDiagBL_TR,
        ScrollDiagBR_TL,
        ScrollFlyBy,
        ScrollAbyss,
        ScrollStarWars,
        ScrollOrbit,
        ScrollTornado,
        ScrollZBounce,
        ScrollSineWave,
        ScrollBouncingDVD,
        ScrollPendulum,
    };
    enum TextRenderMode {
        Wireframe,
        Layered,
        Solid
    };
    enum TextOscillationStyle {
        OscillationNone,
        OscillationGentle,
        OscillationDynamic,
        OscillationChaotic
    };
    struct Text3DSettings {
        QString text = "Zenopix";
        QString fontFamily = "Arial";
        int fontWeight = 75;
        int fontSize = 60;
        double scale = 1.0;
        double depth = 40.0;
        TextScrollDirection scrollDirection = ScrollLeft;
        int speedFactor = 5;
        bool autoColorFlow = true;
        bool rainbowMode = false;
        bool fitOnScreen = false;
        double colorChangeInterval = 3.0;
        QColor textColor = QColor(60, 220, 255);
        QColor backgroundColor = Qt::black;
        TextRenderMode renderMode = Solid;
        TextOscillationStyle oscillationStyle = OscillationNone;
        double oscillationAmount = 1.0;
    };

    struct Text3DDynamicParams {
        double time = 0.0;
        double scrollOffset = 0.0;
        double currentHue = 0.0;
        double targetHue = 0.0;
        double lastColorChangeTime = 0.0;
        QString lastText = "";
        QString lastFontFamily = "";
        int lastFontWeight = 0;
        int lastFontSize = 0;
        double textBoundingWidth = 0.0;
        double textBoundingHeight = 0.0;
        QVector<std::pair<QPointF, QPointF>> cached2DLines;
        QList<QPolygonF> cachedPolys;
    };
    struct SolidText3DSettings {
        QString text = "ZENOPİX 3D";
        QString fontFamily = "Arial";
        int fontSize = 40;
        bool autoScale = false;
        double thickness = 0.2;
        QColor colorFront = QColor(0, 255, 255);
        QColor colorBack = QColor(0, 255, 255);
        QColor backgroundColor = Qt::black;
        _3D_AxisDirection direction = _3D_Z_Plus;
        double speedFactor = 0.0;
        bool showGizmo = true;
        bool resetOnDirectionChange = true;
    };

    struct SolidText3DDynamicParams {
        double phase = 0.0;
        double rotationX = 0.0;
        double rotationY = 0.0;
        double rotationZ = 0.0;
        QString lastText = "";
        QString lastFontFamily = "";
        int lastFontSize = 0;
        bool lastAutoScale = false;
        _3D_AxisDirection lastDirection = _3D_Z_Plus;
        int calculatedFontSize = 40;
        double lastThickness = 0.0;
        QPainterPath cachedPath;
        QList<QPolygonF> cachedPolys;
    };

    struct LinearWaveSettings {
        QColor waveColor = QColor(0, 200, 255);
        QColor backgroundColor = Qt::black;
        double amplitude = 0.8;
        double waveNumber = 3.0;
        double speedFactor = 1.0;
        _3D_AxisDirection direction = _3D_X_Plus;
        int activeGridSize = 18;
    };

    struct LinearWaveDynamicParams {
        double time = 0.0;
    };

    struct WaveFountain3DSettings {
        QColor waterColor = QColor(0, 255, 150);
        QColor backgroundColor = Qt::black;
        double amplitude = 0.8;
        double waveNumber = 4.0;
        double speedFactor = 1.0;
        _3D_AxisDirection direction = _3D_Y_Plus;
        int activeGridSize = 18;
    };

    struct WaveFountain3DDynamicParams {
        double time = 0.0;
    };

    struct PendulumWaveSettings {
        QColor color = QColor(0, 255, 200);
        QColor backgroundColor = Qt::black;
        double baseFrequency = 0.5;
        double frequencyStep = 0.05;
        double amplitude = 0.5;
        double sphereSize = 0.15;
        double speedFactor = 1.0;
        Core::_3D_AxisDirection direction = Core::_3D_Z_Plus;
        bool rainbowMode = false;
        bool flatMode = true;
    };


    struct PendulumWaveDynamicParams {
        double internalTime = 0.0;
    };
    struct KineticRainSettings {
        QColor dropColor = QColor(200, 220, 255);
        QColor backgroundColor = Qt::black;
        double dropSize = 0.15;
        double waveFrequency = 1.0;
        double amplitude = 0.5;
        double speedFactor = 1.0;
        Core::_3D_AxisDirection direction = Core::_3D_Z_Plus;
        bool rippleMode = true;
        bool rainbowMode = false;
    };

    struct KineticRainDynamicParams {
        double internalTime = 0.0;
    };

    struct AnimationParams
    {
        StaticCubeSettings staticCubeSettings;
        HyperCubeSettings hyperCubeSettings;
        Text3DSettings text3DSettings;
        SolidText3DSettings solidText3DSettings;
        LinearWaveSettings linearWaveSettings;
        WaveFountain3DSettings waveFountainSettings;
        PendulumWaveSettings pendulumWaveSettings;
        KineticRainSettings kineticRainSettings;
    };

    struct AnimationDynamicParams
    {
        StaticCubeDynamicParams staticCubeDynamicParams;
        HyperCubeDynamicParams hyperCubeDynamicParams;
        Text3DDynamicParams text3DDynamicParams;
        SolidText3DDynamicParams solidText3DDynamicParams;
        LinearWaveDynamicParams linearWaveDynamicParams;
        WaveFountain3DDynamicParams waveFountainDynamicParams;
        PendulumWaveDynamicParams pendulumWaveDynamicParams;
        KineticRainDynamicParams kineticRainDynamicParams;
    };


    struct ReactionSettings {
        double bassGain = 1.0;
        double midGain = 1.0;
        double trebleGain = 1.0;
    };

    QPair<ReactionSettings,ReactionSettings> reactionSettings;

    static QVector3D rotatePoint(const QVector3D &pt, double ax, double ay, double az);

};

#endif // CORE_H
