# -- coding: utf-8 --
"""
Created on Tue Jan 30 01:06:35 2024

@author: cdmor
"""

#Se importan las librerías y se importa YOLO
import numpy as np
import cv2
import torch
from ultralytics import YOLO
from ultralytics.utils.benchmarks import benchmark

from IPython.display import display, Image
from IPython import display
display.clear_output()
import ultralytics
ultralytics.checks()

import time
import sys
import os
os.environ['KMP_DUPLICATE_LIB_OK']='True'

#Se declaran las configuraciones para el uso de YOLO
device1 = 'cuda' if torch.cuda.is_available() else 'cpu'
model = YOLO('/home/pi/LeptonModule-master/software/raspberrypi_video/prueba6.onnx') #Se envia la ruta donde se encuentra el modelo de YOLO previamente entrenado

#Aquí es donde se realiza el trackeo de los objetos en la cámara térmica, si reconoce alguna de los objetos guarda sus coordenadas en un archivo, si no reconoce nada las coordenadas serán 0 0 0 0 
while True:
       try:
              csv_file = "/home/Thermal_Camera/AI_Temps.csv"
              matrix = np.genfromtxt(csv_file, delimiter=',')
              image_rgb = np.stack((matrix,matrix,matrix),axis=-1)

              results = model.predict(image_rgb, 
                        save=False, 
                        imgsz=160,  # Puede ser un solo número
                        conf=0.5, 
                        device=device1)
              boxes=[]
              for result in results:
                     boxes.append(result.cpu().numpy().boxes.xyxy)
                     print(boxes)
                     
              for box in boxes:
                     for i in box:
                            with open('/home/Thermal_Camera/output.csv', 'w') as f:
                                   temp = [round(num) for num in i]
                                   print(*temp,file=f)
                                   print(*temp)
                     if not box:
                            with open('/home/Thermal_Camera/output.csv', 'w') as f:
                                   temp = (0,0,0,0)
                                   print(*temp,file=f)
                                   print(*temp)
#Se agrega una pequeña pausa ya que la interferencia es muy rápida
              time.sleep(0.1)
#Si la carpeta AI no se encuentra termina el programa
       except IOError as e:
              os.remove('/home/Thermal_Camera/output.csv')
              sys.exit()
                 
#Se agrega una pequeña pausa para comenzar de nuevo y evitar errores 
       except Exception as e:
              time.sleep(0.1)

