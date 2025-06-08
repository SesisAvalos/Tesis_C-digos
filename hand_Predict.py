# -- coding: utf-8 --
"""
Created on Wed Nov 22 17:48:55 2023

@author: cdmor
"""
import numpy as np
import cv2
import torch
from ultralytics import YOLO
from ultralytics.utils.benchmarks import benchmark
import os

from IPython.display import display, Image
from IPython import display
display.clear_output()
import ultralytics
ultralytics.checks()

# Evitar conflictos con librerías
os.environ['KMP_DUPLICATE_LIB_OK'] = 'True'

# Configurar el dispositivo de inferencia
device1 = 'cuda' if torch.cuda.is_available() else 'cpu'

# Cargar modelo YOLO entrenado
model = YOLO('/home/pi/LeptonModule-master/software/raspberrypi_video/prueba11.pt')

# Directorio de imágenes
#Img_Test = '/home/pi/LeptonModule-master/prueba'
Img_Test = '/home/Thermal_Camera/patients/pruebaLepton1/Timed'

# Ejecutar inferencia con YOLO (forzando la detección de clases 0 y 1)
results = model.predict(
    source=Img_Test, 
    save=True, 
    imgsz=160,
    augment=True,
    flipud=0.0,  # Probabilidad de voltear verticalmente
    fliplr=0.5,  # Probabilidad de voltear horizontalmente
    degrees=90,
    name='Test',
    device=device1,
    conf=0.2,   # Se baja el umbral de confianza para detectar más objetos
    classes=[0, 1]  # Se fuerza a YOLO a detectar solo estas clases
)

# Lista para almacenar las detecciones
detections = []

# Recorrer los resultados obtenidos
for result in results:
    # Extraer cajas delimitadoras y clases detectadas
    boxes = result.boxes.xyxy.cpu().numpy()  # Coordenadas de los cuadros
    classes = result.boxes.cls.cpu().numpy().astype(int)  # Clases detectadas (convertidas a enteros)

    # Almacenar en la lista de detecciones
    for box, cls in zip(boxes, classes):
        detections.append((cls, box))  # Guardar clase y caja

# Mostrar las clases y cajas detectadas
if detections:
    print("\n📌 *Objetos detectados:*")
    for cls, box in detections:
        print(f"Clase: {cls} - Coordenadas: {box}")
else:
    print("\nNo se detectaron objetos en la imagen.")
