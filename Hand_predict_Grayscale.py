#Se importan las librerias y se importa YOLO
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
model = YOLO('/home/pi/LeptonModule-master/software/raspberrypi_video/prueba11.onnx')

#Aqui es donde se realiza el trackeo de los objetos en la camara termica, si reconoce alguno de los objetos guarda sus coordenadas en un archivo, si no reconoce nada las coordenadas seran 0 0 0 0
while True:
       try:
              csv_file = "/home/Thermal_Camera/AI_Temps.csv"
              matrix = np.genfromtxt(csv_file, delimiter=',')
              image_rgb = np.stack((matrix,matrix,matrix),axis=-1)
              image_rgb = image_rgb.astype(np.uint8)
              
              results = model.predict(image_rgb, 
                        save=False, 
                        imgsz=160,  # Puede ser un solo número
                        conf=0.3,
                        #iou=0.3, 
                        device=device1)
              
              #for i in results:
              #        print("Boxes: ", r.boxes.xyxy)
              #        print("Confianzas: ", r.boxes.conf)
              
              boxes=[]
              for result in results:
                     boxes.append(result.cpu().numpy().boxes.xyxy)
                     print(boxes)
                     
              #print("Si no hay caja entonces se escrben ceros, y si si hay se escriben las coordenadas en output.csv")
              with open('/home/Thermal_Camera/output.csv','w') as f:
                      any_box = False
                      for box in boxes:
                              if len(box) > 0:
                                      any_box = True
                                      for i in box:
                                              temp = [round(num) for num in i]
                                              print(*temp, file=f)
                                              #print(*temp)
                      if not any_box:
                               temp = (0,0,0,0)
                               print(*temp,file=f)
                               print(*temp)
#Se agrega una pequenna pausa ya que la inferencia es muy rapida
              time.sleep(0.1)
#Si la carpeta AI no se encuentra termina el programa
       except IOError as e:
              os.remove('/home/Thermal_Camera/output.csv')
              sys.exit()

#Se agrega una pequenna pausa para comenzar de nuevo y evitar errores
       except Exception as e:
              print("Da una pausa para comenzar de nuevo y evitar errores")
              time.sleep(0.1)
