# Resumen del proyecto

## 1. Contexto

La somnolencia al volante es un factor importante en la accidentalidad vial. Este proyecto implementa un detector de somnolencia de bajo costo usando una MaixCAM Pro con procesador RISC-V.

## 2. Objetivo

Detectar en tiempo real signos de somnolencia en el rostro del conductor mediante:

- Parpadeos y ojos cerrados prolongados (EAR).
- Bostezos (MAR).
- Un nivel de peligro que pueda activar avisos al conductor.

La lógica principal se implementa en C (librería compartida) y Python se usa solo para integrar cámara, modelos y pantalla táctil.

## 3. Arquitectura

- MaixCAM Pro con Linux.
- Modelos .mud:
  - `retinaface.mud` para detección de rostro.
  - `face_landmarks.mud` para landmarks faciales.
- Librería C `libdrowsiness.so`:
  - Cálculo de EAR y MAR.
  - Contadores de parpadeos y bostezos.
  - Nivel de peligro (0, 1, 2).
- Script Python `detector_final_hybrid.py`:
  - Captura video.
  - Ejecuta modelos.
  - Llama a la librería C.
  - Dibuja overlay y maneja la pantalla táctil.

## 4. Flujo básico

1. La cámara entrega un frame.
2. RetinaFace detecta el rostro y genera un recorte.
3. El modelo de landmarks densos obtiene puntos de ojos y boca.
4. Python pasa estos puntos a `libdrowsiness.so`.
5. La librería C calcula EAR y MAR, detecta eventos y devuelve el nivel de peligro.
6. Python actualiza el overlay y los contadores visibles.

## 5. Resultados

El sistema:

- Detecta parpadeos normales y ráfagas de parpadeo.
- Marca ojos cerrados durante varios frames como posible microsueño.
- Cuenta bostezos amplios de forma consistente.
- Mantiene un desempeño adecuado para uso interactivo en la MaixCAM.

## 6. Limitaciones y trabajo futuro

- Umbrales de EAR/MAR ajustados de forma empírica con pocas personas.
- Sensible a cambios fuertes de iluminación y a gafas oscuras.
- Posibles mejoras:
  - Calibración por usuario.
  - Registro de eventos en archivos de log.
  - Integración completa con buzzer, LEDs y actuadores hápticos mediante GPIO.
