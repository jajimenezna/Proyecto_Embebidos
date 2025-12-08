# Flujo de Ejecución del Sistema

## Secuencia de Inicio

1. Boot de MaixCAM
2. Sistema ejecuta `/etc/init.d/S06maixapp`
3. Script lanza `python3 /maixapp/apps/menu_selector/main.py`
4. Pantalla muestra menú con botón "DETECTOR DE SOMNOLENCIA"
5. Usuario presiona botón
6. Sistema ejecuta `os.execv()` hacia `/maixapp/apps/drowsiness_detector/main.py`

## Inicialización del Detector

### Capa Python
```python
lib = ctypes.CDLL('/root/libdrowsiness.so')
cam = camera.Camera(640, 480, image.Format.FMT_RGB888)
disp = display.Display()
ts = touchscreen.TouchScreen()
detector = nn.YOLOv8(model="/root/models/yolov8n_face.mud")
landmark_detector = nn.FaceLandmarks(model="/root/models/face_landmarks.mud")
```

### Capa C
La librería se carga en memoria. Variables estáticas globales se inicializan a cero.

## Loop Principal (30 FPS)

### Frame N (procesado)

**1. Captura**
```python
img = cam.read()
```

**2. Detección de rostro (GPU)**
```python
objs = detector.detect(img, conf_th=0.5, iou_th=0.45)
b = objs[0]
```

**3. Extracción de landmarks (TPU)**
```python
face = img.crop(b.x, b.y, b.w, b.h)
face_resized = face.resize(256, 256)
lm = landmark_detector.detect(face_resized, 0.5, True, False)
```

**4. Preparación de datos**
```python
left_eye = [(x, y) for puntos en LEFT_EYE]
right_eye = [(x, y) for puntos en RIGHT_EYE]
mouth = [(x, y) for puntos en MOUTH]
```

**5. Llamada a C**
```python
lib.analyze_drowsiness(left_arr, 6, right_arr, 6, mouth_arr, 4, ctypes.byref(metrics))
```

**6. Análisis en C**
```c
ear_avg = (calculate_ear(left) + calculate_ear(right)) / 2.0f
mar = calculate_mar(mouth)
eyes_closed = (ear_avg < 0.18f)

if (eyes_closed) {
    duration = time_now - eyes_closed_start
    if (duration > 2.5f) microsleep_count++
    else if (duration > 1.0f) long_blinks++
}

if (mar > 0.70f) yawn_count++

drowsiness_level += increments
drowsiness_level -= decay

if (microsleep_count >= 3 || drowsiness_level >= 80) danger_level = 2
else if (long_blinks >= 5 || yawn_count >= 3 || drowsiness_level >= 60) danger_level = 1
else danger_level = 0
```

**7. Renderizado**
```python
img.draw_rect(cara)
img.draw_circle(landmarks)
img.draw_string(metricas)
if danger_level > 0:
    img.draw_rect(alerta)
    img.draw_string(texto_alerta)
```

**8. Display**
```python
disp.show(img)
```

### Frame N+1 (skip)

Reutiliza resultado del frame anterior. Solo ejecuta captura, renderizado y display.

## Gestión de Eventos Touch
```python
data = ts.read()
if data[2] == 1:
    if boton_volver:
        os.execv("/usr/bin/python3", ["python3", "/maixapp/apps/menu_selector/main.py"])
    elif boton_reset:
        lib.reset_drowsiness()
```

## Máquina de Estados
```
SEGURO (0-59%) --> CANSANCIO (60-79%) --> PELIGRO (80-100%)
     ^                  ^                      |
     |                  |                      |
     +------------------+----------------------+
              RESET (usuario)
```

## Terminación

Usuario presiona VOLVER:
1. `del cam`
2. `del disp`
3. `del ts`
4. `os.execv()` hacia menú selector
