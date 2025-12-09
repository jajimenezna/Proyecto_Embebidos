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

Implementar un sistema de detección de somnolencia en tiempo real con el algoritmo completo en C puro, minimizando el uso de Python al acceso estrictamente necesario de hardware propietario.

## Distribución del Código

### Balance Final
- **C:** 366 líneas (80%)
- **Python:** 91 líneas (20%)

### Optimización Realizada
- **Versión inicial:** Python 53% (207 líneas)
- **Versión optimizada:** Python 20% (91 líneas)
- **Reducción:** 56% menos código Python

### Código C (366 líneas)
Implementación completa del algoritmo:
- `calculate_ear()` - Eye Aspect Ratio
- `calculate_mar()` - Mouth Aspect Ratio
- `analyze_drowsiness()` - Análisis completo
- `generate_render_commands()` - Comandos de visualización
- `reset_drowsiness()` - Reinicio del sistema

### Código Python (91 líneas)
Wrapper mínimo inevitable:
- Inicialización hardware Cvitek (20 líneas)
- Ejecución YOLO/Landmarks en TPU (30 líneas)
- Interfaz ctypes con C (15 líneas)
- Renderizado básico (26 líneas)

## Resultados

Sistema funcional que:
- Detecta parpadeos, microsueños y bostezos
- Calcula nivel de somnolencia (0-100%)
- Emite alertas graduales en tiempo real
- Latencia < 70ms

## Conclusión

Se logró minimizar el código Python al 20%, manteniendo el 80% del proyecto en C puro. El algoritmo completo permanece en C, usando Python únicamente para el acceso inevitable a hardware propietario sin drivers estándar de Linux.
