#!/usr/bin/env python3
from maix import display, image, touchscreen, app
import time
import os

def show_menu():
    # Inicialización de hardware
    disp = display.Display()
    ts = touchscreen.TouchScreen()
    
    # Configuración de botón
    btn_w, btn_h = 500, 200
    btn_x = (640 - btn_w) // 2
    btn_y = (480 - btn_h) // 2
    
    while not app.need_exit():
        # Creación de frame
        img = image.Image(640, 480, image.Format.FMT_RGB888)
        img.draw_rect(0, 0, 640, 480, image.COLOR_BLACK, thickness=-1)
        
        # Título
        img.draw_string(130, 80, "SISTEMA DE DETECCION", color=image.COLOR_WHITE, scale=1.8)
        
        # Botón principal
        img.draw_rect(btn_x, btn_y, btn_w, btn_h, image.Color.from_rgb(56, 120, 235), thickness=-1)
        img.draw_string(btn_x+110, btn_y+60, "DETECTOR DE", color=image.COLOR_WHITE, scale=2.5)
        img.draw_string(btn_x+105, btn_y+110, "SOMNOLENCIA", color=image.COLOR_WHITE, scale=2.5)
        
        # Información
        img.draw_string(220, 400, "Core: C puro", color=image.Color.from_rgb(0,255,255), scale=1.2)
        disp.show(img)
        
        # Procesamiento de eventos táctiles
        data = ts.read()
        if data and isinstance(data, list) and len(data) == 3:
            x, y, pressed = data[0], data[1], data[2]
            if pressed == 1 and btn_x <= x <= btn_x+btn_w and btn_y <= y <= btn_y+btn_h:
                del disp
                del ts
                time.sleep(0.3)
                os.execv("/usr/bin/python3", ["python3", "/maixapp/apps/drowsiness_detector/main.py"])
                return
        time.sleep(0.03)

if __name__ == "__main__":
    show_menu()
