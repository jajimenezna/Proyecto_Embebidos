# Detector de Somnolencia para Conductores

Sistema de detección de somnolencia en tiempo real con núcleo algorítmico en C puro, implementado sobre hardware MaixCAM (RISC-V 64-bit).

## Autores

- Jaime Alfonso Jiménez Naranjo
- Miguel Alejandro Carvajal Medina
- Daniel Sierra Peña

**Universidad Nacional de Colombia - Sede Bogotá**
**Asignatura:** Sistemas Embebidos
**Año:** 2025

## Arquitectura

El sistema se estructura en tres capas:

### Hardware (MaixCAM SG2002)
- Procesador: RISC-V 64-bit @ 1GHz
- Cámara: GC4653 (640x480 @ 30 FPS)
- TPU: Acelerador neural
- Display: LCD 480x640

### Capa de Hardware (Python - 29%)
Responsabilidad: Acceso a subsistemas propietarios Cvitek

### Núcleo Algorítmico (C Puro - 71%)
Responsabilidad: Análisis completo de somnolencia

## Compilación
```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-riscv64.cmake ..
make -j4
```

## Deployment
```bash
cd deployment
./install.sh
```

## Licencia

MIT License

## Optimización del Código

### Versión Final
- **C:** 366 líneas (80%)
  - Algoritmo completo de somnolencia: 187 líneas
  - Funciones extendidas de análisis: 179 líneas
- **Python:** 91 líneas (20%)
  - Wrapper mínimo de hardware propietario: 64 líneas
  - Menú de interfaz: 27 líneas

### Python Minimizado
El código Python se redujo al mínimo indispensable:
- Inicialización de hardware (cámara, display, TPU)
- Ejecución de YOLO y Landmarks en TPU
- Paso de datos a funciones C
- Renderizado básico de resultados

**Todo el algoritmo inteligente permanece en C.**
