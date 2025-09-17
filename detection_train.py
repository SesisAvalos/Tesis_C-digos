# -- coding: utf-8 --
#Importa la clase YOLO desde ultralytics
from ultralytics import YOLO
#Importaciones para mostrar y visualizar imágenes
from IPython.display import display, Image
from IPython import display
#Se limpia la salida y se importa ultralytics
display.clear_output()
import ultralytics
#Se verifica que el entorno este bien desarrollado
ultralytics.checks()
#Se importa os para interactuar con el sistema operativo y se configura una variable de entorno para evitar errores con bibliotecas duplicadas 
import os
os.environ['KMP_DUPLICATE_LIB_OK']='True'

model = YOLO("D:/Usuarios/SesiTesis/Modelos Preentrenados Yolo/yolo11n.pt") # Version del modelo de Yolo a usar n es nano

PATH=r"D:\Usuarios\SesiTesis\Tesis_SA_DATASETS3.v1i.yolov11\data.yaml" #Ruta del archivo yaml (archivo donde vienen las rutas de los datos y los tipos de clases a evaluar)

#Se proporcionan los parametros para el entrenamiento
model.train(data=PATH,
            epochs=300, #numero de epocas del entrenamiento
            patience=50, #cantidad de epocas para evaluar un posible early stop en caso de que no mejore el entrenamiento despues de esa cantidad
            imgsz=256, #Image Size
            batch=16, #Tamano del lote no colocar tan grande si el imgsz es muy grande o no lograra soportar
            workers=0,
            flipud=0.5, # image flip up-down (probability)
            fliplr=0.5,  # image flip left-right (probability)
            degrees=180, 
            device = 0,
            name='Prueba_11') #CAMBIA EL NOMBRE EN CADA ENTRENAMIENTO
