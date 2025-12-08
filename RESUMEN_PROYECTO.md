# Resumen del Proyecto

## Información General

**Título:** Detector de Somnolencia para Conductores  
**Autores:**
- Jaime Alfonso Jiménez Naranjo
- Miguel Alejandro Carvajal Medina
- Daniel Sierra Peña

**Institución:** Universidad Nacional de Colombia - Sede Bogotá  
**Asignatura:** Sistemas Embebidos  
**Año:** 2025

## Objetivo

Desarrollar un sistema embebido de detección de somnolencia en tiempo real para aplicaciones de seguridad vehicular, implementando el núcleo algorítmico en C puro sobre plataforma RISC-V.

## Hardware Utilizado

- **Plataforma:** MaixCAM
- **SoC:** Sophgo SG2002
- **Arquitectura:** RISC-V C906 @ 1GHz
- **Acelerador:** TPU 1 TOPS INT8
- **Memoria:** 256MB DDR3
- **Cámara:** Sensor GC4653 (640x480 @ 30 FPS)
- **Display:** LCD 480x640
- **Interfaz:** Touchscreen capacitivo

## Tecnologías Implementadas

### Núcleo en C (71%)
- Análisis de Eye Aspect Ratio (EAR)
- Análisis de Mouth Aspect Ratio (MAR)
- Clasificación de eventos (parpadeos, microsueños, bostezos)
- Sistema de análisis temporal
- Máquina de estados de somnolencia
- Evaluación de nivel de peligro

### Interfaz de Hardware en Python (29%)
- Acceso a subsistemas Cvitek propietarios
- Detección de rostros (YOLO en TPU)
- Extracción de landmarks faciales (TPU)
- Renderizado y visualización

## Métricas de Rendimiento

- **Latencia total:** < 70ms
- **FPS de análisis:** 15 FPS
- **FPS de display:** 30 FPS
- **Tamaño de librería:** 7.9 KB

## Resultados

Sistema funcional capaz de:
- Detectar parpadeos normales, largos y microsueños
- Identificar bostezos
- Calcular nivel de somnolencia (0-100%)
- Emitir alertas graduales (SEGURO / CANSANCIO / PELIGRO)
- Operar en tiempo real sin latencia perceptible

## Compilación

La librería C se compila mediante:
```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-riscv64.cmake ..
make -j4
```

Genera: `libdrowsiness.so` (ELF 64-bit RISC-V)

## Deployment

Sistema de instalación automatizada mediante `deployment/install.sh` que:
1. Transfiere librería compilada
2. Copia aplicaciones Python
3. Configura auto-inicio
4. Reinicia el sistema

## Documentación

- `docs/JUSTIFICACION_TECNICA.md`: Análisis de arquitectura híbrida
- `docs/FLUJO_EJECUCION.md`: Secuencia detallada de operación
- `docs/diagramas/DIAGRAMAS_BLOQUES.md`: Diagramas del sistema
- `README.md`: Guía de usuario y compilación

## Estructura del Código
```
src/drowsiness_c.c
├── calculate_ear()         - Cálculo Eye Aspect Ratio
├── calculate_mar()         - Cálculo Mouth Aspect Ratio
├── analyze_drowsiness()    - Función principal de análisis
└── reset_drowsiness()      - Reinicio de estado
```

## Conclusiones

Se logró implementar un sistema funcional de detección de somnolencia con núcleo algorítmico íntegramente en C puro, demostrando viabilidad técnica de análisis en tiempo real sobre arquitectura RISC-V. La arquitectura híbrida Python-C responde a restricciones del hardware, manteniendo el algoritmo crítico en C de acuerdo a los objetivos del proyecto.
