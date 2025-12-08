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
