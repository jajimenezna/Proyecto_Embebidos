# Detector de Somnolencia para Conductores

Sistema de detección de somnolencia en tiempo real con algoritmo completo implementado en C puro sobre hardware MaixCAM (RISC-V 64-bit).

## Autores

- Jaime Alfonso Jiménez Naranjo
- Miguel Alejandro Carvajal Medina
- Daniel Sierra Peña

**Universidad Nacional de Colombia - Sede Bogotá**
**Asignatura:** Sistemas Embebidos
**Año:** 2025

## Distribución del Código

**Balance Final: C 80% | Python 20%**

### Algoritmo Completo en C (366 líneas - 80%)
**Archivo:** `src/drowsiness_c.c`

Implementación completa:
- Cálculo de Eye Aspect Ratio (EAR) y Mouth Aspect Ratio (MAR)
- Análisis geométrico de landmarks
- Clasificación de eventos (parpadeos normales, largos, microsueños, bostezos)
- Sistema de análisis temporal con contadores
- Evaluación de nivel de peligro con 3 estados
- Sistema de decaimiento progresivo
- Generación de comandos de renderizado

### Wrapper Mínimo de Hardware (91 líneas - 20%)
**Archivos:** `python/drowsiness_detector/main.py` (64 líneas), `python/menu_selector/main.py` (27 líneas)

Funciones exclusivas de acceso a hardware propietario:
- Inicialización de subsistemas Cvitek (ISP, TPU, VO)
- Captura de frames desde cámara
- Ejecución de YOLO y Landmarks en TPU (aceleración hardware)
- Paso de datos a funciones C mediante ctypes
- Renderizado básico en display

**Líneas de algoritmo en Python: 0**

## Justificación Técnica

### Hardware Propietario

El SoC Sophgo SG2002 utiliza subsistemas propietarios Cvitek sin drivers estándar V4L2. El acceso requiere APIs de MaixPy, haciendo inevitable el uso mínimo de Python.

### Optimización Realizada

El código Python se redujo al mínimo absoluto:
- **Antes:** 207 líneas (53%)
- **Después:** 91 líneas (20%)
- **Reducción:** 56% menos Python

Todo el procesamiento inteligente permanece en C puro.

## Arquitectura
```
Hardware (Cvitek) → Python (wrapper 20%) → C (algoritmo 80%) → Resultado
```

## Métricas de Detección

### Eye Aspect Ratio (EAR)
```
EAR = distancia_vertical / distancia_horizontal
```
Umbral: EAR < 0.18 indica ojos cerrados

### Mouth Aspect Ratio (MAR)
```
MAR = distancia_vertical / distancia_horizontal
```
Umbral: MAR > 0.70 indica bostezo

### Clasificación de Eventos

| Evento | Criterio | Acción |
|--------|----------|--------|
| Parpadeo Normal | < 1.0 seg | +2% somnolencia |
| Parpadeo Largo | 1.0-2.5 seg | +15% somnolencia |
| Microsueño | > 2.5 seg | +40% somnolencia |
| Bostezo | MAR > 0.70 | +10% somnolencia |

### Niveles de Alerta

- **Nivel 0 (0-59%):** Conducción segura
- **Nivel 1 (60-79%):** Descanso necesario
- **Nivel 2 (80-100%):** Detener vehículo

## Compilación
```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-riscv64.cmake ..
make -j4
```

**Salida:** `libdrowsiness.so` (ELF 64-bit RISC-V)

## Deployment
```bash
cd deployment
./install.sh
```

## Licencia

MIT License
