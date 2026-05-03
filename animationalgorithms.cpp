#include "animationalgorithms.h"
#include <cmath>
#include <algorithm>
#include <QtMath>
#include <QPainter>
#include <limits>
#include <QPainterPath>
#include <QFont>
#include <QFontMetrics>
#include <cmath>
#include <QDateTime>
#include <QMatrix4x4>

static QVector3D localRotateX(const QVector3D& p, double theta) {
    double c = std::cos(theta);
    double s = std::sin(theta);
    return QVector3D(p.x(), p.y() * c - p.z() * s, p.y() * s + p.z() * c);
}

static QVector3D localRotateY(const QVector3D& p, double theta) {
    double c = std::cos(theta);
    double s = std::sin(theta);
    return QVector3D(p.x() * c + p.z() * s, p.y(), -p.x() * s + p.z() * c);
}

static QVector3D localRotateZ(const QVector3D& p, double theta) {
    double c = std::cos(theta);
    double s = std::sin(theta);
    return QVector3D(p.x() * c - p.y() * s, p.x() * s + p.y() * c, p.z());
}

static QPointF projectIsometric(const QVector3D& p, const QPointF& center, double scale) {
    double rotX = p.x() * 0.707 + p.y() * -0.707;
    double rotY = p.x() * 0.408 + p.y() * 0.408 + p.z() * -0.816;
    return QPointF(center.x() + rotX * scale, center.y() + rotY * scale);
}


static void DrawReferenceGizmo(QPainter* painter, const QSize& size, double rotX, double rotY, double rotZ)
{
    double len = 40.0;
    QPointF center(size.width() - 60, size.height() - 60);

    QVector<QVector3D> axes = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    };

    QVector<QColor> colors = {Qt::red, Qt::green, Qt::blue};
    QStringList labels = {"X", "Y", "Z"};

    struct AxisDraw {
        QPointF end;
        QColor color;
        QString label;
        double zVal;
    };

    QVector<AxisDraw> drawList;

    for(int i=0; i<3; ++i) {
        QVector3D p = axes[i] * len;

        p = localRotateZ(p, rotZ);
        p = localRotateY(p, rotY);
        p = localRotateX(p, rotX);

        QPointF endPt(center.x() + p.x(), center.y() + p.y());
        drawList.append({endPt, colors[i], labels[i], p.z()});
    }

    std::sort(drawList.begin(), drawList.end(), [](const AxisDraw& a, const AxisDraw& b){
        return a.zVal < b.zVal;
    });

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen;
    pen.setWidth(3);
    pen.setCapStyle(Qt::RoundCap);

    for(const auto& axis : drawList) {
        pen.setColor(axis.color);
        painter->setPen(pen);
        painter->drawLine(center, axis.end);

        painter->setPen(Qt::white);
        painter->drawText(axis.end + QPointF(5, 5), axis.label);
    }

    painter->restore();
}


QImage AnimationAlgorithms::CreateStaticCube(QSize size, Core::StaticCubeSettings* settings, Core::StaticCubeDynamicParams* dynParams, const QVector<QVector3D>& ledCoords, QVector<QColor>& ledColors, Core::ReactionSettings* reactionSettings, double low, double mid, double high)
{
    if (ledColors.size() != ledCoords.size()) ledColors.resize(ledCoords.size());
    std::fill(ledColors.begin(), ledColors.end(), settings->backgroundColor);

    float minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9, minZ = 1e9, maxZ = -1e9;
    if (!ledCoords.isEmpty()) {
        for (const QVector3D& p : ledCoords) {
            minX = std::min(minX, p.x()); maxX = std::max(maxX, p.x());
            minY = std::min(minY, p.y()); maxY = std::max(maxY, p.y());
            minZ = std::min(minZ, p.z()); maxZ = std::max(maxZ, p.z());
        }
    } else {
        return QImage(size, QImage::Format_ARGB32_Premultiplied);
    }

    QVector3D center((minX+maxX)*0.5, (minY+maxY)*0.5, (minZ+maxZ)*0.5);
    double maxDim = std::max({maxX-minX, maxY-minY, maxZ-minZ});
    if(maxDim < 1.0) maxDim = 1.0;

    // --- OTOMATİK ROTASYON MANTIĞI ---
    // ideal "İzometrik" açılar
    double autoRotX = 0;
    double autoRotY = 0;
    double autoRotZ = 0;

    switch (settings->direction) {
    case Core::_3D_Z_Plus: // Mavi Ön Yüz (Varsayılan İzometrik)
        autoRotX = 35.0; autoRotY = 45.0; autoRotZ = 0.0;
        break;
    case Core::_3D_Z_Minus: // Mavi Arka Yüz
        autoRotX = 35.0; autoRotY = 225.0; autoRotZ = 0.0;
        break;
    case Core::_3D_X_Plus: // Kırmızı Sağ Yüz
        autoRotX = 35.0; autoRotY = -45.0; autoRotZ = 0.0;
        break;
    case Core::_3D_X_Minus: // Kırmızı Sol Yüz
        autoRotX = 35.0; autoRotY = 135.0; autoRotZ = 0.0;
        break;
    case Core::_3D_Y_Plus: // Yeşil Üst Yüz (Kuşbakışı)
        autoRotX = 90.0; autoRotY = 0.0; autoRotZ = 45.0;
        break;
    case Core::_3D_Y_Minus: // Yeşil Alt Yüz
        autoRotX = -90.0; autoRotY = 0.0; autoRotZ = 45.0;
        break;
    }

    double radX = qDegreesToRadians(autoRotX);
    double radY = qDegreesToRadians(autoRotY);
    double radZ = qDegreesToRadians(autoRotZ);

    double targetSize = settings->size * maxDim * 0.4;
    double thickness = settings->thickness * maxDim * 0.1;

#pragma omp parallel for
    for (int i = 0; i < ledCoords.size(); ++i) {
        QVector3D p = ledCoords[i] - center;

        // Ters rotasyon (Mekanik hizalama)
        p = localRotateX(p, -radX);
        p = localRotateY(p, -radY);
        p = localRotateZ(p, -radZ);

        double absX = std::abs(p.x());
        double absY = std::abs(p.y());
        double absZ = std::abs(p.z());

        double maxComp = std::max({absX, absY, absZ});
        double distToSurface = std::abs(maxComp - targetSize);

        bool isEdge = distToSurface < thickness;
        bool isInside = maxComp < targetSize;

        if (isEdge || (isInside && !settings->wireframeOnly)) {
            QColor finalColor;
            if (maxComp == absX) finalColor = settings->xColor;
            else if (maxComp == absY) finalColor = settings->yColor;
            else finalColor = settings->zColor;

            if (isInside && !isEdge) finalColor.setAlpha(80);
            ledColors[i] = finalColor;
        }
    }

    // --- EKRAN ÇİZİMİ (RENDER) ---
    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::black);
    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);

    QPointF center2D(size.width()/2.0, size.height()/2.0);
    double previewScale = size.height() / (maxDim * 1.5);

    QVector<QVector3D> corners = {
        {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
        {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1}
    };

    QVector<QVector3D> rotCorners3D(8);
    QVector<QPointF> screenPts(8);

    for(int i=0; i<8; ++i) {
        QVector3D p = corners[i] * targetSize;
        p = localRotateZ(p, radZ);
        p = localRotateY(p, radY);
        p = localRotateX(p, radX);

        rotCorners3D[i] = p;
        screenPts[i] = projectIsometric(p, center2D, previewScale);
    }

    if (!settings->wireframeOnly) {
        struct Face { int idx[4]; QColor color; double zDepth; };
        QVector<Face> faces = {
            {{0,1,2,3}, settings->zColor, 0},
            {{4,7,6,5}, settings->zColor, 0},
            {{0,3,7,4}, settings->xColor, 0},
            {{1,5,6,2}, settings->xColor, 0},
            {{3,2,6,7}, settings->yColor, 0},
            {{0,4,5,1}, settings->yColor, 0}
        };

        for(auto &f : faces) {
            f.zDepth = 0;
            for(int i=0; i<4; ++i) f.zDepth += rotCorners3D[f.idx[i]].z();
            f.zDepth /= 4.0;
            f.color.setAlpha(100);
        }

        std::sort(faces.begin(), faces.end(), [](const Face& a, const Face& b){
            return a.zDepth < b.zDepth;
        });

        painter.setPen(Qt::NoPen);
        for(const auto &f : faces) {
            QPointF polyPts[4];
            for(int i=0; i<4; ++i) polyPts[i] = screenPts[f.idx[i]];
            painter.setBrush(f.color);
            painter.drawPolygon(polyPts, 4);
        }
    }

    struct EdgeDef { int start; int end; int axis; };
    QVector<EdgeDef> definedEdges = {
        {0,4, 2}, {1,5, 2}, {2,6, 2}, {3,7, 2},
        {0,3, 1}, {1,2, 1}, {4,7, 1}, {5,6, 1},
        {0,1, 0}, {3,2, 0}, {4,5, 0}, {7,6, 0}
    };

    for(const auto& edge : definedEdges) {
        QColor edgeCol;
        if      (edge.axis == 0) edgeCol = settings->xColor;
        else if (edge.axis == 1) edgeCol = settings->yColor;
        else                     edgeCol = settings->zColor;

        double zDepth = (rotCorners3D[edge.start].z() + rotCorners3D[edge.end].z()) * 0.5;
        int alpha = std::clamp(static_cast<int>(150 + zDepth * 100), 50, 255);
        edgeCol.setAlpha(alpha);

        QPen pen(edgeCol);
        pen.setWidthF(2.0);
        painter.setPen(pen);
        painter.drawLine(screenPts[edge.start], screenPts[edge.end]);
    }
    DrawReferenceGizmo(&painter, size, radX, radY, radZ);

    return img;
}


QImage AnimationAlgorithms::CreateHyperCube(QSize size, Core::HyperCubeSettings* settings, Core::HyperCubeDynamicParams* dynParams, const QVector<QVector3D>& ledCoords, QVector<QColor>& ledColors, Core::ReactionSettings* reactionSettings, double low, double mid, double high)
{
    // === A) REAKSİYON VE BUFFER HAZIRLIĞI ===
    const Core::ReactionSettings defaultReaction;
    const Core::ReactionSettings* rs = reactionSettings ? reactionSettings : &defaultReaction;

    if (ledColors.size() != ledCoords.size()) {
        ledColors.resize(ledCoords.size());
    }
    std::fill(ledColors.begin(), ledColors.end(), settings->backgroundColor);

    // === B) BOUNDING BOX (İSTENİLEN STANDART GERİ GELDİ) ===
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    float maxZ = std::numeric_limits<float>::lowest();

    bool hasLeds = !ledCoords.isEmpty();
    if (hasLeds) {
        for (const QVector3D& p : ledCoords) {
            if (p.x() < minX) minX = p.x();
            if (p.y() < minY) minY = p.y();
            if (p.z() < minZ) minZ = p.z();
            if (p.x() > maxX) maxX = p.x();
            if (p.y() > maxY) maxY = p.y();
            if (p.z() > maxZ) maxZ = p.z();
        }
    } else {
        minX = minY = minZ = 0;
        maxX = maxY = maxZ = 100;
    }

    // === C) SİSTEM BOYUTLARI VE MERKEZ ===
    double sysWidth = std::max(1.0, double(maxX - minX));
    double sysDepth = std::max(1.0, double(maxY - minY));
    double sysHeight = std::max(1.0, double(maxZ - minZ));
    QVector3D center((minX + maxX) * 0.5, (minY + maxY) * 0.5, (minZ + maxZ) * 0.5);

    // === D) YÖN EKSENİ VE AKTİF BOYUT  ===
    double activeDimension = std::min({sysWidth, sysDepth, sysHeight});

    // === E) AUDIO MODÜLASYONLARI VE ÖLÇEKLEME ===
    double speedMod = 1.0 + (mid * rs->midGain * 0.8);
    double sizeMod = 1.0 + (low * rs->bassGain * 0.4);
    double edgeBrightMod = high * rs->trebleGain;

    double audioEnergy = (low + mid) * 0.5;
    double wobbleMod = (audioEnergy > 0.05 ? audioEnergy : 0.0) * rs->midGain * 0.8;

    double baseScale = activeDimension * 0.25; // dönüş payını içerir
    double targetSize = settings->size * baseScale * sizeMod;
    double thickness = settings->thickness * activeDimension * 0.1;

    // === F) DİNAMİK DURUM GÜNCELLEMESİ (HAREKET MANTIĞI) ===
    double step = 0.01 * settings->speedFactor * speedMod;
    dynParams->continuousRotation += step;
    dynParams->wobblePhase += step * 0.7;

    if(dynParams->continuousRotation > 6.283) dynParams->continuousRotation -= 6.283;
    if(dynParams->wobblePhase > 6.283) dynParams->wobblePhase -= 6.283;

    double angleX = 0, angleY = 0, angleZ = 0;

    switch (settings->direction) {
    case Core::_3D_Z_Plus: case Core::_3D_Z_Minus:
        angleZ = dynParams->continuousRotation;
        angleX = std::sin(dynParams->wobblePhase) * wobbleMod;
        angleY = std::cos(dynParams->wobblePhase) * wobbleMod;
        break;
    case Core::_3D_X_Plus: case Core::_3D_X_Minus:
        angleX = dynParams->continuousRotation;
        angleY = std::sin(dynParams->wobblePhase) * wobbleMod;
        angleZ = std::cos(dynParams->wobblePhase) * wobbleMod;
        break;
    case Core::_3D_Y_Plus: case Core::_3D_Y_Minus:
        angleY = dynParams->continuousRotation;
        angleX = std::sin(dynParams->wobblePhase) * wobbleMod;
        angleZ = std::cos(dynParams->wobblePhase) * wobbleMod;
        break;
    }

// === 5. VOLUMETRİK HESAPLAMA (FİZİKSEL LED'LER) ===
#pragma omp parallel for
    for (int i = 0; i < ledCoords.size(); ++i) {
        QVector3D p = ledCoords[i] - center;

        p = localRotateX(p, -angleX);
        p = localRotateY(p, -angleY);
        p = localRotateZ(p, -angleZ);

        double dist = std::max({std::abs(p.x()), std::abs(p.y()), std::abs(p.z())});
        bool isEdge = std::abs(dist - targetSize) < thickness;
        bool isInside = dist < targetSize;

        if (isEdge) {
            QColor c = settings->edgeColor;
            if (edgeBrightMod > 0.4) c = c.lighter(150);
            ledColors[i] = c;
        } else if (isInside && !settings->wireframeOnly) {
            ledColors[i] = settings->faceColor;
        }
    }

    // === 6. UI ÖNİZLEME ÇİZİMİ (RENDER) ===
    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(settings->backgroundColor);
    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);

    QPointF center2D(size.width()/2.0, size.height()/2.0);

    // UI çiziminin ekrana her zaman sığması için UI ölçeği sabit bir oran kullanır
    double maxSystemDim = std::max({sysWidth, sysDepth, sysHeight});
    double previewScale = size.height() / (maxSystemDim * 1.5);

    QVector<QVector3D> corners = {
        {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
        {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1}
    };

    QVector<QVector3D> rotCorners3D(8);
    QVector<QPointF> screenPts(8);

    for(int i=0; i<8; ++i) {
        QVector3D p = corners[i] * targetSize;
        p = localRotateZ(p, angleZ);
        p = localRotateY(p, angleY);
        p = localRotateX(p, angleX);

        rotCorners3D[i] = p;
        screenPts[i] = projectIsometric(p, center2D, previewScale);
    }

    QColor edgeCol = settings->edgeColor;
    if (edgeBrightMod > 0.4) edgeCol = edgeCol.lighter(150);
    QPen edgePen(edgeCol);
    edgePen.setWidthF(2.0);
    edgePen.setJoinStyle(Qt::RoundJoin);

    if (!settings->wireframeOnly) {
        // --- KATI CİSİM VE SANAL IŞIK MODU ---

        // İzometrik projeksiyon kamerasının 3D uzaydaki tam bakış açısı (Diagonal)
        QVector3D cameraDir(1.0f, 1.0f, 1.0f);
        cameraDir.normalize();

        // Işık da kamerayla aynı yönden vursun
        QVector3D lightDir = cameraDir;

        struct Face { int idx[4]; double zDepth; QColor shadedColor; };

        QVector<Face> faces = {
            {{4,5,6,7}, 0, Qt::black}, // Ön
            {{3,2,1,0}, 0, Qt::black}, // Arka
            {{5,1,2,6}, 0, Qt::black}, // Sağ
            {{0,4,7,3}, 0, Qt::black}, // Sol
            {{7,6,2,3}, 0, Qt::black}, // Alt
            {{0,1,5,4}, 0, Qt::black}  // Üst
        };

        QColor baseColor = settings->faceColor;
        QVector<Face> visibleFaces;

        for(auto &f : faces) {
            QVector3D p0 = rotCorners3D[f.idx[0]];
            QVector3D p1 = rotCorners3D[f.idx[1]];
            QVector3D p2 = rotCorners3D[f.idx[2]];
            QVector3D p3 = rotCorners3D[f.idx[3]];

            // 1. Dışarı Bakan Normali Bul
            QVector3D v1 = p1 - p0;
            QVector3D v2 = p2 - p1;
            QVector3D normal = QVector3D::crossProduct(v1, v2).normalized();

            QVector3D faceCenter = (p0 + p1 + p2 + p3) / 4.0;
            if (QVector3D::dotProduct(normal, faceCenter) < 0) {
                normal = -normal; // Normalin her zaman dışarı baktığından emin ol
            }

            // 2. KUSURSUZ 3D BACK-FACE CULLING
            // Yüzey kameraya bakmıyorsa (Normal ile Kamera Vektörü zıtsa) çöpe at
            if (QVector3D::dotProduct(normal, cameraDir) <= 0.0f) {
                continue;
            }

            // 3. IŞIK HESAPLAMASI
            float lightIntensity = QVector3D::dotProduct(normal, lightDir);
            if(lightIntensity < 0) lightIntensity = 0;

            float ambient = 0.55f;
            float diffuse = 0.45f * lightIntensity;
            float totalLight = std::min(1.0f, ambient + diffuse);

            int r = std::clamp(static_cast<int>(baseColor.red() * totalLight), 0, 255);
            int g = std::clamp(static_cast<int>(baseColor.green() * totalLight), 0, 255);
            int b = std::clamp(static_cast<int>(baseColor.blue() * totalLight), 0, 255);

            f.shadedColor = QColor(r, g, b, 255);

            // Sıralama için derinlik kaydı
            f.zDepth = faceCenter.z();
            visibleFaces.append(f);
        }

        // Z-Fighting önlemek için görünen yüzeyleri sırala
        std::sort(visibleFaces.begin(), visibleFaces.end(), [](const Face& a, const Face& b){
            return a.zDepth < b.zDepth;
        });

        // ÇİZİM KISMI
        painter.setPen(Qt::NoPen);
        for(const auto &f : visibleFaces) {
            QPointF polyPts[4];
            for(int k=0; k<4; ++k) polyPts[k] = screenPts[f.idx[k]];

            painter.setBrush(f.shadedColor);
            painter.drawPolygon(polyPts, 4);
        }
    } else {
        // --- SADECE TEL KAFES MODU ---
        int edges[12][2] = {{0,1}, {1,2}, {2,3}, {3,0}, {4,5}, {5,6}, {6,7}, {7,4}, {0,4}, {1,5}, {2,6}, {3,7}};

        painter.setPen(edgePen);
        painter.setBrush(Qt::NoBrush);

        for(auto& e : edges) {
            painter.drawLine(screenPts[e[0]], screenPts[e[1]]);
        }
    }

    DrawReferenceGizmo(&painter, size, angleX, angleY, angleZ);

    return img;
}

QVector3D Core::rotatePoint(const QVector3D &pt, double ax, double ay, double az)
{
    double cx = std::cos(ax);
    double sx = std::sin(ax);
    double x1 = pt.x();
    double y1 = pt.y() * cx - pt.z() * sx;
    double z1 = pt.y() * sx + pt.z() * cx;

    double cy = std::cos(ay);
    double sy = std::sin(ay);
    double x2 = x1 * cy + z1 * sy;
    double y2 = y1;
    double z2 = -x1 * sy + z1 * cy;

    double cz = std::cos(az);
    double sz = std::sin(az);
    double x3 = x2 * cz - y2 * sz;
    double y3 = x2 * sz + y2 * cz;
    double z3 = z2;

    return QVector3D(x3, y3, z3);
}

QImage AnimationAlgorithms::CreateText3DFrame(QSize size, Core::Text3DSettings* settings, Core::Text3DDynamicParams* dynParams, double low, double mid, double high)
{
    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(img.rect(), settings->backgroundColor);

    double currentTime = QDateTime::currentMSecsSinceEpoch() / 1000.0;
    double deltaTime = 0.033;
    dynParams->time += deltaTime;


    if (dynParams->lastText != settings->text || dynParams->lastFontFamily != settings->fontFamily || dynParams->lastFontWeight != settings->fontWeight || dynParams->lastFontSize != settings->fontSize) {
        dynParams->cached2DLines.clear();
        dynParams->cachedPolys.clear();

        QFont font(settings->fontFamily, settings->fontSize, settings->fontWeight);
        QPainterPath path;
        path.addText(0, 0, font, settings->text);

        QRectF rect = path.boundingRect();
        dynParams->textBoundingWidth = rect.width();
        dynParams->textBoundingHeight = rect.height();

        double cx = rect.center().x();
        double cy = rect.center().y();

        QList<QPolygonF> polys = path.toSubpathPolygons();
        for (const QPolygonF& poly : polys) {
            QPolygonF centeredPoly;
            for (int i = 0; i < poly.size(); ++i) {
                QPointF p1 = poly[i];
                QPointF p2 = poly[(i + 1) % poly.size()];
                QPointF cp1(p1.x() - cx, p1.y() - cy);
                QPointF cp2(p2.x() - cx, p2.y() - cy);

                centeredPoly << cp1;
                dynParams->cached2DLines.append({cp1, cp2});
            }
            dynParams->cachedPolys.append(centeredPoly);
        }

        dynParams->lastText = settings->text;
        dynParams->lastFontFamily = settings->fontFamily;
        dynParams->lastFontWeight = settings->fontWeight;
        dynParams->lastFontSize = settings->fontSize;
    }

    if (dynParams->time - dynParams->lastColorChangeTime > settings->colorChangeInterval) {
        dynParams->targetHue = fmod(rand(), 360.0);
        dynParams->lastColorChangeTime = dynParams->time;
    }

    double hueDiff = dynParams->targetHue - dynParams->currentHue;
    if (hueDiff > 180.0) hueDiff -= 360.0;
    else if (hueDiff < -180.0) hueDiff += 360.0;
    dynParams->currentHue += hueDiff * 0.05;

    if (dynParams->currentHue < 0) dynParams->currentHue += 360.0;
    if (dynParams->currentHue >= 360.0) dynParams->currentHue -= 360.0;

    double dynamicSpeed = (settings->speedFactor * 20.0) + (low * 300.0);
    dynParams->scrollOffset += dynamicSpeed * deltaTime;

    double scaleCompensation = 2.5;
    double globalScale = settings->scale * scaleCompensation * (1.0 + low * 0.2);
    double dynamicDepth = settings->depth * (1.0 + low * 2.5);
    double halfDepth = dynamicDepth / 2.0;

    double actualTextWidth = dynParams->textBoundingWidth * globalScale;
    double actualTextHeight = dynParams->textBoundingHeight * globalScale;

    if (settings->scrollDirection == Core::ScrollBouncingDVD || settings->scrollDirection == Core::ScrollPendulum) {
        double maxAllowedWidth = size.width() * 0.75;
        if (actualTextWidth > maxAllowedWidth) {
            double ratio = maxAllowedWidth / actualTextWidth;
            globalScale *= ratio;
            actualTextWidth = dynParams->textBoundingWidth * globalScale;
            actualTextHeight = dynParams->textBoundingHeight * globalScale;
        }
    }

    double paddingX = actualTextWidth * 0.2 + 50.0;
    double paddingY = actualTextHeight * 0.2 + 50.0;

    double limitX = size.width() + actualTextWidth + paddingX;
    double limitY = size.height() + actualTextHeight + paddingY;

    double rotX = 0.0;
    double rotY = 0.0;
    double rotZ = 0.0;
    double amt = settings->oscillationAmount;

    if (settings->oscillationStyle == Core::OscillationGentle) {
        rotX += std::sin(dynParams->time * 1.5) * 0.2 * amt;
        rotY += std::cos(dynParams->time * 1.2) * 0.3 * amt;
        rotZ += std::sin(dynParams->time * 0.8) * 0.1 * amt;
    } else if (settings->oscillationStyle == Core::OscillationDynamic) {
        rotX += std::sin(dynParams->time * 4.5) * 0.35 * amt;
        rotY += std::cos(dynParams->time * 3.8) * 0.45 * amt;
        rotZ += std::sin(dynParams->time * 3.0) * 0.20 * amt;
    } else if (settings->oscillationStyle == Core::OscillationChaotic) {
        rotX += std::sin(dynParams->time * 12.0) * 0.6 * amt;
        rotY += std::cos(dynParams->time * 15.0) * 0.8 * amt;
        rotZ += std::sin(dynParams->time * 10.0) * 0.4 * amt;
    }

    double screenOffsetX = 0.0;
    double screenOffsetY = 0.0;
    double worldOffsetX = 0.0;
    double worldOffsetY = 0.0;
    double worldOffsetZ = 0.0;

    double camDist = 600.0;
    double depthAngleX = -0.35;
    double depthAngleY = -0.35;

    double safeW = std::max(1.0, (size.width() - (actualTextWidth + (dynamicDepth * std::abs(depthAngleX) * globalScale))) / 2.0);
    double safeH = std::max(1.0, (size.height() - (actualTextHeight + (dynamicDepth * std::abs(depthAngleY) * globalScale))) / 2.0);

    auto getPingPong = [](double time, double maxVal) -> double {
        return std::abs(fmod(time, maxVal * 4.0) - maxVal * 2.0) - maxVal;
    };

    double loopMax = std::max(limitX, limitY);

    switch (settings->scrollDirection) {
    case Core::ScrollLeft:
        if (settings->fitOnScreen) screenOffsetX = getPingPong(dynParams->scrollOffset * 0.8, safeW);
        else screenOffsetX = (limitX / 2.0) - fmod(dynParams->scrollOffset, limitX);
        break;
    case Core::ScrollRight:
        if (settings->fitOnScreen) screenOffsetX = -getPingPong(dynParams->scrollOffset * 0.8, safeW);
        else screenOffsetX = -(limitX / 2.0) + fmod(dynParams->scrollOffset, limitX);
        break;
    case Core::ScrollUp:
        if (settings->fitOnScreen) screenOffsetY = getPingPong(dynParams->scrollOffset * 0.8, safeH);
        else screenOffsetY = (limitY / 2.0) - fmod(dynParams->scrollOffset, limitY);
        break;
    case Core::ScrollDown:
        if (settings->fitOnScreen) screenOffsetY = -getPingPong(dynParams->scrollOffset * 0.8, safeH);
        else screenOffsetY = -(limitY / 2.0) + fmod(dynParams->scrollOffset, limitY);
        break;
    case Core::ScrollDiagTL_BR:
    case Core::ScrollDiagTR_BL:
    case Core::ScrollDiagBL_TR:
    case Core::ScrollDiagBR_TL:
        if (settings->fitOnScreen) {
            screenOffsetX = getPingPong(dynParams->scrollOffset * 0.8, safeW);
            screenOffsetY = getPingPong(dynParams->scrollOffset * 0.8, safeH);
        } else {
            screenOffsetX = -(loopMax / 2.0) + fmod(dynParams->scrollOffset, loopMax);
            screenOffsetY = -(loopMax / 2.0) + fmod(dynParams->scrollOffset, loopMax);
        }
        break;
    case Core::ScrollFlyBy: {
        double loopZ = 3000.0;
        if (settings->fitOnScreen) {
            // YENİ: Donma yapmayan mükemmel merkezli ping-pong genliği
            worldOffsetZ = 500.0 - getPingPong(dynParams->scrollOffset * 1.5, 1000.0);
        } else {
            worldOffsetZ = 2000.0 - fmod(dynParams->scrollOffset * 4.0, loopZ);
        }
        if (worldOffsetZ < -550.0) worldOffsetZ = -550.0;
        worldOffsetX = -worldOffsetZ * depthAngleX;
        worldOffsetY = -worldOffsetZ * depthAngleY;
        break;
    }
    case Core::ScrollAbyss: {
        double loopZ = 3000.0;
        if (settings->fitOnScreen) {
            // YENİ: Donma yapmayan mükemmel merkezli ping-pong genliği
            worldOffsetZ = 500.0 + getPingPong(dynParams->scrollOffset * 1.5, 1000.0);
        } else {
            worldOffsetZ = -500.0 + fmod(dynParams->scrollOffset * 4.0, loopZ);
        }
        if (worldOffsetZ < -550.0) worldOffsetZ = -550.0;
        worldOffsetX = -worldOffsetZ * depthAngleX;
        worldOffsetY = -worldOffsetZ * depthAngleY;
        break;
    }
    case Core::ScrollStarWars: {
        double loopZ = 2500.0;
        double baseOffsetY = 0.0;

        if (settings->fitOnScreen) {
            worldOffsetZ = 600.0 + getPingPong(dynParams->scrollOffset * 1.0, 250.0);
            baseOffsetY = 0.0;
        } else {
            double progress = fmod(dynParams->scrollOffset * 2.0, loopZ);
            baseOffsetY = 600.0 - progress * 0.8;
            worldOffsetZ = -200.0 + progress * 1.5;
        }

        if (worldOffsetZ < -550.0) worldOffsetZ = -550.0;

        worldOffsetX = -worldOffsetZ * depthAngleX;
        worldOffsetY = baseOffsetY - (worldOffsetZ * depthAngleY);

        rotX += -1.1;
        break;
    }
    case Core::ScrollOrbit: {
        double loopDist = 2000.0;
        double progress = fmod(dynParams->scrollOffset, loopDist) / loopDist;
        double angle = progress * (2.0 * 3.14159265);
        double radius = settings->fitOnScreen ? safeW * 0.8 : size.width() * 0.35;
        worldOffsetX = std::sin(angle) * radius;
        worldOffsetZ = std::cos(angle) * radius;
        break;
    }
    case Core::ScrollTornado: {
        double loopDist = 2500.0;
        double progress = fmod(dynParams->scrollOffset, loopDist) / loopDist;
        double angle = progress * (6.0 * 3.14159265);
        double radius = settings->fitOnScreen ? safeW * 0.6 : 50.0 + (progress * (size.width() * 0.35));
        double startY = (size.height() / 2.0) + 150.0;
        double endY   = -(size.height() / 2.0) - 150.0;
        if (settings->fitOnScreen) worldOffsetY = getPingPong(dynParams->scrollOffset * 1.5, safeH);
        else worldOffsetY = startY + (endY - startY) * progress;
        worldOffsetX = std::sin(angle) * radius;
        worldOffsetZ = std::cos(angle) * radius - 100.0;
        rotY += angle;
        break;
    }
    case Core::ScrollZBounce: {
        double loopX_zb = limitX * 1.5;
        if (settings->fitOnScreen) screenOffsetX = getPingPong(dynParams->scrollOffset * 0.8, safeW);
        else screenOffsetX = (loopX_zb / 2.0) - fmod(dynParams->scrollOffset, loopX_zb);
        worldOffsetZ = std::sin(dynParams->scrollOffset * 0.01) * 600.0;
        break;
    }
    case Core::ScrollSineWave: {
        double loopX_sw = limitX * 1.5;
        if (settings->fitOnScreen) screenOffsetX = getPingPong(dynParams->scrollOffset * 0.8, safeW);
        else screenOffsetX = (loopX_sw / 2.0) - fmod(dynParams->scrollOffset, loopX_sw);
        screenOffsetY = std::sin(dynParams->scrollOffset * 0.015) * safeH;
        break;
    }
    case Core::ScrollBouncingDVD: {
        double tX = dynParams->scrollOffset * 0.8;
        double tY = dynParams->scrollOffset * 0.55;
        screenOffsetX = std::abs(fmod(tX, safeW * 4.0) - safeW * 2.0) - safeW;
        screenOffsetY = std::abs(fmod(tY, safeH * 4.0) - safeH * 2.0) - safeH;
        break;
    }
    case Core::ScrollPendulum: {
        double swingTime = dynParams->time * 2.5;
        rotZ = std::sin(swingTime) * 0.6;
        screenOffsetX = std::sin(swingTime) * safeW;
        screenOffsetY = -150.0 + std::cos(swingTime * 2.0) * 40.0;
        break;
    }
    }

    painter.translate(size.width() / 2.0 + screenOffsetX, size.height() / 2.0 + screenOffsetY);

    QColor baseColor = settings->textColor;
    if (settings->autoColorFlow && !settings->rainbowMode) {
        baseColor = QColor::fromHsv(static_cast<int>(dynParams->currentHue), 220, 255);
    }
    if (high > 0.1) {
        baseColor = baseColor.lighter(100 + static_cast<int>(high * 150));
    }

    auto project = [&](const QVector3D &pt) -> QPointF {
        QVector3D worldPt = pt + QVector3D(worldOffsetX, worldOffsetY, worldOffsetZ);
        double px = (worldPt.x() + worldPt.z() * depthAngleX) * globalScale;
        double py = (worldPt.y() + worldPt.z() * depthAngleY) * globalScale;
        double effectiveZ = camDist + worldOffsetZ;
        if (effectiveZ <= 0.1) return QPointF(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN());
        double factor = camDist / effectiveZ;
        return QPointF(px * factor, py * factor);
    };

    QMatrix4x4 rotMatrix;
    rotMatrix.rotate(qRadiansToDegrees(rotX), 1, 0, 0);
    rotMatrix.rotate(qRadiansToDegrees(rotY), 0, 1, 0);
    rotMatrix.rotate(qRadiansToDegrees(rotZ), 0, 0, 1);

    double safeWidth = std::max(1.0, dynParams->textBoundingWidth);
    auto getRainbowHue = [&](double fraction) -> double {
        return fmod(dynParams->currentHue + (fraction * 360.0) + 720.0, 360.0);
    };

    auto getRainbowColor = [&](double localX) -> QColor {
        if (!settings->rainbowMode) return baseColor;
        double fraction = (localX + safeWidth / 2.0) / safeWidth;
        double finalHue = getRainbowHue(fraction);
        QColor c = QColor::fromHsv(static_cast<int>(finalHue), 220, 255);
        if (high > 0.1) c = c.lighter(100 + static_cast<int>(high * 150));
        return c;
    };

    if (settings->renderMode == Core::Wireframe) {
        painter.setBrush(Qt::NoBrush);
        for (const auto& line2D : dynParams->cached2DLines) {
            QVector3D p1_f = rotMatrix * QVector3D(line2D.first.x(), line2D.first.y(), halfDepth);
            QVector3D p2_f = rotMatrix * QVector3D(line2D.second.x(), line2D.second.y(), halfDepth);
            QVector3D p1_b = rotMatrix * QVector3D(line2D.first.x(), line2D.first.y(), -halfDepth);
            QVector3D p2_b = rotMatrix * QVector3D(line2D.second.x(), line2D.second.y(), -halfDepth);

            QPointF proj1_f = project(p1_f);
            QPointF proj2_f = project(p2_f);
            QPointF proj1_b = project(p1_b);
            QPointF proj2_b = project(p2_b);

            double lineCX = (line2D.first.x() + line2D.second.x()) / 2.0;
            QPen pen(getRainbowColor(lineCX));
            pen.setWidthF(std::max(1.0, 2.0 + low * 3.0));
            painter.setPen(pen);

            if (!qIsNaN(proj1_f.x()) && !qIsNaN(proj2_f.x())) painter.drawLine(proj1_f, proj2_f);
            if (!qIsNaN(proj1_b.x()) && !qIsNaN(proj2_b.x())) painter.drawLine(proj1_b, proj2_b);
            if (!qIsNaN(proj1_f.x()) && !qIsNaN(proj1_b.x())) painter.drawLine(proj1_f, proj1_b);
        }

    } else if (settings->renderMode == Core::Layered || settings->renderMode == Core::Solid) {

        QVector3D viewDir(-depthAngleX, -depthAngleY, 1.0f);
        viewDir.normalize();

        auto getShadedColor = [&](const QVector3D& normal, bool isFrontFace, QColor targetColor) -> QColor {
            if (isFrontFace) return targetColor;

            float diffuse = std::max(0.0f, QVector3D::dotProduct(normal.normalized(), viewDir));
            float ambient = 0.15f;
            float intensity = std::min(1.0f, ambient + (diffuse * 0.5f));

            return QColor(
                std::clamp(int(targetColor.red() * intensity), 0, 255),
                std::clamp(int(targetColor.green() * intensity), 0, 255),
                std::clamp(int(targetColor.blue() * intensity), 0, 255)
                );
        };

        struct RenderItem {
            enum Type { Path, Quad } type;
            QPainterPath path;
            QPolygonF quad;
            double zDepth;
            QBrush brush;
        };

        QVector<RenderItem> items;
        QBrush frontBrush;
        QBrush backBrush;

        if (settings->rainbowMode) {
            QPointF pStartF = project(rotMatrix * QVector3D(-dynParams->textBoundingWidth/2.0, 0, halfDepth));
            QPointF pEndF = project(rotMatrix * QVector3D(dynParams->textBoundingWidth/2.0, 0, halfDepth));
            QPointF pStartB = project(rotMatrix * QVector3D(-dynParams->textBoundingWidth/2.0, 0, -halfDepth));
            QPointF pEndB = project(rotMatrix * QVector3D(dynParams->textBoundingWidth/2.0, 0, -halfDepth));

            if (qIsNaN(pStartF.x()) || qIsNaN(pEndF.x()) || qIsNaN(pStartB.x()) || qIsNaN(pEndB.x())) {
                frontBrush = QBrush(baseColor);
                backBrush = QBrush(baseColor);
            } else {
                QLinearGradient frontGrad(pStartF, pEndF);
                QLinearGradient backGrad(pStartB, pEndB);

                for (int i = 0; i <= 10; ++i) {
                    double fraction = i / 10.0;
                    double h = getRainbowHue(fraction);
                    QColor c = QColor::fromHsv(static_cast<int>(h), 220, 255);
                    if (high > 0.1) c = c.lighter(100 + static_cast<int>(high * 150));

                    frontGrad.setColorAt(fraction, getShadedColor(rotMatrix * QVector3D(0, 0, 1), true, c));
                    backGrad.setColorAt(fraction, getShadedColor(rotMatrix * QVector3D(0, 0, -1), false, c));
                }
                frontBrush = QBrush(frontGrad);
                backBrush = QBrush(backGrad);
            }
        } else {
            frontBrush = QBrush(getShadedColor(rotMatrix * QVector3D(0, 0, 1), true, baseColor));
            backBrush = QBrush(getShadedColor(rotMatrix * QVector3D(0, 0, -1), false, baseColor));
        }

        QPainterPath frontPath;
        frontPath.setFillRule(Qt::OddEvenFill);
        double frontZ = (rotMatrix * QVector3D(0, 0, halfDepth)).z();

        for (const QPolygonF& poly : dynParams->cachedPolys) {
            QPolygonF projPoly;
            for (const QPointF& pt : poly) projPoly << project(rotMatrix * QVector3D(pt.x(), pt.y(), halfDepth));
            frontPath.addPolygon(projPoly);
        }
        if (QVector3D::dotProduct(rotMatrix * QVector3D(0, 0, 1), viewDir) > 0) {
            items.append({RenderItem::Path, frontPath, QPolygonF(), frontZ, frontBrush});
        }

        QPainterPath backPath;
        backPath.setFillRule(Qt::OddEvenFill);
        double backZ = (rotMatrix * QVector3D(0, 0, -halfDepth)).z();

        for (const QPolygonF& poly : dynParams->cachedPolys) {
            QPolygonF projPoly;
            for (const QPointF& pt : poly) projPoly << project(rotMatrix * QVector3D(pt.x(), pt.y(), -halfDepth));
            backPath.addPolygon(projPoly);
        }
        if (QVector3D::dotProduct(rotMatrix * QVector3D(0, 0, -1), viewDir) > 0) {
            items.append({RenderItem::Path, backPath, QPolygonF(), backZ, backBrush});
        }

        for (const auto& line2D : dynParams->cached2DLines) {
            QVector3D p1_f = rotMatrix * QVector3D(line2D.first.x(), line2D.first.y(), halfDepth);
            QVector3D p2_f = rotMatrix * QVector3D(line2D.second.x(), line2D.second.y(), halfDepth);
            QVector3D p2_b = rotMatrix * QVector3D(line2D.second.x(), line2D.second.y(), -halfDepth);
            QVector3D p1_b = rotMatrix * QVector3D(line2D.first.x(), line2D.first.y(), -halfDepth);

            QVector3D v1 = p2_f - p1_f;
            QVector3D v2 = p1_b - p1_f;
            QVector3D normal = QVector3D::crossProduct(v1, v2).normalized();

            if (QVector3D::dotProduct(normal, viewDir) <= 0) continue;

            double quadZ = (p1_f.z() + p2_f.z() + p2_b.z() + p1_b.z()) / 4.0;
            double quadCX = (line2D.first.x() + line2D.second.x()) / 2.0;
            QColor quadBaseColor = getRainbowColor(quadCX);
            QBrush quadBrush(getShadedColor(normal, false, quadBaseColor));

            QPointF proj1_f = project(p1_f);
            QPointF proj2_f = project(p2_f);
            QPointF proj2_b = project(p2_b);
            QPointF proj1_b = project(p1_b);

            if (!qIsNaN(proj1_f.x()) && !qIsNaN(proj2_f.x()) && !qIsNaN(proj2_b.x()) && !qIsNaN(proj1_b.x())) {
                QPolygonF quad;
                quad << proj1_f << proj2_f << proj2_b << proj1_b;
                items.append({RenderItem::Quad, QPainterPath(), quad, quadZ, quadBrush});
            }
        }

        std::sort(items.begin(), items.end(), [](const RenderItem& a, const RenderItem& b) {
            return a.zDepth < b.zDepth;
        });

        if (settings->renderMode == Core::Layered) {
            QColor edgeColor = baseColor.darker(150);
            QPen edgePen(edgeColor);
            edgePen.setWidthF(1.0);

            for (const RenderItem& item : items) {
                painter.setBrush(item.brush);
                painter.setPen(edgePen);
                if (item.type == RenderItem::Path) painter.drawPath(item.path);
                else painter.drawPolygon(item.quad);
            }
        } else if (settings->renderMode == Core::Solid) {
            painter.setPen(Qt::NoPen);

            for (const RenderItem& item : items) {
                painter.setBrush(item.brush);
                if (item.type == RenderItem::Path) painter.drawPath(item.path);
                else painter.drawPolygon(item.quad);
            }
        }
    }

    return img;
}


struct SolidFace3D {
    QPolygonF projectedPoints;
    double avgZ;
    QColor color;
};

QImage AnimationAlgorithms::CreateSolidText3D(QSize size, Core::SolidText3DSettings* settings, Core::SolidText3DDynamicParams* dynParams, const QVector<QVector3D>& ledCoords, QVector<QColor>& ledColors, Core::ReactionSettings* reactionSettings, double low, double mid, double high)
{
    Q_UNUSED(reactionSettings);

    QImage frame(size, QImage::Format_ARGB32_Premultiplied);
    frame.fill(settings->backgroundColor);

    if (settings->text.isEmpty()) {
        ledColors.fill(settings->backgroundColor);
        return frame;
    }

    dynParams->phase += 0.05 * (1.0 + high);

    if (dynParams->lastDirection != settings->direction) {
        if (settings->resetOnDirectionChange) {
            dynParams->rotationX = 0.0;
            dynParams->rotationY = 0.0;
            dynParams->rotationZ = 0.0;
        }
        dynParams->lastDirection = settings->direction;
    }

    if (settings->speedFactor <= 0.0) {
        dynParams->rotationX = 0.0;
        dynParams->rotationY = 0.0;
        dynParams->rotationZ = 0.0;
    } else {
        double rotSpeed = settings->speedFactor * 0.05 * (1.0 + mid);
        switch (settings->direction) {
        case Core::_3D_X_Plus:  dynParams->rotationX += rotSpeed; break;
        case Core::_3D_X_Minus: dynParams->rotationX -= rotSpeed; break;
        case Core::_3D_Y_Plus:  dynParams->rotationY += rotSpeed; break;
        case Core::_3D_Y_Minus: dynParams->rotationY -= rotSpeed; break;
        case Core::_3D_Z_Plus:  dynParams->rotationZ += rotSpeed; break;
        case Core::_3D_Z_Minus: dynParams->rotationZ -= rotSpeed; break;
        default: dynParams->rotationZ += rotSpeed; break;
        }
    }

    double targetThickness = settings->thickness * (1.0 + low * 2.0);

    bool needsUpdate = (dynParams->lastText != settings->text ||
                        dynParams->lastFontFamily != settings->fontFamily ||
                        dynParams->lastThickness != targetThickness ||
                        dynParams->lastFontSize != settings->fontSize ||
                        dynParams->lastAutoScale != settings->autoScale);

    if (needsUpdate) {
        int genSize = settings->autoScale ? 100 : settings->fontSize;
        QFont finalFont(settings->fontFamily, genSize, QFont::Bold);
        QFontMetrics finalFm(finalFont);

        QPainterPath rawPath;
        rawPath.addText(0, 0, finalFont, settings->text);

        QRectF bounds = rawPath.boundingRect();

        QTransform centerTransform;
        centerTransform.translate(-bounds.center().x(), -bounds.center().y());
        QPainterPath centeredPath = centerTransform.map(rawPath);
        bounds = centeredPath.boundingRect();

        double scaleFactor = 1.0;

        if (settings->autoScale) {
            double targetWidth = 2.4;
            if (bounds.width() > 0) {
                scaleFactor = targetWidth / bounds.width();
            }
            dynParams->calculatedFontSize = static_cast<int>(genSize * (scaleFactor / 0.0075));
        } else {
            scaleFactor = 0.0075;
            dynParams->calculatedFontSize = settings->fontSize;
        }

        QTransform transform;
        transform.scale(scaleFactor, scaleFactor);

        dynParams->cachedPath = transform.map(centeredPath);
        dynParams->cachedPolys = dynParams->cachedPath.toSubpathPolygons();

        dynParams->lastText = settings->text;
        dynParams->lastFontFamily = settings->fontFamily;
        dynParams->lastThickness = targetThickness;
        dynParams->lastFontSize = settings->fontSize;
        dynParams->lastAutoScale = settings->autoScale;
    }

    QVector3D minP(1e9, 1e9, 1e9), maxP(-1e9, -1e9, -1e9);
    for (const auto& p : ledCoords) {
        minP.setX(std::min(minP.x(), p.x()));
        minP.setY(std::min(minP.y(), p.y()));
        minP.setZ(std::min(minP.z(), p.z()));
        maxP.setX(std::max(maxP.x(), p.x()));
        maxP.setY(std::max(maxP.y(), p.y()));
        maxP.setZ(std::max(maxP.z(), p.z()));
    }

    QVector3D center = (minP + maxP) / 2.0f;
    double maxDim = std::max({maxP.x() - minP.x(), maxP.y() - minP.y(), maxP.z() - minP.z()});
    if (maxDim < 1e-3) maxDim = 1.0;

    QMatrix4x4 invRotMatrix;
    invRotMatrix.rotate(-qRadiansToDegrees(dynParams->rotationZ), 0, 0, 1);
    invRotMatrix.rotate(-qRadiansToDegrees(dynParams->rotationY), 0, 1, 0);
    invRotMatrix.rotate(-qRadiansToDegrees(dynParams->rotationX), 1, 0, 0);

    double halfThick = targetThickness / 2.0;

#pragma omp parallel for
    for (int i = 0; i < ledCoords.size(); ++i) {
        QVector3D normP = (ledCoords[i] - center) / maxDim;
        QVector3D localP = invRotMatrix * normP;

        if (std::abs(localP.z()) > halfThick) {
            ledColors[i] = settings->backgroundColor;
            continue;
        }

        if (dynParams->cachedPath.contains(QPointF(localP.x(), localP.y()))) {
            double t = (localP.z() + halfThick) / targetThickness;
            t = std::clamp(t, 0.0, 1.0);

            int r = settings->colorBack.red() + t * (settings->colorFront.red() - settings->colorBack.red());
            int g = settings->colorBack.green() + t * (settings->colorFront.green() - settings->colorBack.green());
            int b = settings->colorBack.blue() + t * (settings->colorFront.blue() - settings->colorBack.blue());

            ledColors[i] = QColor(r, g, b);
        } else {
            ledColors[i] = settings->backgroundColor;
        }
    }

    QMatrix4x4 rotMatrix;
    rotMatrix.rotate(qRadiansToDegrees(dynParams->rotationX), 1, 0, 0);
    rotMatrix.rotate(qRadiansToDegrees(dynParams->rotationY), 0, 1, 0);
    rotMatrix.rotate(qRadiansToDegrees(dynParams->rotationZ), 0, 0, 1);

    QList<SolidFace3D> faces;

    auto projectPoint = [](const QVector3D& p, const QSize& s) -> QPointF {
        double rotX = p.x() + p.z() * 0.2;
        double rotY = p.y() - p.z() * 0.2;
        double scale = static_cast<double>(std::min(s.width(), s.height())) * 0.45;
        return QPointF(s.width() / 2.0 + rotX * scale, s.height() / 2.0 + rotY * scale);
    };

    QColor sideColor = settings->colorFront.darker(150);

    for (const QPolygonF& poly : dynParams->cachedPolys) {
        if (poly.size() < 3) continue;

        QPolygonF projectedFront, projectedBack;
        double avgZFront = 0, avgZBack = 0;

        for (int i = 0; i < poly.size(); ++i) {
            QVector3D p_f = rotMatrix * QVector3D(poly[i].x(), poly[i].y(), halfThick);
            QVector3D p_b = rotMatrix * QVector3D(poly[i].x(), poly[i].y(), -halfThick);

            projectedFront << projectPoint(p_f, size);
            projectedBack << projectPoint(p_b, size);

            avgZFront += p_f.z();
            avgZBack += p_b.z();

            int next = (i + 1) % poly.size();
            QVector3D p_f_next = rotMatrix * QVector3D(poly[next].x(), poly[next].y(), halfThick);
            QVector3D p_b_next = rotMatrix * QVector3D(poly[next].x(), poly[next].y(), -halfThick);

            SolidFace3D sideFace;
            sideFace.projectedPoints << projectPoint(p_f, size)
                                     << projectPoint(p_f_next, size)
                                     << projectPoint(p_b_next, size)
                                     << projectPoint(p_b, size);
            sideFace.avgZ = (p_f.z() + p_f_next.z() + p_b.z() + p_b_next.z()) / 4.0;
            sideFace.color = sideColor;
            faces.append(sideFace);
        }

        SolidFace3D frontFace;
        frontFace.projectedPoints = projectedFront;
        frontFace.avgZ = avgZFront / poly.size();
        frontFace.color = settings->colorFront;
        faces.append(frontFace);

        SolidFace3D backFace;
        backFace.projectedPoints = projectedBack;
        backFace.avgZ = avgZBack / poly.size();
        backFace.color = settings->colorBack;
        faces.append(backFace);
    }

    std::sort(faces.begin(), faces.end(), [](const SolidFace3D& a, const SolidFace3D& b) {
        return a.avgZ < b.avgZ;
    });

    QPainter painter(&frame);
    painter.setRenderHint(QPainter::Antialiasing);

    for (const SolidFace3D& face : faces) {
        painter.setBrush(face.color);
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(face.projectedPoints);

        QPen outlinePen(face.color.darker(200), 1);
        painter.setPen(outlinePen);
        painter.setBrush(Qt::NoBrush);
        painter.drawPolygon(face.projectedPoints);
    }

    if (settings->showGizmo) {
        QPointF gCenter(50, size.height() - 50);
        double gScale = 30.0;

        auto projectGizmo = [&](const QVector3D& p) -> QPointF {
            double rotX = p.x() + p.z() * 0.2;
            double rotY = p.y() - p.z() * 0.2;
            return QPointF(gCenter.x() + rotX * gScale, gCenter.y() + rotY * gScale);
        };

        QVector3D xAxis = rotMatrix * QVector3D(1, 0, 0);
        QVector3D yAxis = rotMatrix * QVector3D(0, 1, 0);
        QVector3D zAxis = rotMatrix * QVector3D(0, 0, 1);

        QFont gizmoFont = painter.font();
        gizmoFont.setBold(true);
        painter.setFont(gizmoFont);

        painter.setPen(QPen(Qt::red, 2));
        painter.drawLine(gCenter, projectGizmo(xAxis));
        painter.drawText(projectGizmo(xAxis) + QPointF(5, 5), "X");

        painter.setPen(QPen(Qt::green, 2));
        painter.drawLine(gCenter, projectGizmo(yAxis));
        painter.drawText(projectGizmo(yAxis) + QPointF(5, 5), "Y");

        painter.setPen(QPen(Qt::blue, 2));
        painter.drawLine(gCenter, projectGizmo(zAxis));
        painter.drawText(projectGizmo(zAxis) + QPointF(5, 5), "Z");
    }

    return frame;
}


QImage AnimationAlgorithms::CreateLinearWave(QSize size, Core::LinearWaveSettings* settings, Core::LinearWaveDynamicParams* dynParams, const QVector<QVector3D>& ledCoords, QVector<QColor>& ledColors, Core::ReactionSettings* reactionSettings, double low, double mid, double high)
{
    if (!settings || !dynParams || ledCoords.isEmpty()) return QImage(size, QImage::Format_ARGB32_Premultiplied);
    if (ledColors.size() != ledCoords.size()) ledColors.resize(ledCoords.size());
    std::fill(ledColors.begin(), ledColors.end(), settings->backgroundColor);

    float minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9, minZ = 1e9, maxZ = -1e9;
    for (const QVector3D& p : ledCoords) {
        if (p.x() < minX) minX = p.x(); if (p.x() > maxX) maxX = p.x();
        if (p.y() < minY) minY = p.y(); if (p.y() > maxY) maxY = p.y();
        if (p.z() < minZ) minZ = p.z(); if (p.z() > maxZ) maxZ = p.z();
    }

    QVector3D center((minX + maxX) * 0.5, (minY + maxY) * 0.5, (minZ + maxZ) * 0.5);
    double dimX = std::max(1.0, double(maxX - minX));
    double dimY = std::max(1.0, double(maxY - minY));
    double dimZ = std::max(1.0, double(maxZ - minZ));
    double maxDim = std::max(dimX, dimZ);
    double allowedRadius = (settings->activeGridSize * 2.5) / 2.0;

    double audioSpeedMod = 1.0 + (mid * (reactionSettings ? reactionSettings->midGain : 1.0) * 0.8);
    double audioAmpMod = 1.0 + (low * (reactionSettings ? reactionSettings->bassGain : 1.0) * 0.5);

    dynParams->time += 0.05 * settings->speedFactor * audioSpeedMod;

    double theta = 0.0;
    switch(settings->direction) {
    case Core::_3D_X_Plus:  theta = 0.0; break;
    case Core::_3D_X_Minus: theta = 3.14159265; break;
    case Core::_3D_Z_Plus:  theta = 1.57079632; break;
    case Core::_3D_Z_Minus: theta = -1.57079632; break;
    case Core::_3D_Y_Plus:  theta = 0.78539816; break;
    case Core::_3D_Y_Minus: theta = -0.78539816; break;
    }

    double cosT = std::cos(theta);
    double sinT = std::sin(theta);
    double targetAmplitude = settings->amplitude * audioAmpMod;

#pragma omp parallel for
    for (int i = 0; i < ledCoords.size(); ++i) {
        QVector3D p = ledCoords[i];
        double dx = p.x() - center.x();
        double dz = p.z() - center.z();

        if (std::abs(dx) > allowedRadius || std::abs(dz) > allowedRadius) continue;

        double u = (dx * cosT + dz * sinT) / (maxDim * 0.5);
        double sineVal = (std::sin(settings->waveNumber * u - dynParams->time) + 1.0) * 0.5;
        double waveHeight = minY + (sineVal * dimY * targetAmplitude);

        if (p.y() <= waveHeight) ledColors[i] = settings->waveColor;
    }

    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(settings->backgroundColor);
    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);

    QPointF center2D(size.width() / 2.0, size.height() / 2.0);
    double previewScale = size.height() / (std::max({dimX, dimY, dimZ}) * 1.5);

    QVector<QPair<double, QVector3D>> fountainBases;
    for (const QVector3D& p : ledCoords) {
        if (std::abs(p.y() - minY) < 3.0) {
            double dx = p.x() - center.x();
            double dz = p.z() - center.z();
            if (std::abs(dx) <= allowedRadius && std::abs(dz) <= allowedRadius) {
                fountainBases.append({p.x() + p.z(), p});
            }
        }
    }

    std::sort(fountainBases.begin(), fountainBases.end(), [](const QPair<double, QVector3D>& a, const QPair<double, QVector3D>& b) {
        return a.first < b.first;
    });

    QPen barPen(settings->waveColor);
    barPen.setCapStyle(Qt::RoundCap);
    barPen.setWidth(5);

    for (const auto& item : fountainBases) {
        QVector3D basePt = item.second;
        double u = ((basePt.x() - center.x()) * cosT + (basePt.z() - center.z()) * sinT) / (maxDim * 0.5);
        double sineVal = (std::sin(settings->waveNumber * u - dynParams->time) + 1.0) * 0.5;
        double currentWaveHeight = minY + (sineVal * dimY * targetAmplitude);
        if (currentWaveHeight - minY < 1.0) continue;

        QVector3D bP = basePt - center;
        QVector3D tP = bP; tP.setY(currentWaveHeight - center.y());

        QPointF screenBase = projectIsometric(QVector3D(bP.x(), bP.z(), bP.y()), center2D, previewScale);
        QPointF screenTop = projectIsometric(QVector3D(tP.x(), tP.z(), tP.y()), center2D, previewScale);

        QColor col = settings->waveColor;
        int darkenAmount = std::clamp(static_cast<int>(((item.first - (minX+minZ)) / (dimX+dimZ)) * 80.0), 0, 80);
        barPen.setColor(col.darker(100 + darkenAmount));
        painter.setPen(barPen);
        painter.drawLine(screenBase, screenTop);
    }

    DrawReferenceGizmo(&painter, size, 0.6, 0.78, 0.0);
    return img;
}

QImage AnimationAlgorithms::CreateWaveFountain3D(QSize size, Core::WaveFountain3DSettings* settings, Core::WaveFountain3DDynamicParams* dynParams, const QVector<QVector3D>& ledCoords, QVector<QColor>& ledColors, Core::ReactionSettings* reactionSettings, double low, double mid, double high)
{
    if (!settings || !dynParams || ledCoords.isEmpty()) return QImage(size, QImage::Format_ARGB32_Premultiplied);
    if (ledColors.size() != ledCoords.size()) ledColors.resize(ledCoords.size());
    std::fill(ledColors.begin(), ledColors.end(), settings->backgroundColor);

    float minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9, minZ = 1e9, maxZ = -1e9;
    for (const QVector3D& p : ledCoords) {
        if (p.x() < minX) minX = p.x(); if (p.x() > maxX) maxX = p.x();
        if (p.y() < minY) minY = p.y(); if (p.y() > maxY) maxY = p.y();
        if (p.z() < minZ) minZ = p.z(); if (p.z() > maxZ) maxZ = p.z();
    }

    QVector3D center((minX + maxX) * 0.5, (minY + maxY) * 0.5, (minZ + maxZ) * 0.5);
    double dimX = std::max(1.0, double(maxX - minX));
    double dimY = std::max(1.0, double(maxY - minY));
    double dimZ = std::max(1.0, double(maxZ - minZ));
    double maxDim = std::max(dimX, dimZ);
    double allowedRadius = (settings->activeGridSize * 2.5) / 2.0;

    double audioSpeedMod = 1.0 + (mid * (reactionSettings ? reactionSettings->midGain : 1.0) * 0.8);
    double audioAmpMod = 1.0 + (low * (reactionSettings ? reactionSettings->bassGain : 1.0) * 0.5);

    dynParams->time += 0.05 * settings->speedFactor * audioSpeedMod;
    double timeDir = (settings->direction == Core::_3D_X_Minus || settings->direction == Core::_3D_Y_Minus || settings->direction == Core::_3D_Z_Minus) ? 1.0 : -1.0;
    double targetAmplitude = settings->amplitude * audioAmpMod;

#pragma omp parallel for
    for (int i = 0; i < ledCoords.size(); ++i) {
        QVector3D p = ledCoords[i];
        double dx = p.x() - center.x();
        double dz = p.z() - center.z();

        if (std::abs(dx) > allowedRadius || std::abs(dz) > allowedRadius) continue;

        double dist = std::sqrt(dx * dx + dz * dz);
        double u = dist / (maxDim * 0.5);
        double sineVal = (std::sin(settings->waveNumber * u + (dynParams->time * timeDir)) + 1.0) * 0.5;
        double waveHeight = minY + (sineVal * dimY * targetAmplitude);

        if (p.y() <= waveHeight) ledColors[i] = settings->waterColor;
    }

    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(settings->backgroundColor);
    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);

    QPointF center2D(size.width() / 2.0, size.height() / 2.0);
    double previewScale = size.height() / (std::max({dimX, dimY, dimZ}) * 1.5);

    QVector<QPair<double, QVector3D>> fountainBases;
    for (const QVector3D& p : ledCoords) {
        if (std::abs(p.y() - minY) < 3.0) {
            double dx = p.x() - center.x();
            double dz = p.z() - center.z();
            if (std::abs(dx) <= allowedRadius && std::abs(dz) <= allowedRadius) {
                fountainBases.append({p.x() + p.z(), p});
            }
        }
    }

    std::sort(fountainBases.begin(), fountainBases.end(), [](const QPair<double, QVector3D>& a, const QPair<double, QVector3D>& b) {
        return a.first < b.first;
    });

    QPen barPen(settings->waterColor);
    barPen.setCapStyle(Qt::RoundCap);
    barPen.setWidth(5);

    for (const auto& item : fountainBases) {
        QVector3D basePt = item.second;
        double dx = basePt.x() - center.x();
        double dz = basePt.z() - center.z();
        double dist = std::sqrt(dx * dx + dz * dz);
        double u = dist / (maxDim * 0.5);
        double sineVal = (std::sin(settings->waveNumber * u + (dynParams->time * timeDir)) + 1.0) * 0.5;
        double currentWaveHeight = minY + (sineVal * dimY * targetAmplitude);
        if (currentWaveHeight - minY < 1.0) continue;

        QVector3D bP = basePt - center;
        QVector3D tP = bP; tP.setY(currentWaveHeight - center.y());

        QPointF screenBase = projectIsometric(QVector3D(bP.x(), bP.z(), bP.y()), center2D, previewScale);
        QPointF screenTop = projectIsometric(QVector3D(tP.x(), tP.z(), tP.y()), center2D, previewScale);

        QColor col = settings->waterColor;
        int darkenAmount = std::clamp(static_cast<int>(((item.first - (minX+minZ)) / (dimX+dimZ)) * 80.0), 0, 80);
        barPen.setColor(col.darker(100 + darkenAmount));
        painter.setPen(barPen);
        painter.drawLine(screenBase, screenTop);
    }

    return img;
}

QImage AnimationAlgorithms::CreatePendulumWave(QSize size, Core::PendulumWaveSettings* settings, Core::PendulumWaveDynamicParams* dynParams, const QVector<QVector3D>& ledCoords, QVector<QColor>& ledColors, Core::ReactionSettings* reactionSettings, double low, double mid, double high)
{
    const Core::ReactionSettings defaultReaction;
    const Core::ReactionSettings* rs = reactionSettings ? reactionSettings : &defaultReaction;

    if (ledColors.size() != ledCoords.size()) ledColors.resize(ledCoords.size());
    std::fill(ledColors.begin(), ledColors.end(), settings->backgroundColor);

    float minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9, minZ = 1e9, maxZ = -1e9;
    for (const QVector3D& p : ledCoords) {
        minX = std::min(minX, p.x()); maxX = std::max(maxX, p.x());
        minY = std::min(minY, p.y()); maxY = std::max(maxY, p.y());
        minZ = std::min(minZ, p.z()); maxZ = std::max(maxZ, p.z());
    }

    QVector3D center((minX + maxX) * 0.5, (minY + maxY) * 0.5, (minZ + maxZ) * 0.5);
    double boundsX = std::max(1.0, (maxX - minX) * 0.5);
    double boundsY = std::max(1.0, (maxY - minY) * 0.5);
    double boundsZ = std::max(1.0, (maxZ - minZ) * 0.5);

    double activeLow = low * rs->bassGain;
    double activeMid = mid * rs->midGain;
    double activeHigh = high * rs->trebleGain;

    double speedMod = 1.0 + (activeMid * 0.5);
    dynParams->internalTime += 0.016 * settings->speedFactor * speedMod;

    double currentAmplitude = settings->amplitude + (activeLow * 1.5);
    double radiusSq = std::pow(settings->sphereSize * std::max({boundsX, boundsY, boundsZ}), 2);

#pragma omp parallel for
    for (int i = 0; i < ledCoords.size(); ++i) {
        QVector3D p = ledCoords[i] - center;
        double sliceIdx = 0;

        switch (settings->direction) {
        case Core::_3D_Z_Plus: case Core::_3D_Z_Minus:
            sliceIdx = (ledCoords[i].z() - minZ) / (maxZ - minZ + 0.1);
            break;
        case Core::_3D_X_Plus: case Core::_3D_X_Minus:
            sliceIdx = (ledCoords[i].x() - minX) / (maxX - minX + 0.1);
            break;
        case Core::_3D_Y_Plus: case Core::_3D_Y_Minus:
            sliceIdx = (ledCoords[i].y() - minY) / (maxY - minY + 0.1);
            break;
        }

        if (settings->direction == Core::_3D_Z_Minus || settings->direction == Core::_3D_X_Minus || settings->direction == Core::_3D_Y_Minus) {
            sliceIdx = 1.0 - sliceIdx;
        }

        double t = dynParams->internalTime;
        if (settings->flatMode) {
            t = std::fmod(t, 10.0);
            double delay = sliceIdx * 2.0;
            t = t - delay;
            if (t < 0) t = 0;
        }

        double currentFreq = settings->baseFrequency + sliceIdx * settings->frequencyStep;
        double swingPos = std::sin(t * currentFreq * 6.28) * currentAmplitude;
        QVector3D spherePos;

        switch (settings->direction) {
        case Core::_3D_Z_Plus: case Core::_3D_Z_Minus:
            spherePos = QVector3D(swingPos * boundsX, 0, p.z());
            break;
        case Core::_3D_X_Plus: case Core::_3D_X_Minus:
            spherePos = QVector3D(p.x(), swingPos * boundsY, 0);
            break;
        case Core::_3D_Y_Plus: case Core::_3D_Y_Minus:
            spherePos = QVector3D(swingPos * boundsX, p.y(), 0);
            break;
        }

        double distSq = (p - spherePos).lengthSquared();
        if (distSq < radiusSq) {
            QColor c = settings->color;
            if (settings->rainbowMode) {
                c = QColor::fromHsvF(std::fmod(sliceIdx + dynParams->internalTime * 0.1, 1.0), 0.8, 1.0);
            }
            if (activeHigh > 0.2) {
                c = c.lighter(100 + static_cast<int>(activeHigh * 150));
            }
            ledColors[i] = c;
        }
    }

    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::black);
    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);

    double previewScale = size.height() / (std::max({boundsX, boundsY, boundsZ}) * 3.0);
    QPointF center2D(size.width() / 2.0, size.height() / 2.0);

    int numPreviewSpheres = 25;
    for (int s = 0; s < numPreviewSpheres; ++s) {
        double sliceFrac = (double)s / (numPreviewSpheres - 1);

        double t = dynParams->internalTime;
        if (settings->flatMode) {
            t = std::fmod(t, 10.0);
            double delay = sliceFrac * 2.0;
            t = t - delay;
            if (t < 0) t = 0;
        }

        double currentFreq = settings->baseFrequency + sliceFrac * settings->frequencyStep;
        double swing = std::sin(t * currentFreq * 6.28) * currentAmplitude;

        QPointF p2d;

        if (settings->flatMode) {
            double screenX = center2D.x() + (sliceFrac - 0.5) * (size.width() * 0.8);
            double screenY = center2D.y() + swing * (size.height() * 0.4);
            p2d = QPointF(screenX, screenY);
        } else {
            QVector3D p3d;
            double axisPos = (sliceFrac - 0.5) * std::max({boundsX, boundsY, boundsZ}) * 2.0;

            switch (settings->direction) {
            case Core::_3D_Z_Plus: case Core::_3D_Z_Minus:
                p3d = QVector3D(swing * boundsX, 0, axisPos);
                break;
            case Core::_3D_X_Plus: case Core::_3D_X_Minus:
                p3d = QVector3D(axisPos, swing * boundsY, 0);
                break;
            case Core::_3D_Y_Plus: case Core::_3D_Y_Minus:
                p3d = QVector3D(swing * boundsX, axisPos, 0);
                break;
            }
            p2d = projectIsometric(p3d, center2D, previewScale);
        }

        QColor c = settings->color;
        if (settings->rainbowMode) {
            c = QColor::fromHsvF(std::fmod(sliceFrac + dynParams->internalTime * 0.1, 1.0), 0.8, 1.0);
        }
        painter.setBrush(c);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(p2d, 10, 10);
    }

    return img;
}

QImage AnimationAlgorithms::CreateKineticRain(QSize size, Core::KineticRainSettings* settings, Core::KineticRainDynamicParams* dynParams, const QVector<QVector3D>& ledCoords, QVector<QColor>& ledColors, Core::ReactionSettings* reactionSettings, double low, double mid, double high)
{
    const Core::ReactionSettings defaultReaction;
    const Core::ReactionSettings* rs = reactionSettings ? reactionSettings : &defaultReaction;

    if (ledColors.size() != ledCoords.size()) ledColors.resize(ledCoords.size());
    std::fill(ledColors.begin(), ledColors.end(), settings->backgroundColor);

    float minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9, minZ = 1e9, maxZ = -1e9;
    for (const QVector3D& p : ledCoords) {
        minX = std::min(minX, p.x()); maxX = std::max(maxX, p.x());
        minY = std::min(minY, p.y()); maxY = std::max(maxY, p.y());
        minZ = std::min(minZ, p.z()); maxZ = std::max(maxZ, p.z());
    }

    QVector3D center((minX + maxX) * 0.5, (minY + maxY) * 0.5, (minZ + maxZ) * 0.5);
    double boundsX = std::max(1.0, (maxX - minX) * 0.5);
    double boundsY = std::max(1.0, (maxY - minY) * 0.5);
    double boundsZ = std::max(1.0, (maxZ - minZ) * 0.5);

    double activeLow = low * rs->bassGain;
    double activeMid = mid * rs->midGain;
    double activeHigh = high * rs->trebleGain;

    double speedMod = 1.0 + (activeMid * 1.5);
    dynParams->internalTime += 0.016 * settings->speedFactor * speedMod;

    double currentAmplitude = settings->amplitude + (activeLow * 0.8);

#pragma omp parallel for
    for (int i = 0; i < ledCoords.size(); ++i) {
        QVector3D p = ledCoords[i] - center;

        double n1 = 0, n2 = 0, v = 0;
        double maxV = 1.0;

        switch (settings->direction) {
        case Core::_3D_Z_Plus: case Core::_3D_Z_Minus:
            n1 = p.x() / boundsX; n2 = p.y() / boundsY; v = p.z(); maxV = boundsZ; break;
        case Core::_3D_X_Plus: case Core::_3D_X_Minus:
            n1 = p.y() / boundsY; n2 = p.z() / boundsZ; v = p.x(); maxV = boundsX; break;
        case Core::_3D_Y_Plus: case Core::_3D_Y_Minus:
            n1 = p.x() / boundsX; n2 = p.z() / boundsZ; v = p.y(); maxV = boundsY; break;
        }

        double waveVal = 0.0;
        if (settings->rippleMode) {
            double r = std::hypot(n1, n2);
            waveVal = std::cos(r * settings->waveFrequency * 5.0 - dynParams->internalTime * 3.0);
        } else {
            waveVal = std::sin(n1 * settings->waveFrequency * 3.0 + dynParams->internalTime * 2.0) * std::cos(n2 * settings->waveFrequency * 3.0 + dynParams->internalTime * 1.5);
        }

        double targetV = waveVal * currentAmplitude * maxV;

        if (settings->direction == Core::_3D_Z_Minus || settings->direction == Core::_3D_X_Minus || settings->direction == Core::_3D_Y_Minus) {
            targetV = -targetV;
        }

        double dist = std::abs(v - targetV);
        double threshold = settings->dropSize * maxV;

        if (dist <= threshold) {
            QColor c = settings->dropColor;

            if (settings->rainbowMode) {
                double hue = std::fmod((n1 + n2) * 0.5 + dynParams->internalTime * 0.2 + 2.0, 1.0);
                c = QColor::fromHsvF(hue, 0.8, 1.0);
            }

            if (waveVal > 0.7 && activeHigh > 0.1) {
                int lightBoost = std::clamp(static_cast<int>(100 + activeHigh * 250), 100, 255);
                c = QColor(255, 255, 255, 255);
            } else if (dist > threshold * 0.5) {
                c.setAlphaF(0.4);
            }

            ledColors[i] = c;
        }
    }

    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(settings->backgroundColor);
    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);

    double previewScale = size.height() / (std::max({boundsX, boundsY, boundsZ}) * 3.0);
    QPointF center2D(size.width() / 2.0, size.height() / 2.0);

    int gridSize = 12;
    for (int x = 0; x < gridSize; ++x) {
        for (int y = 0; y < gridSize; ++y) {
            double n1 = (x / (double)(gridSize - 1)) * 2.0 - 1.0;
            double n2 = (y / (double)(gridSize - 1)) * 2.0 - 1.0;

            double waveVal = 0.0;
            if (settings->rippleMode) {
                double r = std::hypot(n1, n2);
                waveVal = std::cos(r * settings->waveFrequency * 5.0 - dynParams->internalTime * 3.0);
            } else {
                waveVal = std::sin(n1 * settings->waveFrequency * 3.0 + dynParams->internalTime * 2.0) * std::cos(n2 * settings->waveFrequency * 3.0 + dynParams->internalTime * 1.5);
            }

            double targetV = waveVal * currentAmplitude;
            if (settings->direction == Core::_3D_Z_Minus || settings->direction == Core::_3D_X_Minus || settings->direction == Core::_3D_Y_Minus) {
                targetV = -targetV;
            }

            QVector3D p3d;
            switch (settings->direction) {
            case Core::_3D_Z_Plus: case Core::_3D_Z_Minus:
                p3d = QVector3D(n1 * boundsX, n2 * boundsY, targetV * boundsZ); break;
            case Core::_3D_X_Plus: case Core::_3D_X_Minus:
                p3d = QVector3D(targetV * boundsX, n1 * boundsY, n2 * boundsZ); break;
            case Core::_3D_Y_Plus: case Core::_3D_Y_Minus:
                p3d = QVector3D(n1 * boundsX, targetV * boundsY, n2 * boundsZ); break;
            }

            QPointF p2d = projectIsometric(p3d, center2D, previewScale);

            QColor c = settings->dropColor;
            if (settings->rainbowMode) {
                double hue = std::fmod((n1 + n2) * 0.5 + dynParams->internalTime * 0.2 + 2.0, 1.0);
                c = QColor::fromHsvF(hue, 0.8, 1.0);
            }
            if (waveVal > 0.7 && activeHigh > 0.1) {
                c = Qt::white;
            }

            painter.setBrush(c);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(p2d, 5, 5);
        }
    }

    return img;
}
