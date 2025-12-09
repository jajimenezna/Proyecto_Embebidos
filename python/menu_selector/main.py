#!/usr/bin/env python3
from maix import display, image, touchscreen, app
import time
import os

disp = display.Display()
ts = touchscreen.TouchScreen()

while not app.need_exit():
    img = image.Image(640, 480, image.Format.FMT_RGB888)
    img.draw_rect(0, 0, 640, 480, image.COLOR_BLACK, -1)
    img.draw_string(130, 80, "SISTEMA DE DETECCION", image.COLOR_WHITE, scale=1.8)
    img.draw_rect(70, 140, 500, 200, image.Color.from_rgb(56, 120, 235), -1)
    img.draw_string(180, 200, "DETECTOR DE", image.COLOR_WHITE, scale=2.5)
    img.draw_string(175, 250, "SOMNOLENCIA", image.COLOR_WHITE, scale=2.5)
    img.draw_string(220, 400, "Core: C puro", image.Color.from_rgb(0,255,255), scale=1.2)
    disp.show(img)
    
    data = ts.read()
    if data and len(data) == 3 and data[2] == 1:
        x, y = data[0], data[1]
        if 70 <= x <= 570 and 140 <= y <= 340:
            del disp, ts
            time.sleep(0.3)
            os.execv("/usr/bin/python3", ["python3", "/maixapp/apps/drowsiness_detector/main.py"])
            break
    time.sleep(0.03)
