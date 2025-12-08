# Instrucciones para Evaluación

## Contenido del Repositorio

Este repositorio contiene el proyecto completo "Detector de Somnolencia para Conductores" desarrollado como proyecto final de la asignatura Sistemas Embebidos.

## Autores

- Jaime Alfonso Jiménez Naranjo
- Miguel Alejandro Carvajal Medina
- Daniel Sierra Peña

Universidad Nacional de Colombia - Sede Bogotá, 2025

## Estructura del Proyecto
```
Proyecto_Embebidos/
├── src/drowsiness_c.c              Núcleo algorítmico en C puro
├── python/                         Interfaz de hardware
│   ├── drowsiness_detector/
│   └── menu_selector/
├── docs/                           Documentación técnica
│   ├── JUSTIFICACION_TECNICA.md
│   ├── FLUJO_EJECUCION.md
│   └── diagramas/
├── deployment/                     Scripts de instalación
├── CMakeLists.txt                  Configuración de compilación
├── toolchain-riscv64.cmake         Toolchain RISC-V
├── README.md                       Documentación principal
├── RESUMEN_PROYECTO.md             Resumen ejecutivo
└── LICENSE                         Licencia MIT
```

## Verificación del Repositorio

### Extraer archivos
```bash
tar -xzf Proyecto_Embebidos.tar.gz
cd Proyecto_Embebidos
```

### Ver historial de commits
```bash
git log --oneline --all
```

### Ver archivos
```bash
find . -type f ! -path "./.git/*" | sort
```

## Compilación del Proyecto

### Prerrequisitos
- Toolchain RISC-V instalado en `~/Downloads/maixcam_toolchain/`
- CMake 3.10 o superior

### Proceso de compilación
```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-riscv64.cmake ..
make -j4
```

### Verificar salida
```bash
file libdrowsiness.so
```

Debe mostrar: `ELF 64-bit LSB shared object, UCB RISC-V`

## Documentos Clave para Evaluación

1. **RESUMEN_PROYECTO.md**: Visión general del proyecto
2. **docs/JUSTIFICACION_TECNICA.md**: Arquitectura y decisiones técnicas
3. **docs/FLUJO_EJECUCION.md**: Funcionamiento detallado del sistema
4. **docs/diagramas/DIAGRAMAS_BLOQUES.md**: Diagramas de arquitectura
5. **src/drowsiness_c.c**: Código fuente del núcleo en C
6. **README.md**: Guía completa de usuario

## Puntos de Evaluación Sugeridos

### Implementación en C
- Núcleo algorítmico completo en C puro
- Funciones de análisis geométrico (EAR, MAR)
- Sistema de análisis temporal
- Máquina de estados
- Sin dependencias externas excepto libm

### Arquitectura
- Separación clara de responsabilidades
- Interfaz bien definida entre Python y C (ctypes)
- Justificación documentada de decisiones arquitectónicas

### Compilación Cruzada
- Toolchain RISC-V configurado correctamente
- CMake con configuración cross-compilation
- Librería compartida generada exitosamente

### Documentación
- Documentación técnica completa
- Diagramas de arquitectura
- Justificación de decisiones técnicas
- Instrucciones de compilación y deployment

## Contacto

Para dudas sobre el proyecto contactar a:
- jjimenezn@unal.edu.co

---

**Nota:** La librería compilada no está incluida en el repositorio Git debido a `.gitignore`, pero puede generarse siguiendo las instrucciones de compilación.
