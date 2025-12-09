#!/usr/bin/env python3
from maix import camera, display, image, app, touchscreen, nn
import ctypes
import os

try:
    lib = ctypes.CDLL('/root/libdrowsiness.so')
    
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
    
    class CircleToDraw(ctypes.Structure):
        _fields_ = [("x", ctypes.c_int), ("y", ctypes.c_int), ("radius", ctypes.c_int),
                    ("r", ctypes.c_ubyte), ("g", ctypes.c_ubyte), ("b", ctypes.c_ubyte)]
    
    class RectToDraw(ctypes.Structure):
        _fields_ = [("x", ctypes.c_int), ("y", ctypes.c_int), ("w", ctypes.c_int), ("h", ctypes.c_int),
                    ("r", ctypes.c_ubyte), ("g", ctypes.c_ubyte), ("b", ctypes.c_ubyte), ("thickness", ctypes.c_int)]
    
    class TextToDraw(ctypes.Structure):
        _fields_ = [("x", ctypes.c_int), ("y", ctypes.c_int), ("text", ctypes.c_char * 200),
                    ("r", ctypes.c_ubyte), ("g", ctypes.c_ubyte), ("b", ctypes.c_ubyte), ("scale", ctypes.c_float)]
    
    class RenderCommands(ctypes.Structure):
        _fields_ = [
            ("circles", CircleToDraw * 20), ("circle_count", ctypes.c_int),
            ("rects", RectToDraw * 10), ("rect_count", ctypes.c_int),
            ("texts", TextToDraw * 15), ("text_count", ctypes.c_int)
        ]
    
    lib.extract_and_scale_landmarks.argtypes = [
        ctypes.POINTER(ctypes.c_float), ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int,
        ctypes.POINTER(ctypes.c_int), ctypes.c_int,
        ctypes.POINTER(ctypes.c_int), ctypes.c_int,
        ctypes.POINTER(ctypes.c_int), ctypes.c_int,
        ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)
    ]
    
    lib.analyze_drowsiness.argtypes = [
        ctypes.POINTER(ctypes.c_float), ctypes.c_int,
        ctypes.POINTER(ctypes.c_float), ctypes.c_int,
        ctypes.POINTER(ctypes.c_float), ctypes.c_int,
        ctypes.POINTER(DrowsinessMetrics)
    ]
    
    lib.generate_render_commands.argtypes = [
        ctypes.POINTER(ctypes.c_float), ctypes.c_int,
        ctypes.POINTER(ctypes.c_float), ctypes.c_int,
        ctypes.POINTER(ctypes.c_float), ctypes.c_int,
        ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int,
        ctypes.POINTER(DrowsinessMetrics),
        ctypes.POINTER(RenderCommands)
    ]
    
    lib.reset_drowsiness.argtypes = []
    
    cam = camera.Camera(640, 480, image.Format.FMT_RGB888)
    disp = display.Display()
    ts = touchscreen.TouchScreen()
    detector = nn.YOLOv8(model="/root/models/yolov8n_face.mud")
    landmark_detector = nn.FaceLandmarks(model="/root/models/face_landmarks.mud")
    
    LEFT_EYE = (ctypes.c_int * 6)(33, 160, 158, 133, 153, 144)
    RIGHT_EYE = (ctypes.c_int * 6)(263, 387, 385, 362, 380, 373)
    MOUTH = (ctypes.c_int * 4)(61, 13, 14, 291)
    
    last_commands = None
    frame_count = 0
    
    while not app.need_exit():
        img = cam.read()
        frame_count += 1
        
        img.draw_rect(10, 10, 80, 35, image.Color.from_rgb(200,0,0), -1)
        img.draw_string(15, 23, "< VOLVER", image.Color.from_rgb(255,255,255), scale=0.8)
        img.draw_rect(540, 10, 90, 35, image.Color.from_rgb(0,150,255), -1)
        img.draw_string(550, 23, "RESET", image.Color.from_rgb(255,255,255), scale=0.8)
        
        if frame_count % 2 == 0:
            objs = detector.detect(img, conf_th=0.5, iou_th=0.45)
            
            if objs:
                b = objs[0]
                face = img.crop(b.x, b.y, b.w, b.h)
                
                if face:
                    face_resized = face.resize(256, 256)
                    lm = landmark_detector.detect(face_resized, 0.5, True, False)
                    
                    if lm and lm.valid:
                        landmarks_478 = (ctypes.c_float * 956)(*lm.points)
                        
                        left_out = (ctypes.c_float * 12)()
                        right_out = (ctypes.c_float * 12)()
                        mouth_out = (ctypes.c_float * 8)()
                        
                        lib.extract_and_scale_landmarks(
                            landmarks_478, b.x, b.y, b.w, b.h,
                            LEFT_EYE, 6, RIGHT_EYE, 6, MOUTH, 4,
                            left_out, right_out, mouth_out
                        )
                        
                        metrics = DrowsinessMetrics()
                        lib.analyze_drowsiness(left_out, 6, right_out, 6, mouth_out, 4, ctypes.byref(metrics))
                        
                        last_commands = RenderCommands()
                        lib.generate_render_commands(
                            left_out, 6, right_out, 6, mouth_out, 4,
                            b.x, b.y, b.w, b.h,
                            ctypes.byref(metrics),
                            ctypes.byref(last_commands)
                        )
            else:
                last_commands = None
        
        if last_commands:
            for i in range(last_commands.rect_count):
                r = last_commands.rects[i]
                img.draw_rect(r.x, r.y, r.w, r.h, image.Color.from_rgb(r.r, r.g, r.b), r.thickness)
            
            for i in range(last_commands.circle_count):
                c = last_commands.circles[i]
                img.draw_circle(c.x, c.y, c.radius, image.Color.from_rgb(c.r, c.g, c.b), -1)
            
            for i in range(last_commands.text_count):
                t = last_commands.texts[i]
                img.draw_string(t.x, t.y, t.text.decode(), image.Color.from_rgb(t.r, t.g, t.b), scale=t.scale)
        else:
            img.draw_string(10, 80, "Buscando cara...", image.Color.from_rgb(255,255,0), scale=1.3)
        
        img.draw_string(10, 460, "Core: C", image.Color.from_rgb(0,255,255), scale=0.6)
        disp.show(img)
        
        data = ts.read()
        if data and len(data) >= 3 and data[2] == 1:
            x, y = data[0], data[1]
            if 10 <= x <= 90 and 10 <= y <= 45:
                del cam, disp, ts
                os.execv("/usr/bin/python3", ["python3", "/maixapp/apps/menu_selector/main.py"])
                break
            if 540 <= x <= 630 and 10 <= y <= 45:
                lib.reset_drowsiness()

except Exception as e:
    print(f"ERROR: {e}")
