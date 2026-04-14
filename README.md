# Qt 3D LED Animation Engine

A high-performance C++ and Qt-based framework designed for real-time 3D volumetric LED animations. This engine utilizes custom 3D projection algorithms and multi-threaded rendering to drive LED matrices with complex geometric transformations.

## Technical Features

* **Volumetric Rendering:** Real-time generation of 3D primitives like `HyperCube` and `StaticCube` using mathematical distance fields.
* **Isometric Projection:** Custom mapping of 3D coordinates to 2D UI previews using isometric matrices.
* **3D Text Engine:** Advanced text rendering supporting `Wireframe`, `Layered`, and `Solid` modes with dynamic oscillation styles.
* **Audio Reactivity:** Integrated frequency analysis (Bass/Mid/High) to modulate animation scale, speed, and color in real-time.
* **Multi-threaded Architecture:** A dedicated `FrameRendererThread` handles calculations at ~30 FPS, decoupled from the main UI thread for maximum performance.
* **Performance Optimization:** Utilization of **OpenMP** for parallelizing volumetric calculations across LED coordinates.

## Mathematical Foundation

The engine performs rotations using Euler angles applied through standard rotation matrices:

$$R_x(\theta) = \begin{bmatrix} 1 & 0 & 0 \\ 0 & \cos\theta & -\sin\theta \\ 0 & \sin\theta & \cos\theta \end{bmatrix}$$

$$R_y(\theta) = \begin{bmatrix} \cos\theta & 0 & \sin\theta \\ 0 & 1 & 0 \\ -\sin\theta & 0 & \cos\theta \end{bmatrix}$$

$$R_z(\theta) = \begin{bmatrix} \cos\theta & -\sin\theta & 0 \\ \sin\theta & \cos\theta & 0 \\ 0 & 0 & 1 \end{bmatrix}$$

These transformations allow for seamless axis alignment and dynamic "wobble" effects during animation cycles.

## Tech Stack
* **Framework:** Qt 6.x
* **Language:** C++17
* **Optimization:** OpenMP
* **Hardware Compatibility:** Designed for ESP32/Art-Net/DMX integration.

---

# Qt 3D LED Animasyon Motoru

Gerçek zamanlı 3D volumetrik LED animasyonları için tasarlanmış, yüksek performanslı C++ ve Qt tabanlı bir framework. Bu motor, karmaşık geometrik dönüşümlerle LED matrislerini sürmek için özel 3D projeksiyon algoritmaları ve çok iş parçacıklı (multi-threaded) render yapısını kullanır.

## Teknik Özellikler

* **Volumetrik Render:** Matematiksel mesafe alanlarını kullanarak `HyperCube` ve `StaticCube` gibi 3D primitiflerin gerçek zamanlı üretimi.
* **İzometrik Projeksiyon:** 3D koordinatların izometrik matrisler kullanılarak 2D arayüz önizlemelerine özel haritalanması.
* **3D Metin Motoru:** Dinamik salınım stilleriyle `Wireframe`, `Layered` ve `Solid` modlarını destekleyen gelişmiş metin render sistemi.
* **Ses Reaktif Yapı:** Animasyon ölçeğini, hızını ve rengini gerçek zamanlı modüle etmek için entegre frekans analizi (Bass/Mid/High).
* **Çok İş Parçacıklı Mimari:** Maksimum performans için ana UI iş parçacığından bağımsız, ~30 FPS hızında hesaplama yapan özel `FrameRendererThread`.
* **Performans Optimizasyonu:** LED koordinatları üzerindeki volumetrik hesaplamaları paralelleştirmek için **OpenMP** kullanımı.

## Matematiksel Temel

Motor, standart rotasyon matrisleri aracılığıyla uygulanan Euler açılarını kullanarak döndürme işlemlerini gerçekleştirir:

$$R_x(\theta) = \begin{bmatrix} 1 & 0 & 0 \\ 0 & \cos\theta & -\sin\theta \\ 0 & \sin\theta & \cos\theta \end{bmatrix}$$

Bu dönüşümler, animasyon döngüleri sırasında kusursuz eksen hizalaması ve dinamik "wobble" (salınım) efektleri sağlar.

## Teknoloji Yığını
* **Framework:** Qt 6.x
* **Dil:** C++17
* **Optimizasyon:** OpenMP
* **Donanım Uyumluluğu:** ESP32, Art-Net ve DMX entegrasyonu için uygun mimari.
