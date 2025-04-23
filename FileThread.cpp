#include <iostream>
//Se incluye FileThread.h
#include "FileThread.h"

//Se crea el constructor y destuctor de la clase FileThread
FileThread::FileThread() : QThread(){
}
FileThread::~FileThread() {
}

void FileThread::setFile(){file.setFileName(Dir);}

//Esta función guarda los datos de la imagen en un archivo como una matriz. El arreglo tempImage contiene los datos de la imagen en escala de grises 
void FileThread::saveMatrix(QVector<int> tempImage){
	if( file.open(QIODevice::WriteOnly))
	{
		QTextStream Stream(&file);.
		for( int i=1;i<19201; i++){
			Stream<<tempImage[i-1];
			if(i%160==0) Stream <<"\n";
			else Stream<<",";
		}
	file.close();.
	}
}

//Se verifica si pythonAI es verdadero o falso
void FileThread::GetImage()
{	if(pythonAI){
        QProcess *process = new QProcess(this);
        QString program = "python";
        QStringList arguments = {"Hand_predict_Grayscale.py"}; //Se ejecuta el código "Hand..." en python

        process->start(program, arguments);     
	}
	else{
		if (!file.exists()) {
			qDebug() << "Error: File does not exist.";
		} else {
			if (!file.remove()) {
				qDebug() << "Error: Unable to remove the file.";
			}
		}
	} 
}

