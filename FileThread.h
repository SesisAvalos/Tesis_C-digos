//Se realiza un inclue guard, evita que la cabecera se incluya más de una vez
#ifndef TXTTHREAD
#define TXTTHREAD

//Se incluyen librerías
#include <ctime>
#include <stdint.h>
#include <cstdio>
#include <thread>
#include <string>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>

//Se incluyen librerías de Qt
#include <QString>
#include <QDir>
#include <QThread>
#include <QFile>
#include <QTextStream>
#include <QVector>

#include <QProcess>
#include <QByteArray>

#include <QDebug>

//Se crea la clase FileThread, esta se ejecuta en un hilo separado 
class FileThread : public QThread
{
  Q_OBJECT;
//Se definen variables públicas
public:
  FileThread(); //Se crea el constructor
  ~FileThread(); //Se crea el destructor
  
  void setFile();
  void createFile();
  bool pythonAI=true;

//Se declaran los slots, estos son funciones en Qt que se llaman en respuesta a señales
public slots:
  void GetImage();  
  void saveMatrix(QVector<int> tempImage);
  
//Se crean los parámetros privados
private slots:
   
signals:
  void readyBBox();

private:
  QString Dir =  QString("/home/Thermal_Camera/AI_Temps.csv");
  QFile file;
  QVector<int> tempsImage;
  
};

#endif
