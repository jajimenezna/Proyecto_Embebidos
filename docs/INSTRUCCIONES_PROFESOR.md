# Instrucciones para probar el sistema

## 1. Arranque

1. Conecte la MaixCAM Pro a la alimentación.
2. Tras el arranque, aparece el menú del proyecto con el título:
   - "SISTEMA DE DETECCION – DETECTOR DE SOMNOLENCIA".

## 2. Iniciar el detector

1. Toque el rectángulo central de la pantalla.
2. La cámara cambia a la vista del detector:
   - Imagen en vivo.
   - Recuadro sobre el rostro detectado.

## 3. Elementos en pantalla

- Recuadro sobre el rostro.
- Esquina superior derecha:
  - Estado: NORMAL / ALERTA / PELIGRO.
  - EAR y MAR.
  - Contador de parpadeos.
  - Contador de bostezos.
- Esquina superior izquierda:
  - Botón "VOLVER" para regresar al menú inicial.

## 4. Cómo forzar situaciones

- **Estado normal**  
  Mirar a la cámara y parpadear de forma natural. El estado suele ser NORMAL.

- **Parpadeos frecuentes**  
  Parpadear varias veces seguidas. El contador de parpadeos sube y el estado puede pasar a ALERTA.

- **Microsueño**  
  Cerrar los ojos durante más de uno o dos segundos. El sistema tiende a marcar PELIGRO.

- **Bostezos**  
  Abrir la boca ampliamente, simulando un bostezo real. El contador de bostezos aumenta.

## 5. Volver al menú

- Tocar el botón "VOLVER" en la parte superior izquierda.
- La MaixCAM regresa al menú de inicio del proyecto.

## 6. Nota técnica

- La lógica de somnolencia (EAR, MAR, contadores y nivel de peligro) está implementada en C en `libdrowsiness.so`.
- Python se encarga de:
  - Capturar video.
  - Ejecutar los modelos .mud.
  - Mostrar la interfaz en pantalla.
  - Leer la pantalla táctil y gestionar el botón "VOLVER".
