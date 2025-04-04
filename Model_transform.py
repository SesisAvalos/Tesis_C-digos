#!/usr/bin/env python
# -- coding: utf-8 --
#
#  Model_transform.py
#  
#  Copyright 2024  <pi@raspberrypi>
#  
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.

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

