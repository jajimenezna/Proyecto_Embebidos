#!/usr/bin/env python3
from maix import display, image, app, touchscreen, nn
import ctypes
import os

try:
    # Cargar librería con hardware wrapper
    lib = ctypes.CDLL('/root/libdrowsiness_hw.so')
    
    class DrowsinessMetrics(ctypes.Structure):
        _fields_ = [
            ("ear_left", ctypes.c_float), ("ear_right", ctypes.c_float),
            ("ear_avg", ctypes.c_float), ("mar", ctypes.c_float),
            ("eyes_closed", ctypes.c_bool), ("mouth_state", ctypes.c_int),
            ("drowsiness_level", ctypes.c_float), ("blink_count", ctypes.c_int),
            ("eyes_closed_duration", ctypes.c_float), ("blinks_per_minute", ctypes.c_int),
            ("danger_level", ctypes.c_int), ("danger_text", ctypes.c_char * 100)
        ]
    
    # Funciones de hardware en C
    lib.hw_init.argtypes = [ctypes.c_int, ctypes.c_int]
    lib.hw_init.restype = ctypes.c_int
    
    lib.hw_camera_read.argtypes = [
        ctypes.POINTER(ctypes.POINTER(ctypes.c_ubyte)),
        ctypes.POINTER(ctypes.c_int),
        ctypes.POINTER(ctypes.c_int),
        ctypes.POINTER(ctypes.c_int),
        ctypes.POINTER(ctypes.c_int)
    ]
    lib.hw_camera_read.restype = ctypes.c_int
    
    lib.hw_camera_free.argtypes = []
    lib.hw_camera_free.restype = None
    
    # Funciones de análisis
    lib.analyze_drowsiness.argtypes = [
        ctypes.POINTER(ctypes.c_float), ctypes.c_int,
        ctypes.POINTER(ctypes.c_float), ctypes.c_int,
        ctypes.POINTER(ctypes.c_float), ctypes.c_int,
        ctypes.POINTER(DrowsinessMetrics)
    ]
    lib.reset_drowsiness.argtypes = []
    
    # Inicializar hardware desde C
    print("Inicializando hardware desde C...")
    if lib.hw_init(640, 480) != 0:
        print("ERROR: hw_init falló")
        exit(1)
    
    # Display sigue siendo Python (necesario para MaixPy)
    disp = display.Display()
    ts = touchscreen.TouchScreen()
    
    # YOLO y Landmarks siguen en Python (TPU - inevitable)
    detector = nn.YOLOv8(model="/root/models/yolov8n_face.mud")
    landmark_detector = nn.FaceLandmarks(model="/root/models/face_landmarks.mud")
    
    LEFT_EYE = (ctypes.c_int * 6)(33, 160, 158, 133, 153, 144)
    RIGHT_EYE = (ctypes.c_int * 6)(263, 387, 385, 362, 380, 373)
    MOUTH = (ctypes.c_int * 4)(61, 13, 14, 291)
    
    last_metrics = None
    frame_count = 0
    
    print("Sistema iniciado con wrapper C")
    
    while not app.need_exit():
        # TODO: Capturar frame desde C
        # Por ahora usar MaixPy hasta implementar conversión de formato
        # El wrapper C está listo, falta integración completa
        
        # Crear imagen temporal para UI
        img = image.Image(640, 480, image.Format.FMT_RGB888)
        
        img.draw_rect(10, 10, 80, 35, image.Color.from_rgb(200,0,0), -1)
        img.draw_string(15, 23, "< VOLVER", image.Color.from_rgb(255,255,255), scale=0.8)
        img.draw_rect(540, 10, 90, 35, image.Color.from_rgb(0,150,255), -1)
        img.draw_string(550, 23, "RESET", image.Color.from_rgb(255,255,255), scale=0.8)
        
        img.draw_string(200, 200, "Wrapper C activo", image.Color.from_rgb(0,255,0), scale=2.0)
        img.draw_string(150, 240, "Integracion en progreso", image.Color.from_rgb(255,255,0), scale=1.5)
        
        disp.show(img)
        
        data = ts.read()
        if data and len(data) >= 3 and data[2] == 1:
            x, y = data[0], data[1]
            if 10 <= x <= 90 and 10 <= y <= 45:
                lib.hw_deinit()
                del disp, ts
                os.execv("/usr/bin/python3", ["python3", "/maixapp/apps/menu_selector/main.py"])
                break

except Exception as e:
    print(f"ERROR: {e}")
    import traceback
    traceback.print_exc()
