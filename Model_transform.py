# -- coding: utf-8 --
#  Copyright 2024  <pi@raspberrypi>
#Este codigo transforma un archivo .pt a .onnx

#Se importan las bibliotecas que ayudan a la transformacion de datasets 
#SE importa YOLO y se verifica si el entorno esta correctamente configurado
import torch
import torchvision
import ultralytics
ultralytics.checks()
from ultralytics import YOLO

import os
os.environ['KMP_DUPLICATE_LIB_OK']='True' #Se establece la variable del entorno

#Se carga el modelo YOLO y se le especifica su ruta
model = YOLO('/home/pi/LeptonModule-master/software/raspberrypi_video/prueba7.pt')

#Se exporta el modelo a .onnx
model.export(imgsz=160,format='onnx',task='detect');

