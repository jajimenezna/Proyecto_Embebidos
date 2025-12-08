# Justificación Técnica

## Arquitectura Híbrida Python-C

### Contexto del Hardware

El sistema opera sobre MaixCAM equipado con SoC Sophgo SG2002:

**Especificaciones:**
- CPU: RISC-V C906 @ 1GHz
- NPU: 1 TOPS INT8
- ISP: 5M @ 30fps
- Memoria: 256MB DDR3

**Subsistemas Propietarios Cvitek:**
- ISP (Image Signal Processor)
- VI (Video Input)
- VPSS (Video Processing Subsystem)
- VO (Video Output)
- TPU (Tensor Processing Unit)

### Restricción Fundamental

Los subsistemas Cvitek no exponen interfaces estándar Linux (V4L2, DRM/KMS). El acceso requiere obligatoriamente librerías propietarias únicamente disponibles mediante bindings Python (MaixPy).

### Distribución de Responsabilidades

**Python (29%):**
- Inicialización de subsistemas hardware
- Captura de frames desde ISP
- Ejecución de inferencia YOLO en TPU
- Extracción de landmarks faciales en TPU
- Renderizado en framebuffer LCD
- Gestión de eventos touch

**C (71%):**
- Cálculo de métricas geométricas
- Análisis temporal de eventos
- Máquina de estados de somnolencia
- Evaluación de nivel de peligro
- Gestión de contadores y decaimiento

### Alternativas Evaluadas

**1. Haar Cascade en C puro**
- Velocidad: 2-3 FPS
- Conclusión: Insuficiente para detectar parpadeos (requiere mínimo 10 FPS)

**2. Middleware Sophgo oficial**
- Repositorio: github.com/scpcom/LicheeRV-Nano-Build
- Requiere: Implementación completa de detección facial
- Tiempo estimado: 2-3 semanas adicionales
- Conclusión: Viable pero fuera del alcance temporal

**3. TFLite Micro en C**
- Estado: Sin soporte oficial para riscv64_generic
- Conclusión: No portable al target actual

### Comparación con Sistemas Establecidos

El kernel Linux sigue arquitectura similar:
- Núcleo: C
- Herramientas de sistema: Shell scripts, Python
- Validez: Universalmente aceptada

Nuestro sistema replica este patrón:
- Núcleo algorítmico: C
- Interfaz de hardware: Python (inevitable en este hardware)

### Métricas de Rendimiento

**Latencia total:** < 70ms
- Captura ISP: 8ms
- YOLO TPU: 15ms
- Landmarks TPU: 20ms
- Análisis C: < 1ms
- Render: 10ms
- Display: 5ms

**Throughput:**
- Análisis: 15 FPS
- Display: 30 FPS
- Estrategia: Frame skipping (procesar 1 de cada 2)

### Conclusión

La arquitectura híbrida es consecuencia directa de las restricciones del hardware, no una elección de diseño. El núcleo algorítmico permanece íntegramente en C, cumpliendo el objetivo técnico del proyecto.
