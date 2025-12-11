#!/usr/bin/env python3
import ctypes
import os
from maix import camera, display, image, nn, touchscreen, app

# Config libreria C
lib = ctypes.CDLL("/root/libdrowsiness.so")

lib.drowsiness_init.argtypes = []
lib.drowsiness_init.restype = ctypes.c_int

lib.process_hybrid.argtypes = [
    ctypes.POINTER(ctypes.c_float),
    ctypes.POINTER(ctypes.c_float),
    ctypes.POINTER(ctypes.c_float),
    ctypes.POINTER(ctypes.c_float),
]
lib.process_hybrid.restype = ctypes.c_int

lib.drowsiness_get_blink_count.argtypes = []
lib.drowsiness_get_blink_count.restype = ctypes.c_int

lib.drowsiness_get_yawn_count.argtypes = []
lib.drowsiness_get_yawn_count.restype = ctypes.c_int

lib.drowsiness_cleanup.argtypes = []
lib.drowsiness_cleanup.restype = None

print("=" * 60)
print("DETECTOR DE SOMNOLENCIA - MAIXCAM")
print("=" * 60)

ret = lib.drowsiness_init()
if ret != 0:
    print(f"Error al inicializar libdrowsiness.so (codigo {ret})")

# Dispositivos MaixCAM
cam = camera.Camera(640, 480)
disp = display.Display()
ts = touchscreen.TouchScreen()

# Modelos .mud
retinaface = nn.Retinaface(model="/root/models/retinaface.mud")
mediapipe = nn.FaceLandmarks(model="/root/models/face_landmarks.mud")

green = image.Color.from_rgb(0, 255, 0)
yellow = image.Color.from_rgb(255, 255, 0)
red = image.Color.from_rgb(255, 0, 0)
white = image.Color.from_rgb(255, 255, 255)

btn_bg = image.Color.from_rgb(40, 40, 40)
btn_border = image.Color.from_rgb(255, 255, 255)

# Boton VOLVER
BACK_X1, BACK_Y1 = 10, 10
BACK_X2, BACK_Y2 = 180, 60

print("Sistema listo\n")

frame_count = 0
IMG_W, IMG_H = 640, 480

try:
    while not app.need_exit():
        img = cam.read()

        # Deteccion de rostro
        faces = retinaface.detect(img, conf_th=0.5)

        if faces and len(faces) > 0:
            face = faces[0]
            x, y, w, h = face.x, face.y, face.w, face.h

            x = max(0, x)
            y = max(0, y)
            w = min(w, IMG_W - x)
            h = min(h, IMG_H - y)

            if (
                w > 20
                and h > 20
                and hasattr(face, "points")
                and face.points
                and len(face.points) >= 10
            ):
                retinaface_lm = (ctypes.c_float * 10)()
                for i in range(10):
                    retinaface_lm[i] = float(face.points[i])

                face_img = img.crop(x, y, w, h)
                face_256 = face_img.resize(256, 256)

                lm_obj = mediapipe.detect(face_256, conf_th=0.5)

                if lm_obj and lm_obj.points and len(lm_obj.points) >= 956:
                    points = lm_obj.points

                    mediapipe_lm = (ctypes.c_float * 956)()
                    for i in range(956):
                        mediapipe_lm[i] = float(points[i])

                    ear_val = ctypes.c_float()
                    mar_val = ctypes.c_float()

                    danger = lib.process_hybrid(
                        mediapipe_lm,
                        retinaface_lm,
                        ctypes.byref(ear_val),
                        ctypes.byref(mar_val),
                    )

                    ear = ear_val.value
                    mar = mar_val.value

                    if danger == 0:
                        color = green
                        status = "NORMAL"
                    elif danger == 1:
                        color = yellow
                        status = "ALERTA"
                    else:
                        color = red
                        status = "PELIGRO"

                    img.draw_rect(x, y, w, h, color=color, thickness=3)

                    blinks = lib.drowsiness_get_blink_count()
                    yawns = lib.drowsiness_get_yawn_count()

                    # Metricas arriba a la derecha
                    base_x = 440
                    base_y = 20
                    dy = 25

                    img.draw_string(base_x, base_y, f"Estado: {status}", color=color, scale=1.2)
                    img.draw_string(base_x, base_y + dy, f"EAR: {ear:.3f}", color=white, scale=1.0)
                    img.draw_string(base_x, base_y + 2 * dy, f"MAR: {mar:.3f}", color=white, scale=1.0)
                    img.draw_string(base_x, base_y + 3 * dy, f"Parpadeos: {blinks}", color=white, scale=1.0)
                    img.draw_string(base_x, base_y + 4 * dy, f"Bostezos: {yawns}", color=white, scale=1.0)

                    if frame_count % 30 == 0:
                        print(
                            f"EAR={ear:.3f} MAR={mar:.3f} "
                            f"Parp={blinks} Bost={yawns} Peligro={danger}"
                        )

        # Boton VOLVER
        btn_w = BACK_X2 - BACK_X1
        btn_h = BACK_Y2 - BACK_Y1
        img.draw_rect(BACK_X1, BACK_Y1, btn_w, btn_h, color=btn_bg, thickness=-1)
        img.draw_rect(BACK_X1, BACK_Y1, btn_w, btn_h, color=btn_border, thickness=2)
        img.draw_string(BACK_X1 + 40, BACK_Y1 + 14, "VOLVER", color=btn_border, scale=1.5)

        # Touch
        touch = ts.read()
        if touch and len(touch) == 3 and touch[2] == 1:
            tx, ty = touch[0], touch[1]
            if BACK_X1 <= tx <= BACK_X2 and BACK_Y1 <= ty <= BACK_Y2:
                print("Volver al menu")
                try:
                    lib.drowsiness_cleanup()
                except Exception:
                    pass
                try:
                    del cam, disp, retinaface, mediapipe, ts
                except Exception:
                    pass
                os.execv("/usr/bin/python3", ["python3", "/maixapp/apps/menu_selector/main.py"])

        disp.show(img)
        frame_count += 1

except KeyboardInterrupt:
    blinks = lib.drowsiness_get_blink_count()
    yawns = lib.drowsiness_get_yawn_count()
    print("\nFin por teclado")
    print(f"Parpadeos: {blinks}")
    print(f"Bostezos: {yawns}")
finally:
    try:
        lib.drowsiness_cleanup()
    except Exception:
        pass
