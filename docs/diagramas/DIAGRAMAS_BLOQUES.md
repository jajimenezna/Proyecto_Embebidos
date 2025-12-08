# Diagramas de Bloques del Sistema

## Arquitectura General
```
┌─────────────────────────────────────────┐
│       HARDWARE MAIXCAM SG2002           │
│  ┌────────┐  ┌────────┐  ┌────────┐    │
│  │ Cámara │  │Display │  │ Touch  │    │
│  │640x480 │  │  LCD   │  │ Screen │    │
│  └───┬────┘  └───▲────┘  └───▲────┘    │
│      │           │           │          │
│  ┌───▼───────────┴───────────┴────┐    │
│  │   Subsistemas Cvitek           │    │
│  │   ISP | TPU | VO               │    │
│  └────────────┬───────────────────┘    │
└───────────────┼────────────────────────┘
                │ APIs Propietarias
                ▼
┌────────────────────────────────────────┐
│        PYTHON (29%)                    │
│  ┌──────────────────────────────┐     │
│  │ Hardware Interface           │     │
│  │ camera.Camera()              │     │
│  │ display.Display()            │     │
│  │ touchscreen.TouchScreen()    │     │
│  └──────────────────────────────┘     │
│  ┌──────────────────────────────┐     │
│  │ AI Detection (GPU/TPU)       │     │
│  │ nn.YOLOv8()                  │     │
│  │ nn.FaceLandmarks()           │     │
│  └──────────────────────────────┘     │
└───────────────┬────────────────────────┘
                │ ctypes
                ▼
┌────────────────────────────────────────┐
│         C PURO (71%)                   │
│  ┌──────────────────────────────┐     │
│  │ Cálculos Geométricos         │     │
│  │ calculate_ear()              │     │
│  │ calculate_mar()              │     │
│  │ distance()                   │     │
│  └──────────────────────────────┘     │
│  ┌──────────────────────────────┐     │
│  │ Análisis Temporal            │     │
│  │ Tracking de eventos          │     │
│  │ Sistema de decaimiento       │     │
│  │ Contadores BPM               │     │
│  └──────────────────────────────┘     │
│  ┌──────────────────────────────┐     │
│  │ Evaluación de Peligro        │     │
│  │ Nivel 0: SEGURO              │     │
│  │ Nivel 1: CANSANCIO           │     │
│  │ Nivel 2: PELIGRO             │     │
│  └──────────────────────────────┘     │
└───────────────┬────────────────────────┘
                │ DrowsinessMetrics
                ▼
┌────────────────────────────────────────┐
│        VISUALIZACIÓN                   │
│  Renderizado de métricas y alertas     │
└────────────────────────────────────────┘
```

## Flujo de Datos
```
Cámara (640x480 @ 30 FPS)
    │
    ▼
ISP + VI (Procesamiento hardware)
    │
    ▼
YOLO TPU (Detección rostro: x,y,w,h)
    │
    ▼
Landmarks TPU (478 puntos faciales)
    │
    ▼
Extractor Python (10 landmarks: 6 ojos + 4 boca)
    │
    ▼
Conversión ctypes (arrays float)
    │
    ▼
┌─────────────────────────────────┐
│ analyze_drowsiness() - C        │
│                                 │
│ calculate_ear() → ear_avg       │
│ calculate_mar() → mar           │
│         ↓                       │
│ Clasificar Eventos              │
│  - Parpadeo Normal              │
│  - Parpadeo Largo               │
│  - Microsueño                   │
│  - Bostezo                      │
│         ↓                       │
│ Acumular drowsiness_level       │
│         ↓                       │
│ Evaluar danger_level            │
└─────────────┬───────────────────┘
              │
              ▼
    DrowsinessMetrics (struct)
              │
              ▼
    Renderizado Python
              │
              ▼
        Display LCD
```

## Máquina de Estados
```
        ┌─────────────┐
        │   INICIO    │
        │drowsiness=0%│
        └──────┬──────┘
               │
               ▼
    ┌──────────────────────┐
    │  SEGURO (0-59%)      │
    │  danger_level = 0    │
    └──┬────────────────┬──┘
       │                │
       │                │ 5+ largos O 3+ bostezos
       │                │ O drowsiness >= 60%
       │                ▼
       │    ┌──────────────────────┐
       │    │ CANSANCIO (60-79%)   │
       │    │ danger_level = 1     │
       │    └──┬────────────────┬──┘
       │       │                │
       │       │                │ 3+ microsueños
       │       │                │ O drowsiness >= 80%
       │       │                ▼
       │       │    ┌──────────────────────┐
       │       │    │ PELIGRO (80-100%)    │
       │       │    │ danger_level = 2     │
       │       │    └──────────┬───────────┘
       │       │               │
       │       │               │ Usuario RESET
       │       │               │
       └───────┴───────────────┘
```

## Temporización
```
Tiempo    0ms     33ms    66ms    100ms   133ms
Frame     1       2       3       4       5

Cámara    █       █       █       █       █
YOLO      ███             ███             ███
Landmark  ████            ████            ████
C         █               █               █
Render    ██      ██      ██      ██      ██
Display   █       █       █       █       █

Estrategia: Procesar frames alternos
Análisis: 15 FPS
Display: 30 FPS
Latencia: < 70ms
```
