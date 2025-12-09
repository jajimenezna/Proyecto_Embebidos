#!/usr/bin/env python3
from maix import camera, display, image, app, touchscreen, nn
import ctypes
import os

try:
    # Carga de librería C
    lib = ctypes.CDLL('/root/libdrowsiness.so')
    
    # Definición de estructura de resultados
    class DrowsinessMetrics(ctypes.Structure):
        _fields_ = [
            ("ear_left", ctypes.c_float), ("ear_right", ctypes.c_float),
            ("ear_avg", ctypes.c_float), ("mar", ctypes.c_float),
            ("eyes_closed", ctypes.c_bool), ("mouth_state", ctypes.c_int),
            ("drowsiness_level", ctypes.c_float), ("blink_count", ctypes.c_int),
            ("eyes_closed_duration", ctypes.c_float),
            ("blinks_per_minute", ctypes.c_int),
            ("danger_level", ctypes.c_int),
            ("danger_text", ctypes.c_char * 100)
        ]
    
    # Configuración de tipos de argumentos
    lib.analyze_drowsiness.argtypes = [
        ctypes.POINTER(ctypes.c_float), ctypes.c_int,
        ctypes.POINTER(ctypes.c_float), ctypes.c_int,
        ctypes.POINTER(ctypes.c_float), ctypes.c_int,
        ctypes.POINTER(DrowsinessMetrics)
    ]
    lib.reset_drowsiness.argtypes = []
    
    # Inicialización de hardware
    cam = camera.Camera(640, 480, image.Format.FMT_RGB888)
    disp = display.Display()
    ts = touchscreen.TouchScreen()
    detector = nn.YOLOv8(model="/root/models/yolov8n_face.mud")
    landmark_detector = nn.FaceLandmarks(model="/root/models/face_landmarks.mud")
    
    # Índices de landmarks
    sub_146 = [0,1,4,5,6,7,8,10,13,14,17,21,33,37,39,40,46,52,53,54,55,58,61,63,65,66,67,70,78,80,81,82,84,87,88,91,93,95,103,105,107,109,127,132,133,136,144,145,146,148,149,150,152,153,154,155,157,158,159,160,161,162,163,168,172,173,176,178,181,185,191,195,197,234,246,249,251,263,267,269,270,276,282,283,284,285,288,291,293,295,296,297,300,308,310,311,312,314,317,318,321,323,324,332,334,336,338,356,361,362,365,373,374,375,377,378,379,380,381,382,384,385,386,387,388,389,390,397,398,400,402,405,409,415,454,466,468,469,470,471,472,473,474,475,476,477]
    LEFT_EYE = [33, 160, 158, 133, 153, 144]
    RIGHT_EYE = [263, 387, 385, 362, 380, 373]
    MOUTH = [61, 13, 14, 291]
    orig_to_sub = {o: i for i, o in enumerate(sub_146)}
    
    # Variables de estado
    last_metrics = None
    last_box = None
    last_landmarks = None
    frame_count = 0
    reset_btn_x, reset_btn_y, reset_btn_w, reset_btn_h = 540, 10, 90, 35
    
    while not app.need_exit():
        img = cam.read()
        frame_count += 1
        
        # Botones de control
        img.draw_rect(10, 10, 80, 35, image.Color.from_rgb(200,0,0), -1)
        img.draw_string(15, 23, "< VOLVER", image.Color.from_rgb(255,255,255), scale=0.8)
        img.draw_rect(reset_btn_x, reset_btn_y, reset_btn_w, reset_btn_h, image.Color.from_rgb(0,150,255), -1)
        img.draw_string(reset_btn_x+10, reset_btn_y+13, "RESET", image.Color.from_rgb(255,255,255), scale=0.8)
        
        # Procesamiento de frames alternos
        if frame_count % 2 == 0:
            objs = detector.detect(img, conf_th=0.5, iou_th=0.45)
            
            if objs:
                b = objs[0]
                last_box = (b.x, b.y, b.w, b.h)
                face = img.crop(b.x, b.y, b.w, b.h)
                
                if face:
                    face_resized = face.resize(256, 256)
                    lm = landmark_detector.detect(face_resized, 0.5, True, False)
                    
                    if lm and lm.valid and all(i in orig_to_sub for i in LEFT_EYE + RIGHT_EYE + MOUTH):
                        # Extracción de subset de landmarks
                        sub_xy = []
                        for idx in sub_146:
                            sub_xy.append(lm.points[idx * 2])
                            sub_xy.append(lm.points[idx * 2 + 1])
                        
                        pts_146 = [(sub_xy[i*2], sub_xy[i*2+1]) for i in range(len(sub_146))]
                        scale_x = b.w / 256.0
                        scale_y = b.h / 256.0
                        
                        # Escalado de coordenadas
                        left = [(pts_146[orig_to_sub[i]][0] * scale_x, pts_146[orig_to_sub[i]][1] * scale_y) for i in LEFT_EYE]
                        right = [(pts_146[orig_to_sub[i]][0] * scale_x, pts_146[orig_to_sub[i]][1] * scale_y) for i in RIGHT_EYE]
                        mouth_pts = [(pts_146[orig_to_sub[i]][0] * scale_x, pts_146[orig_to_sub[i]][1] * scale_y) for i in MOUTH]
                        
                        last_landmarks = {
                            'left': [(b.x + pt[0], b.y + pt[1]) for pt in left],
                            'right': [(b.x + pt[0], b.y + pt[1]) for pt in right],
                            'mouth': [(b.x + pt[0], b.y + pt[1]) for pt in mouth_pts]
                        }
                        
                        # Conversión a arrays C
                        left_arr = (ctypes.c_float * 12)(*[c for pt in left for c in pt])
                        right_arr = (ctypes.c_float * 12)(*[c for pt in right for c in pt])
                        mouth_arr = (ctypes.c_float * 8)(*[c for pt in mouth_pts for c in pt])
                        
                        # Llamada a función C
                        last_metrics = DrowsinessMetrics()
                        lib.analyze_drowsiness(left_arr, 6, right_arr, 6, mouth_arr, 4, ctypes.byref(last_metrics))
            else:
                last_box = None
                last_landmarks = None
        
        # Renderizado de detección
        if last_box:
            img.draw_rect(last_box[0], last_box[1], last_box[2], last_box[3], image.Color.from_rgb(0,255,0), 1)
        
        # Renderizado de landmarks
        if last_landmarks and last_metrics:
            eye_color = image.Color.from_rgb(255,0,0) if last_metrics.eyes_closed else image.Color.from_rgb(0,255,0)
            for pt in last_landmarks['left'] + last_landmarks['right']:
                img.draw_circle(int(pt[0]), int(pt[1]), 3, eye_color, -1)
            
            mouth_colors = [image.Color.from_rgb(0,255,0), image.Color.from_rgb(255,255,0), image.Color.from_rgb(255,0,0)]
            for pt in last_landmarks['mouth']:
                img.draw_circle(int(pt[0]), int(pt[1]), 3, mouth_colors[last_metrics.mouth_state], -1)
        
        # Renderizado de métricas
        if last_metrics:
            img.draw_string(10, 80, f"Parpadeos: {last_metrics.blink_count}", image.Color.from_rgb(255,255,255), scale=1.3)
            img.draw_string(10, 110, f"Por min: {last_metrics.blinks_per_minute}", image.Color.from_rgb(255,255,255), scale=1.3)
            img.draw_string(10, 140, f"EAR: {last_metrics.ear_avg:.2f}", image.Color.from_rgb(0,200,255), scale=1.2)
            
            # Alerta de peligro
            danger_colors = [image.Color.from_rgb(0,255,0), image.Color.from_rgb(255,200,0), image.Color.from_rgb(255,0,0)]
            danger_color = danger_colors[last_metrics.danger_level]
            danger_text = last_metrics.danger_text.decode()
            
            if last_metrics.danger_level > 0:
                img.draw_rect(150, 200, 350, 80, danger_color, -1)
                text_color = image.Color.from_rgb(0,0,0) if last_metrics.danger_level == 1 else image.Color.from_rgb(255,255,255)
                img.draw_string(160, 230, danger_text, text_color, scale=1.8)
        else:
            img.draw_string(10, 80, "Buscando cara...", image.Color.from_rgb(255,255,0), scale=1.3)
        
        img.draw_string(10, 460, "Core: C", image.Color.from_rgb(0,255,255), scale=0.6)
        disp.show(img)
        
        # Procesamiento de eventos táctiles
        data = ts.read()
        if data and len(data) >= 3 and data[2] == 1:
            x, y = data[0], data[1]
            if 10 <= x <= 90 and 10 <= y <= 45:
                del cam
                del disp
                del ts
                os.execv("/usr/bin/python3", ["python3", "/maixapp/apps/menu_selector/main.py"])
                break
            if reset_btn_x <= x <= reset_btn_x+reset_btn_w and reset_btn_y <= y <= reset_btn_y+reset_btn_h:
                lib.reset_drowsiness()

except KeyboardInterrupt:
    pass
except Exception as e:
    print(f"ERROR: {e}")
