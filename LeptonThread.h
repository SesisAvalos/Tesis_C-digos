//Se incluyen librerías
#ifndef TEXTTHREAD
#define TEXTTHREAD
#include <ctime>
#include <stdint.h>

//Se incluyen librerías de Qt
#include <QThread>
#include <QtCore>
#include <QPixmap>
#include <QImage>
#include <QString>
#include <QDir>
#include <QTimer>
#include <QMessageBox>
#include <QGenericMatrix>
#include <QVector>

#include <QPainter>
#include <QPen>

//Se manda a llamar a LEPTON_CONFIG.h
#include "LEPTON_CONFIG.h"

//Se definen varias constantes relacionadas con el tamaño de los paquetes de datos:
#define PACKET_SIZE 164
#define PACKET_SIZE_UINT16 (PACKET_SIZE/2)
#define PACKETS_PER_FRAME 60
#define FRAME_SIZE_UINT16 (PACKET_SIZE_UINT16*PACKETS_PER_FRAME)

//Se declara la clase LeptonThread
class LeptonThread : public QThread
{
  Q_OBJECT;
//Se declaran métodos públicos
public:
  LeptonThread();	//Constructor 
  ~LeptonThread();	//Destructor
//Se declaran los métodos de configuración
  void setLogLevel(uint16_t);
  void useColormap(int);
  void useLepton(int);
  void useSpiSpeedMhz(unsigned int);
  void setAutomaticScalingRange();
  void useRangeMinValue(uint16_t);
  void useRangeMaxValue(uint16_t);
//Se declaran los métodos para el procesamiento y almacenamiento  
bool Datos(QString,uint16_t,uint16_t);	 
  void saveImage(QString);
  void useOutputFormat(uint8_t);
  void Set_NROI(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2);
  void setBBox(int Temp_BBox[4]);
  
//Método principal para el hilo
  void run();
  
  //Variables públicas
  uint16_t Frames;
  uint16_t Time;
  uint16_t FPA_temp;
  uint16_t Hous_temp;
  bool AI;
  
//Slots públicos
public slots:
  void performFFC();
  void Photo();
  void Video(); 
  void Timed();
  void startAI();

//Señales públicas
signals:
  void updateImage(QImage);  
  void infereImage(QVector<int> ImageBuffer);  

//Se declaran las variables privadas
private:
  void log_message(uint16_t, std::string);
  uint16_t loglevel;
  int typeColormap;
  const int *selectedColormap;
  int selectedColormapSize;
  int typeLepton;
  unsigned int spiSpeed;
  bool autoRangeMin;
  bool autoRangeMax;
  uint16_t rangeMin;
  uint16_t rangeMax;
  int myImageWidth;
  int myImageHeight;
  QImage myImage;
  QString Name;
  bool video;


//Se declaran las variables de procesamiento de datos, almacenan regiones de interés y estadísticas de temperatitura
  void saveImageROI(QString);
  LEP_SYS_VIDEO_ROI_T ROI;
  LEP_RAD_ROI_T ROI_1;
  LEP_SYS_SCENE_STATISTICS_T ROI_Stats;
  LEP_RAD_SPOTMETER_OBJ_KELVIN_T ROI_Stats_1;
  bool AGC,Raw14,Raw16;
  
//Se declaran buffers de datos, estos almacenan los paquetes de datos de la cámara antes de ser convertidos en imágenes
  int array[8];
  uint8_t result[PACKET_SIZE*PACKETS_PER_FRAME];
  uint8_t shelf[4][PACKET_SIZE*PACKETS_PER_FRAME];
  uint16_t *frameBuffer;
};

#endif

