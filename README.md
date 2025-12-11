# Detector de somnolencia en MaixCAM

Proyecto de sistemas embebidos para detectar somnolencia en un conductor usando una MaixCAM Pro (LicheeRV Nano / SG2002, RISC-V).

La cámara captura el rostro, se calculan métricas de ojos y boca y se estima un nivel de peligro. La parte crítica está en C (librería compartida) y Python se usa para integrar cámara, modelos y pantalla táctil.

**Presentado por:**
*Jaime A. Jiménez Naranjo, Miguel Alejandro Carvajal Medina, Daniel Sierra Peña*

## Contexto y motivación

La somnolencia al volante es un problema frecuente en conductores que manejan durante trayectos largos o en horarios de madrugada. No siempre es fácil para la persona darse cuenta de que está entrando en un estado de fatiga, y cuando reacciona muchas veces ya es tarde.

La idea de este proyecto es aprovechar la MaixCAM Pro como plataforma embebida de bajo consumo para monitorear en tiempo real el rostro del conductor. A partir de la apertura de los ojos y de la boca, el sistema identifica patrones de parpadeo, microsueños y bostezos que se asocian con somnolencia. La meta no es solo “detectar una cara”, sino dar una señal clara de estado (normal, alerta o peligro) que pueda usarse para activar avisos sonoros, visuales o hápticos.

## Primera etapa:

## Compilación del SDK y generación de la imagen para MaixCAM Pro (SG2002)

Para garantizar compatibilidad completa con el hardware SG2002 y asegurar un entorno limpio donde correr la aplicación de detección de somnolencia, se reconstruyó el sistema de la MaixCAM Pro usando el SDK oficial de Sophgo/Sipeed. Este proceso generó un kernel actualizado, un rootfs funcional y una imagen final lista para flashear.

---

### 1. Clonar el SDK de Sophgo/Sipeed

    git clone https://github.com/sophgo/sophgo-sdk
    cd sophgo-sdk

---

### 2. Configurar la plataforma SG2002 (MaixCAM)

    ./build.sh lunch

En el menú interactivo seleccionar:

    sg2002_maixcam

---

### 3. Compilar el kernel

    ./build.sh kernel

Esto genera los archivos del kernel, device tree y módulos asociados.

---

### 4. Construir el root filesystem (Buildroot)

    ./build.sh buildroot

El rootfs resultante incluye Python3, controladores multimedia y dependencias necesarias para ejecutar modelos .mud y la librería en C.

---

### 5. Generar la imagen final

    ./build.sh pack

Los archivos producidos quedan en:

    output/sg2002_maixcam/

Contenido típico esperado:

    full_image.img   → imagen completa para microSD
    boot.sd          → partición de arranque
    rootfs.sd        → sistema de archivos
    kernel.bin       → kernel compilado
    modules.tar.gz   → módulos del kernel

---

### 6. Flashear la imagen a la microSD

    sudo dd if=full_image.img of=/dev/sdX bs=4M status=progress
    sync

---

### 7. Verificación posterior al flasheo

Una vez insertada la tarjeta en la MaixCAM, se verificó:

- Kernel cargando correctamente (`uname -a`)
- Controladores ISP / VIC / CVITEK funcionando
- Python3 disponible
- GPIO expuestos en `/sys/class/gpio`
- Cámara CSI activa y usable desde el programa

Este procedimiento asegura que la aplicación corra sobre una base limpia y estable, alineada con la plataforma SG2002.


## Arquitectura

- MaixCAM Pro con Linux 5.10.
- Modelos .mud:
  - retinaface.mud (detección de rostro + 5 puntos).
  - face_landmarks.mud (landmarks faciales densos).
- Librería C libdrowsiness.so:
  - Cálculo de EAR (ojos) y MAR (boca).
  - Contadores de parpadeos y bostezos.
  - Nivel de peligro: 0 (normal), 1 (alerta), 2 (peligro).
- Script Python detector_final_hybrid.py:
  - Captura video.
  - Ejecuta modelos.
  - Llama a la librería C.
  - Dibuja overlay y maneja la pantalla táctil.

## Estructura del repositorio

Proyecto_Embebidos/
- src/                  Código C (drowsiness_hybrid_final.c)
- include/              Headers (si aplica)
- python/               Scripts Python (detector_final_hybrid.py)
- deployment/           Scripts para la MaixCAM (menu_selector_main.py)
- docs/                 Resumen, instrucciones y diagramas
- libs_maixcam/         Librerías/headers de MaixCAM (si se usan)
- CMakeLists.txt
- README.md

La carpeta build/ y los binarios generados (.o, .so) no se incluyen en el repositorio.

## Compilación de la librería C

Desde la raíz del proyecto:

1. Crear carpeta de compilación:
   - mkdir -p build
   - cd build

2. Generar archivos de compilación:
   - cmake ..

3. Compilar:
   - make

La salida principal es el archivo libdrowsiness.so.

Para usarla en la cámara, debe compilarse con un compilador RISC-V (dentro del SDK de MaixCAM o con un toolchain RISC-V instalado).

## Despliegue en la MaixCAM

Suponiendo que la cámara está accesible como root@CAM_IP:

- Librería C:
  scp build/libdrowsiness.so root@CAM_IP:/root/libdrowsiness.so

- Script Python principal:
  scp python/detector_final_hybrid.py root@CAM_IP:/root/

- Menú selector:
  scp deployment/menu_selector_main.py root@CAM_IP:/maixapp/apps/menu_selector/main.py

Los modelos .mud deben estar en la cámara, por ejemplo en:
- /root/models/retinaface.mud
- /root/models/face_landmarks.mud

## Uso

### Desde el menú táctil

1. Encender la MaixCAM.
2. Aparece el menú del proyecto.
3. Tocar el rectángulo central para iniciar el detector.
4. En pantalla se muestra:
   - Recuadro del rostro.
   - Estado, EAR, MAR, parpadeos y bostezos en la esquina superior derecha.
   - Botón "VOLVER" arriba a la izquierda para regresar al menú.

### Desde consola (SSH)

- ssh root@CAM_IP
- python3 /root/detector_final_hybrid.py

## Notas

- Los umbrales de EAR/MAR se fijaron tomando como referencia rangos típicos reportados en la literatura para ojos abiertos/cerrados y boca cerrada/abierta, y se verificaron con pruebas sobre el sistema.
- La calidad de la detección depende de la iluminación y del uso de gafas.
- Los GPIO están previstos para buzzer, LEDs y actuador háptico, pero el cableado depende del montaje físico.
