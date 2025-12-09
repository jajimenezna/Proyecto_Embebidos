#!/usr/bin/env python3
from maix import camera, display, image, app, nn
import ctypes
import os

lib = ctypes.CDLL('/root/libdrowsiness.so')

class DrowsinessMetrics(ctypes.Structure):
    _fields_ = [
        ("ear_left", ctypes.c_float), ("ear_right", ctypes.c_float),
        ("ear_avg", ctypes.c_float), ("mar", ctypes.c_float),
        ("eyes_closed", ctypes.c_bool), ("mouth_state", ctypes.c_int),
        ("drowsiness_level", ctypes.c_float), ("blink_count", ctypes.c_int),
        ("eyes_closed_duration", ctypes.c_float), ("blinks_per_minute", ctypes.c_int),
        ("danger_level", ctypes.c_int), ("danger_text", ctypes.c_char * 100)
    ]

lib.analyze_drowsiness.argtypes = [
    ctypes.POINTER(ctypes.c_float), ctypes.c_int,
    ctypes.POINTER(ctypes.c_float), ctypes.c_int,
    ctypes.POINTER(ctypes.c_float), ctypes.c_int,
    ctypes.POINTER(DrowsinessMetrics)
]

cam = camera.Camera(640, 480, image.Format.FMT_RGB888)
disp = display.Display()
detector = nn.YOLOv8(model="/root/models/yolov8n_face.mud")
lm_det = nn.FaceLandmarks(model="/root/models/face_landmarks.mud")

L_EYE = (ctypes.c_int * 6)(33, 160, 158, 133, 153, 144)
R_EYE = (ctypes.c_int * 6)(263, 387, 385, 362, 380, 373)
MOUTH = (ctypes.c_int * 4)(61, 13, 14, 291)

frame_cnt = 0
last_result = None

while not app.need_exit():
    img = cam.read()
    frame_cnt += 1
    
    if frame_cnt % 2 == 0:
        objs = detector.detect(img, conf_th=0.5, iou_th=0.45)
        if objs:
            b = objs[0]
            face = img.crop(b.x, b.y, b.w, b.h).resize(256, 256)
            lm = lm_det.detect(face, 0.5, True, False)
            
            if lm and lm.valid:
                sx, sy = b.w/256.0, b.h/256.0
                left = (ctypes.c_float * 12)(*[c for i in L_EYE for c in [lm.points[i*2]*sx+b.x, lm.points[i*2+1]*sy+b.y]])
                right = (ctypes.c_float * 12)(*[c for i in R_EYE for c in [lm.points[i*2]*sx+b.x, lm.points[i*2+1]*sy+b.y]])
                mouth = (ctypes.c_float * 8)(*[c for i in MOUTH for c in [lm.points[i*2]*sx+b.x, lm.points[i*2+1]*sy+b.y]])
                
                last_result = DrowsinessMetrics()
                lib.analyze_drowsiness(left, 6, right, 6, mouth, 4, ctypes.byref(last_result))
    
    if last_result:
        cols = [image.Color.from_rgb(0,255,0), image.Color.from_rgb(255,200,0), image.Color.from_rgb(255,0,0)]
        if last_result.danger_level > 0:
            img.draw_rect(150, 200, 350, 80, cols[last_result.danger_level], -1)
            img.draw_string(170, 230, last_result.danger_text.decode(), image.COLOR_WHITE, scale=1.8)
        img.draw_string(10, 80, f"Blinks: {last_result.blink_count} | BPM: {last_result.blinks_per_minute}", image.COLOR_WHITE, scale=1.0)
    
    disp.show(img)
