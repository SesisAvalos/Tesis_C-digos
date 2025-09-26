//Se importan y agregan las librerias necesarias
#include <QApplication>
#include <QThread> 
#include <QMutex>
#include <QColor> 
#include <QLabel>
#include <QtDebug>
#include <QPushButton>
#include <QCloseEvent>
//Se importan los codigos que hacen funcionar la cámara térmica
#include "LeptonThread.h"
#include "FileThread.h" 
#include "MyLabel.h"
#include "LEPTON_CONFIG.h"

#include <QAbstractButton> 
#include <QLineEdit>
#include <QDebug>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QPainter>
#include <QPen>
#include <QGridLayout>
#include <QMetaType>
#include <QProcess>

#include <ctime>
#include <stdint.h>
#include <cstdio> 
#include <thread>
#include <string>
#include <iostream>
#include <wiringPi.h>

//Se define la clase MyWidget. El método closeEvent se ha sobrescrito para gestionar un evento de cierre, en el que se comprueba la existencia de un archivo CSV y se elimina si existe.
class MyWidget : public QWidget {
public:
    MyWidget(QWidget *parent = nullptr) : QWidget(parent) {}

protected:
    void closeEvent(QCloseEvent *event) override {
	QString Dir =  QString("/home/Thermal_Camera/AI_Temps.csv");
	QFile file(Dir); 
	if(!file.exists()){
     if (!file.remove()) {}
	}
	event->accept();}
};

//Se declaran dos punteros, thread y file, que apuntaran a objetos de las clases LeptonThread y FileThread respectivamente.
LeptonThread *thread = new LeptonThread();
FileThread *file = new FileThread();

//Variable para los parametros relacionados con la radiometria de flujo lineal
LEP_RAD_FLUX_LINEAR_PARAMS_T Radiometry;

//Variable para los modos de ganancia del sistema de la camara termica FLIR
LEP_SYS_GAIN_MODE_OBJ_T gainModeObj;

//Variable de información relacionada con la región de interés específica para el control automático de ganancia en una cámara FLIR.
LEP_AGC_ROI_T agcROI; 

//Variable que representará el límite superior para el recorte en el proceso de ecualización del histograma dentro del AGC en una cámara térmica FLIR.
LEP_UINT16 agcHeqClipLimitHigh;
LEP_UINT16 agcHeqDampingFactor;
LEP_UINT16 agcHeqLinearPercent;

//Función que despliega las opciones que puede pasar al ejecutar el programa, el uso de cdname permite que el mensaje de ayuda sea dinamico
void printUsage(char *cmd) {
        char *cmdname = basename(cmd);
	printf("Usage: %s [OPTION]...\n"
               " -h      display this help and exit\n"
               " -cm x   select colormap\n"
               "           1 : rainbow\n"
               "           2 : grayscale\n"
               "           3 : ironblack [default]\n"
               " -tl x   select type of Lepton\n"
               "           2 : Lepton 2.x [default]\n"
               "           3 : Lepton 3.x\n"
               "               [for your reference] Please use nice command\n"
               "                 e.g. sudo nice -n 0 ./%s -tl 3\n"
               " -ss x   SPI bus speed [MHz] (10 - 30)\n"
               "           20 : 20MHz [default]\n"
               " -min x  override minimum value for scaling (0 - 65535)\n"
               "           [default] automatic scaling range adjustment\n"
               "           e.g. -min 30000\n"
               " -max x  override maximum value for scaling (0 - 65535)\n"
               "           [default] automatic scaling range adjustment\n"
               "           e.g. -max 32000\n"
               " -d x    log level (0-255)\n"
               "", cmdname, cmdname);
	return;
}

//Comienza el main
//Ve si un directorio existe, y si no, lo crea y ajusta los permisos para que cualquier usuario pueda leer y escribir en él.
int main( int argc, char **argv )
{
//Se busca o se crea la carpeta AI
	file->setFile();
	QString folderPath = "/home/Thermal_Camera";
	QDir dir(folderPath);
	if (!dir.exists()) {
		QString command = "sudo mkdir -p " + folderPath;

		QProcess process;
		process.start(command);
		process.waitForFinished(-1); 

		int exitCode = process.exitCode();
		if (exitCode != 0) {
		qDebug() << "Failed to create folder for AI Exit code:" << exitCode;
		} else {
		qDebug() << "Folder created for AI successfully!";
		}
		
		command = "sudo chmod  a+rw /home/Thermal_Camera" ;

		process.start(command);
		process.waitForFinished(-1);

		exitCode = process.exitCode();
		if (exitCode != 0) {
		qDebug() << "Writing permission denied" << exitCode;
		} else {
		qDebug() << "Writing permission granted";
		}
	}

//Se busca o se crea la carpeta patients
	folderPath = "/home/Thermal_Camera/patients";
	dir.setPath(folderPath);
	if (!dir.exists()) {
		QString command = "sudo mkdir -p " + folderPath;

		QProcess process;
		process.start(command);
		process.waitForFinished(-1);

		int exitCode = process.exitCode();
		if (exitCode != 0) {
		qDebug() << "Failed to create folder for patients Exit code:" << exitCode;
		} else {
		qDebug() << "Folder created for patients successfully!";
		}
		command = "sudo chmod  a+rw /home/Thermal_Camera/patients" ;

		process.start(command);
		process.waitForFinished(-1);

		exitCode = process.exitCode();
		if (exitCode != 0) {
		qDebug() << "Writing permission denied" << exitCode;
		} else {
		qDebug() << "Writing permission granted";
		}
	}
//Se hace la declaración de la configuración inicial del sensor
       / * 
	 * NOTA: SOLAMENTE SE PUEDEN CAMBIAR EL VALOR DE LAS VARIABLES COLORMAP Y TYPE LEPTON
	 */
	int typeColormap=1;// colormap_ironblack
	int typeLepton = 3; // Lepton 2.x
	int spiSpeed = 20; // SPI bus speed 20MHz
	int rangeMin = -1; 
	int rangeMax = -_1; 
	int loglevel = 0;.

// Se declara un arreglo de tipo entero sin signo de 16 bits, con dos elementos inicializados en 290 y 420
	uint16_t Temp[2] = {290, 420};.

//Inicializacion de Variables para configuración
	bool Adv=false;
	int basicConfig[4];
	basicConfig[0]=0;
	basicConfig[1]=0;
	basicConfig[2]=0;
	basicConfig[3]=0;

//Se obtienen los parámetros de radiometría iniciales
	LEP_GetRadParms(&Radiometry);
//Se obtienen los valores iniciales de la configuración de ganancia
	LEP_GetGainConfig(&gainModeObj);
//Se obtiene la configuración del control automático de ganancia (AGC)
	LEP_GetAGCConfig(&agcROI, &agcHeqClipLimitHigh, &agcHeqDampingFactor, &agcHeqLinearPercent);

//Se  procesan los argumentos pasados al programa a través de la línea de comandos, permitiendo opciones como -h (para mostrar ayuda), -d (para configurar el nivel de registro), -cm, -tl, -ss (para la velocidad SPI), y los valores mínimo y máximo (-min, -max).
	for(int i=1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0) {
			printUsage(argv[0]);
			exit(0);
		}
		else if (strcmp(argv[i], "-d") == 0) {
			int val = 3;
			if ((i + 1 != argc) && (strncmp(argv[i + 1], "-", 1) != 0)) {
				val = std::atoi(argv[i + 1]);
				i++;
			}
			if (0 <= val) {
				loglevel = val & 0xFF;
			}
		}
		else if ((strcmp(argv[i], "-cm") == 0) && (i + 1 != argc)) {
			int val = std::atoi(argv[i + 1]);
			if ((val == 1) || (val == 2)) {
				typeColormap = val;
				i++;
			}
		}
		else if ((strcmp(argv[i], "-tl") == 0) && (i + 1 != argc)) {
			int val = std::atoi(argv[i + 1]);
			if (val == 3) {
				typeLepton = val;
				i++;
			}
		}
		else if ((strcmp(argv[i], "-ss") == 0) && (i + 1 != argc)) {
			int val = std::atoi(argv[i + 1]);
			if ((10 <= val) && (val <= 30)) {
				spiSpeed = val;
				i++;
			}
		}
		else if ((strcmp(argv[i], "-min") == 0) && (i + 1 != argc)) {
			int val = std::atoi(argv[i + 1]);
			if ((0 <= val) && (val <= 65535)) {
				rangeMin = val;
				i++;
			}
		}
		else if ((strcmp(argv[i], "-max") == 0) && (i + 1 != argc)) {
			int val = std::atoi(argv[i + 1]);
			if ((0 <= val) && (val <= 65535)) {
				rangeMax = val;
				i++;
			}
		}
	}

// Se crea el objeto principal que maneja la aplicacion. Esta linea es esencial en cualquier programa Qt para gestionar la interfaz grafica y el ciclo de eventos.
// argc y argv son los parámetros que recibe la aplicación desde la línea de comandos. 
	QApplication a( argc, argv );

/////////////////////// Main Window ///////////////////////////////////
//Se crea un widget principal (myWidget) de tipo MyWidget, se le da coordenadas de posición y se le da un tamaño.
	MyWidget *myWidget = new MyWidget;
	myWidget->setGeometry(0, 0, 840, 610);

///////////////////////// Config Window ////////////////////////////////
// Se crea el widget para la configuración y se le asigna una posicion y tamaño
	QWidget *configWidget = new QWidget;
	configWidget->setGeometry(200, 150, 260, 300);

///////////////////////// Adanced Config Window ////////////////////////	
// Se crea el widget para la configuración avanzada y se le asigna posicion y tamaño
	QTabWidget *advConfigWidget = new QTabWidget;
	advConfigWidget->setGeometry(200, 150, 450, 300);

// Se crea un objeto, el formato::Format_RGB888 indica que cada píxel de la imagen tiene 3 componentes (Rojo, Verde y Azul) con 8 bits cada uno
	QImage myImage;
	myImage = QImage(160*4, 120*4, QImage::Format_RGB888);

//Se crea una variable red de tipo QRgb con el valor rojo puro (255 en el componente rojo, 0 en los componentes verde y azul)
	QRgb red = qRgb(255,0,0);
	for(int i=0;i<80;i++) {
		for(int j=0;j<60;j++) {
			myImage.setPixel(i, j, red);
		}
	}
// Aquí se recorre una porción de la imagen, cubriendo un área de 80x60 píxeles (la esquina superior izquierda).

//Se crea el objeto myLabel y se establece su tamaño y posición
	MyLabel myLabel(myWidget);
	myLabel.setGeometry(10, 10, 160*4, 120*4);
// Aquí, se convierte el QImage myImage a un QPixmap y se muestra la imagen en el label.
	myLabel.setPixmap(QPixmap::fromImage(myImage));

//Se crean los botones para la pantalla principal
//Se crea el botón FFC y se le asigna tamaño y posición
	QPushButton *button1 = new QPushButton("Perform FFC", myWidget);
	button1->setGeometry(680, 135, 100, 30);
	
//Se crea el botón para las fotos y se le asigna tamaño y posición
	QPushButton *button2 = new QPushButton("Photo", myWidget);
	button2->setGeometry(680, 170, 100, 30);
 
//Se crea el botón para el video y se le asigna tamaño y posición 
	QPushButton *button3 = new QPushButton("Video",myWidget);
	button3->setGeometry(680, 205, 100, 30);
	button3->setCheckable(true);
	
//Se crea el botón para tomar cierto número de fotos a un tiempo determinado y se le asigna tamaño y posición
	QPushButton *button4 = new QPushButton("Timed",myWidget);
	button4->setGeometry(680, 240, 100, 30);
//Se crea el temporizador para el disparo de fotos a cierto tiempo
	QTimer *timer = new QTimer(myWidget);

//Se crea el botón AI y se le asigna tamaño y posición
	QPushButton *AI = new QPushButton("Track", myWidget);
	AI->setGeometry(680, 275, 100, 30);
	AI->setCheckable(true);
//Se crea el temporizador para el disparo de fotos a cierto tiempo
	QTimer *AI_timer = new QTimer(myWidget);
	
//Se crea el botón para enviar los parámetros del main al object thread y se le asigna tamaño y posición
	QPushButton *button5 = new QPushButton(myWidget);
	button5->setGeometry(230,530, 100, 50);
	button5->setText("Parameters");
	
//Se crea el botón para abrir la configuración y se le asigna tamaño y posición
	QPushButton *config = new QPushButton("Config", myWidget);
	config->setGeometry(680, 310, 100, 30);
	
//Se crea una casilla deverificación para AutoRange
	QCheckBox *autoRange = new QCheckBox("Auto Range",myWidget);
    autoRange->setChecked(true); 
    autoRange->setGeometry(680, 105, 110, 30);
	
//Se crea una casilla de verificación para Advanced
	QCheckBox *Advanced = new QCheckBox("Advanced",myWidget);
    Advanced->setChecked(false); //Set the initial state (checked)
    Advanced->setGeometry(680, 345, 110, 30);

//Se crea la caja de texto, el marco y el tiempo para Nombre
	QLineEdit *Nombre = new QLineEdit(myWidget);
	Nombre->setStyleSheet("QLineEdit { width: 250px; height: 20px; }");
	Nombre->move(80,500);
	QLabel *Name = new QLabel("Name: ",myWidget);
	Name->setGeometry(20,500,50,20);
	
//Se crea la caja de texto, el marco y el tiempo para Frames
	QLineEdit *Cuadros = new QLineEdit(myWidget);
	Cuadros->setStyleSheet("QLineEdit { width: 100px; height: 20px; }");
	Cuadros->move(80,530);
	Cuadros->setText("0");
	//Se crea la etiqueta para la caja de texto de etiquetas
	QLabel *Frames = new QLabel("Frames:",myWidget);
	Frames->setGeometry(10,530,60,20);
	
	//Se crea la caja de texto donde se introduce el periodo
	QLineEdit *Tiempo = new QLineEdit(myWidget); 
	Tiempo->setStyleSheet("QLineEdit { width: 100px; height: 20px; }");
	Tiempo->move(80,560);
	Tiempo->setText("0");
//Etiqueta para la caja de texto del periodo
	QLabel *Time = new QLabel("Time: ",myWidget);
	Time->setGeometry(20,560,50,20);

	//Controles deslizantes verticales para máximo y mínimo
	QSlider *sliderMin = new QSlider(Qt::Vertical,myWidget);
	QSlider *sliderMax = new QSlider(Qt::Vertical,myWidget);
	
	sliderMin->setRange(0, 1400);
	sliderMax->setRange(0, 1400);
	
	sliderMin->setValue(Temp[0]);
	sliderMax->setValue(Temp[1]);
	
//Configuración para el slider de rango mínimo	
sliderMin->setStyleSheet(
		"QSlider::groove:vertical {"
		"	border: 1px solid #bbb;"
		"	background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 red, stop:1 blue);"
	
		"	width: 10px;"
		"	border-radius: 5px;"
		"}"
		"QSlider::handle:vertical {"
		"	background: white;"
		"	border: 1px solid #777;"
		"	height: 20px;"
		"	margin: 0 -5px;"
		"	border-radius: 10px;"
		"	backgroung: transparent;"
		"}"
	);
//Configuración para el slider de rango máximo
	sliderMax->setStyleSheet(
		"QSlider::groove:vertical {"
		"	border: 1px solid #bbb;"
		"	background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 red, stop:1 blue);"
	
		"	width: 10px;"
		"	border-radius: 5px;"
		"}"
		"QSlider::handle:vertical {"
		"	background: white;"
		"	border: 1px solid #777;"
		"	height: 20px;"
		"	margin: 0 -5px;"
		"	border-radius: 10px;"
		"}"
	);
	
	//Se configura el movimiento de los sliders
	sliderMin->setTickInterval(10);
	sliderMin->setTickPosition(QSlider::TicksLeft);
	sliderMin->setSingleStep(10);
	sliderMax->setTickInterval(10);
	sliderMax->setTickPosition(QSlider::TicksLeft);
	sliderMax->setSingleStep(10);
	//Se configura la posición de los sliders
	sliderMin->setGeometry(680, 10, 30, 100);
	sliderMax->setGeometry(750, 10, 30, 100);
	
//Se crea un slider para temps
	QLabel *tempMin = new QLabel("",myWidget);
	tempMin->setText("Scale\n min:\n"+QString::number(Temp[0]*10));
	tempMin->setGeometry(705, 10, 50, 70);
	
	QLabel *tempMax = new QLabel("",myWidget);
	tempMax->setText("Scale\n max:\n"+QString::number(Temp[1]*10));
	tempMax->setGeometry(775, 10, 50, 70);

	//Se crea un hilo para recopilar datos SPI y se inicializa el sensor
	thread->setLogLevel(loglevel);
	thread->useColormap(typeColormap);
	thread->useLepton(typeLepton);
	thread->useSpiSpeedMhz(spiSpeed);
	thread->setAutomaticScalingRange();
	
	//Se establece el rango automático
	if (0 <= rangeMin) thread->useRangeMinValue(rangeMin);
	if (0 <= rangeMax) thread->useRangeMaxValue(rangeMax);

	//Se conecta la imagen del sensor a la pantalla principal y la muestra
		QObject::connect(thread, SIGNAL(updateImage(QImage)), &myLabel, SLOT(setImage(QImage)));

	QObject::connect(AI, SIGNAL(clicked()), thread, SLOT(startAI()));
	QObject::connect(AI, SIGNAL(clicked()), file, SLOT(GetImage()));
// Se conecta el botón de la IA para iniciar el guardado de imágenes en un archivo CSV y comenzar la IA.
// Conecta la señal clicked() del objeto AI al slot startAI() del objeto thread. Cuando el botón de IA es presionado, se ejecuta el método startAI() en el hilo.
// Conecta la señal clicked() del mismo botón AI al slot GetImage() del objeto file. Esto indica que al hacer clic en el botón de IA, también se ejecutará el método GetImage() en el objeto file para obtener y posiblemente guardar la imagen.

// Se conecta el botón AI a un temporizador (AI_timer) que actualizará una caja delimitadora (Bounding Box) cada 100 ms.
// Se usa la función QObject::connect para conectar el evento clicked a una función lambda que ejecuta el código que sigue dentro de {}. 
	QObject::connect(AI, &QPushButton::clicked, [&]()){
		if (AI_timer->isActive()){			
			AI_timer->stop();
		} else {
			AI_timer->setInterval(100);
			AI_timer->start();
		}
// Si el temporizador está activo, significa que ya se está ejecutando, por lo que lo detiene .
// Si el temporizador no está activo (es decir, está detenido), lo configura para que se ejecute cada 100 ms
		file->pythonAI=!(file->pythonAI);
	});
// Invierte el estado de la variable pythonAI del objeto file. Si el valor es true, se cambia a false, y viceversa.

// Las imagenes se enviarán al objeto file para ser guardadas en un archivo CSV, que será leído por un script de Python.
	QObject::connect(thread, SIGNAL(infereImage(QVector<int>)), file, SLOT(saveMatrix(QVector<int>)));
	
// Se conecta el evento clicked del botón button4 para iniciar la toma de fotos en un intérvalo de tiempo.
	QObject::connect(button4, &QPushButton::clicked, [&](){
// Se inicia el temporizador timer con un intervalo definido, multiplicado por 1000 (para convertir segundos a milisegundos).
		timer->start((thread->Time)*1000);
		thread->Timed();
		
		QString Count = QString::number((thread->Frames));
		Cuadros->setText(Count);
	});

//Se conecta el botón button1 con la ranura performFFC() para realizar la corrección de fondo (Flat Field Correction o FFC)
	QObject::connect(button1, SIGNAL(clicked()), thread, SLOT(performFFC()));

//Se conecta el botón button2 con la ranura Photo() para realizar la acción de tomar una foto
	QObject::connect(button2, SIGNAL(clicked()), thread, SLOT(Photo()));

// Conecta el botón button3 con la ranura Video() para realizar la acción de grabar un video
	QObject::connect(button3, SIGNAL(clicked()), thread, SLOT(Video()));

//Se configura el temporizador para tomar fotos periódicas y se actualiza el contador de cuadros.
	QObject::connect(button4, &QPushButton::clicked, [&](){
		timer->start((thread->Time)*1000);
				thread->Timed();
		QString Count = QString::number((thread->Frames));
		Cuadros->setText(Count);
	});	

// Conecta la señal timeout del temporizador con una lambda que se ejecuta cada vez que el temporizador se activa (al cumplirse el intervalo).
	QObject::connect(timer, &QTimer::timeout, [&](){
QString Count = QString::number((thread->Frames)-1);
// Actualiza el texto de Cuadros con el número de cuadros restantes (restando 1 a Frames).
Cuadros->setText(Count);
// Si Frames es menor o igual a 1, se detiene el temporizador. Esto asegura que el temporizador se detenga cuando se hayan tomado todas las fotos.
		if((thread->Frames)<=1) timer->stop();
	});	

	QObject::connect(timer, SIGNAL(timeout()), thread, SLOT(Timed()));
// Conecta el evento timeout del temporizador con la ranura Timed() del objeto thread. Esto guarda una foto automáticamente cada vez que el temporizador se activa

// Conecta el botón button5 al envío de parámetros al sensor y a la verificación de la información.
	QObject::connect(button5, &QPushButton::clicked, [&](){	

		if (thread->Datos(Nombre->text(),Cuadros->text().toUInt(), Tiempo->text().toUInt()))
// Llama al método Datos() del objeto thread. Si los datos son correctos, se muestra un cuadro de diálogo
			QMessageBox::information(nullptr,"Information","Correct");
		else QMessageBox::critical(nullptr,"Error","Invalid Name");
		// Detiene el temporizador
		timer->stop();
		});

	// Conecta el botón config para abrir la ventana de configuración.
	QObject::connect(config, &QPushButton::clicked, [&](){
// Si Adv es falso, se muestra la ventana de configuración estándar. Si Adv es verdadero, se muestra la ventana de configuración avanzada.
if(!Adv) configWidget->show();
		else advConfigWidget->show();
	});	

// Se conecta el evento stateChanged del checkbox autoRange. Cuando cambia el estado de la casilla (activada o desactivada), se ejecuta una función lambda.
QObject::connect(autoRange, &QCheckBox::stateChanged, [&](bool state) {
// Si state es verdadero, se habilita el rango automático.
        if (state) {
thread->setAutomaticScalingRange();
// Si rangeMin y rangeMax son mayores o iguales a 0, se ajustan los valores mínimo y máximo del rango de escalado automático 
		if (0 <= rangeMin) thread->useRangeMinValue(rangeMin);
		if (0 <= rangeMax) thread->useRangeMaxValue(rangeMax);
        	} 
else {
// Si state es falso, significa que el rango automático está desactivado, por lo que se establece un rango manual.
            thread->useRangeMinValue(Temp[0]);
            thread->useRangeMaxValue(Temp[1]);
        }
    });
    
    //Conecta el evento stateChanged del checkbox Advanced
    QObject::connect(Advanced, &QCheckBox::stateChanged, [&](bool state) {
    //  Si state es verdadero, se asigna el valor true a la variable Adv, indicando que el modo avanzado está activado.
        if (state) Adv=state;
    //Si state es falso, se asigna false a Adv, desactivando el modo avanzado.
		else Adv=state;
    });

//Se configura el comportamiento del sliders (deslizadores) de rango mímimo de la ventana principal.
	QObject::connect(sliderMin, &QSlider::valueChanged, [&](uint16_t value) {

 //Desactiva la opción de ajuste automático de rango
		autoRange->setChecked(false);
		Temp[0] = value;
		if(basicConfig[0]==0) tempMin->setText("Scale\n min:\n"+QString::number(Temp[0]*10));
		else if(basicConfig[0]== 1) tempMin->setText("Temp\n min:\n"+QString::number(Temp[0]/10));
		else tempMin->setText("scale\n min:\n"+QString::number(Temp[0]/10*255/140));
		thread->useRangeMinValue(Temp[0]);
		thread->useRangeMaxValue(Temp[1]);
	})

//Se configura el comportamiento del sliders (deslizadores) de rango máximo de la ventana principal.
	QObject::connect(sliderMax, &QSlider::valueChanged, [&](uint16_t value) {
		autoRange->setChecked(false);
		Temp[1] = value;
		if(basicConfig[0]==0) tempMax->setText("Scale\n max:\n"+QString::number(Temp[1]*10));
		else if(basicConfig[0]== 1) tempMax->setText("Temp\n max:\n"+QString::number(Temp[1]/10));
		else tempMax->setText("scale\n max:\n"+QString::number(Temp[1]/10*255/140));
		thread->useRangeMinValue(Temp[0]);
		thread->useRangeMaxValue(Temp[1]);
	});

//////////////////////////basic config window///////////////////////////

// Se crea un boton que enviara los parametros de configuracion, el botón se llamara ‘aceptar’ y se le asigna un tamaño y posicion
	QPushButton *ConfigAccept = new QPushButton("Accept", configWidget);
	ConfigAccept->setGeometry(65, 260, 60, 30);

// Se crea un boton que cierra la ventana de configuracion y se le signa un tamaño y posicion
	QPushButton *configClose = new QPushButton("Close", configWidget);
	configClose->setGeometry(135, 260, 60, 30);

	//Se crea un botón con menú despegable para seleccionar el mapa de color
	QComboBox *colorMap=new QComboBox(configWidget);
	// Se le agrega posicion y tamaño
	colorMap->setGeometry(150,10,100,20);
	// Se agregan los items a la lista pegable
	colorMap->addItem("Rainbow");
	colorMap->addItem("Grayscale");
colorMap->addItem("Ironblack");
// Aquí se establece cuál de las opciones estará seleccionada por defecto. setCurrentIndex() (en este caso, "Grayscale")
	colorMap->setCurrentIndex(typeColormap-1);.
//Se mostrará un texto que mostrará la etiqueta es "ColorMa:". 
	QLabel *Colors = new QLabel("ColorMa:",configWidget);
//Se le agrega posicion y tamaño 
	Colors->setGeometry(10,10,100,20);

//Se repite el procedimiento que en el bloque anterior, ahora el boton es para seleccionar el formato de salida, se le agrega posicion y tamaño 
	QComboBox *formOut=new QComboBox(configWidget);
	formOut->setGeometry(150,40,100,20);
	formOut->addItem("Raw 14");
	formOut->addItem("Raw 16-K");
	formOut->addItem("AGC");
	formOut->setCurrentIndex(0);
	QLabel *formOutLabel = new QLabel("Format Out:",configWidget);
	formOutLabel->setGeometry(10,40,100,20);

// Se repite el procedimiento que en el bloque anterior, ahora el boton es para seleccionar el formato de ACG, se le agrega posicion y tamaño 
	QComboBox *agcFormat=new QComboBox(configWidget);
	agcFormat->setGeometry(150,70,100,20);
	agcFormat->addItem("Linear");
	agcFormat->addItem("HEQ");
	agcFormat->addItem("");
	agcFormat->setCurrentIndex(2);
	QLabel *agcFormatLabel = new QLabel("AGC Format:",configWidget);
	agcFormatLabel->setGeometry(10,70,100,20);

//Se repite el procedimiento que en el bloque anterior, ahora el boton es para seleccionar la ganancia, se le agrega posicion y tamaño
	QComboBox *Gain=new QComboBox(configWidget);
	Gain->setGeometry(150,100,100,20);
	Gain->addItem("High");
	Gain->addItem("Low");
	Gain->addItem("Auto");
	Gain->setCurrentIndex(0);
	QLabel *gainLabel = new QLabel("Gain:",configWidget);
	gainLabel->setGeometry(10,100,100,20);

// Se repite el procedimiento que en el bloque anterior, ahora el boton es para seleccionar el shutter 
	QComboBox *Shutter=new QComboBox(configWidget);
	Shutter->setGeometry(150,130,100,20);
	Shutter->addItem("Manual");
	Shutter->addItem("Auto");
	Shutter->setCurrentIndex(0);
	QLabel *shutterLabel = new QLabel("Shutter:",configWidget);
	shutterLabel->setGeometry(10,130,100,20);


	//Widget de entrada de texto que permite al usuario escribir texto.
	QLineEdit *emiBox = new QLineEdit(configWidget);
//Se cambia el tamaño de la caja de texto. Define que el ancho sea de 100 píxeles y la altura de 20 píxeles.
emiBox->setStyleSheet("QLineEdit { width: 100px; height: 20px; }");
	// Mueve la caja de texto a la posición (150, 160)
	emiBox->move(150,160);
//Se define una variable para la emisividad
//El valor se calcula dividiendo el valor de Radiometry.sceneEmissivity entre 8192.
	float emissivity =Radiometry.sceneEmissivity/8192.0;

// Aquí se establece el texto de la caja de texto (emiBox) con el valor de la emisividad calculada.
	emiBox->setText(QString::number(emissivity));
	//Se le asigna un nombre, posicion y tamaño a la caja de texto
	QLabel *emiLabel = new QLabel("Emissivity:",configWidget);
	emiLabel->setGeometry(10,160,80,20);

//Se conecta el botón ConfigAccept a una función lambda que será ejecutada cuando el botón sea presionado
	QObject::connect(ConfigAccept, &QPushButton::clicked, [&](){
//Abre un cuadro de diálogo de información mostrando el mensaje "Correct" en una ventana titulada "Information". 
	QMessageBox::information(nullptr,"Information","Correct");
	//Se configura el colormap
thread->useColormap(typeColormap);
		
// Esta función establece el formato de salida usando el valor almacenado en basicConfig[0].
		LEP_SetOutputFormat(basicConfig[0]);
// El formato de salida también se aplica al hilo de ejecución (thread) usando el mismo valor.
		thread->useOutputFormat(basicConfig[0]);
// Si el formato de salida es 2, se configuran los valores mínimos y máximos del "slider" en 0 y 1400 respectivamente.
// También se configuran los valores mínimos y máximos para el rango en el hilo de ejecución.
		if(basicConfig[0]==2){
			sliderMin->setValue(0);
			sliderMax->setValue(1400);
			thread->useRangeMinValue(0);
			thread->useRangeMaxValue(1400);
		}
else{
// Si el formato de salida no es 2, se utilizan los valores Temp[0] y Temp[1] para establecer el rango mínimo y máximo en el hilo.
			thread->useRangeMinValue(Temp[0]);
			thread->useRangeMaxValue(Temp[1]);			
		}

// Si el formato de salida es 2, se establece el formato de Auto Gain Control (AGC) usando el valor basicConfig[1].	
		if(basicConfig[0]==2) LEP_SetAGCEqu(basicConfig[1]);
		
		//Se configura el modo de ganancia con el valor basicConfig[2].
		LEP_SetGainMode(basicConfig[2]);	
		
		//Se establece el valor del obturador utilizando basicConfig[3].
		LEP_SetShutter(basicConfig[3]);
		
//Se obtiene el valor de emisividad desde el campo de texto emiBox y se convierte en un valor float.
		emissivity=emiBox->text().toFloat();
		//Se aplica la emisividad a la radiometría utilizando el valor obtenido.
		LEP_SetEmissivity(Radiometry, emissivity);

//El valor máximo de temperatura se actualiza dependiendo del valor de basicConfig[0]
//Si es 0, se multiplica el valor de Temp[1] por 10.
		if(basicConfig[0]==0) tempMax->setText("Scale\n max:\n"+QString::number(Temp[1]*10));
		//Si es 1, se divide el valor de Temp[1] por 10.
else if(basicConfig[0]== 1) tempMax->setText("Temp\n max:\n"+QString::number(Temp[1]/10));
//Si es otro valor, se realiza una operación adicional para calcular la escala máxima.
		else tempMax->setText("scale\n max:\n"+QString::number(Temp[1]/10*255/140));

//El valor mínimo de temperatura se actualiza dependiendo del valor de basicConfig[0]
		if(basicConfig[0]==0) tempMin->setText("Scale\n min:\n"+QString::number(Temp[0]*10));
		else if(basicConfig[0]== 1) tempMin->setText("Temp\n min:\n"+QString::number(Temp[0]/10));
		else tempMin->setText("scale\n min:\n"+QString::number(Temp[0]/10*255/140));		
	});	

// Conecta la señal currentIndexChanged del menú desplegable (QComboBox) colorMap a una función lambda. QOverload<int>::of se usa para manejar sobrecargas de funciones, asegurándose de que la señal maneje el tipo correcto
    QObject::connect(colorMap, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {
		typeColormap = index + 1;
    });

// Conecta el cambio de índice del menú desplegable formOut a una lambda.
// Actualiza la configuración del formato asignando el índice seleccionado a basicConfig[0]
	QObject::connect(formOut, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {
		basicConfig[0]=index;
// Si el índice seleccionado es 0 o 1, se configura el agcFormat en el índice 2.
		if(index<=1) agcFormat->setCurrentIndex(2);
		// Si el índice es mayor a 1, se configura agcFormat en el índice 0. 
		else agcFormat->setCurrentIndex(0);
    });

	// Conecta el cambio de índice en agcFormat (formato AGC) a la lambda
    QObject::connect(agcFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {
		basicConfig[1]=index;
		if(index!=2)formOut->setCurrentIndex(2);
    });

// Conecta el cambio de índice en Gain al evento de cambio de índice en el obturador.
	QObject::connect(Gain, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {
		basicConfig[2]=index;
    });
  
// Conecta el cambio de índice en el menú Shutter a una función lambda.
	QObject::connect(Shutter, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {
		basicConfig[3]=index;
    });

// Conecta el botón configClose a una lambda que se ejecuta cuando se hace clic en el botón.
	QObject::connect(configClose, &QPushButton::clicked, [&](){
		configWidget->close();
	});	

//////////////////////////Advanced Cconfig window///////////////////////
	//Se crea un widget  para Gain
    QWidget *gainTab = new QWidget(advConfigWidget);
    //Se crea un widget para la emisividad
    QWidget *emissivityTab = new QWidget(advConfigWidget);
//Se crea un widget  para AGC
    QWidget *agcTab = new QWidget(advConfigWidget);
    
    //Se agrega una nueva pestaña a cada widget
    advConfigWidget->addTab(gainTab, "Gain");
   advConfigWidget->addTab(emissivityTab, "Emissivity");
    advConfigWidget->addTab(agcTab, "AGC");


////////////////////////////// GAIN tab///////////////////////////

	//Se crea un layout de tipo grid (rejilla) para organizar los widgets dentro de la pestaña gainTab. Este layout posiciona los widgets en una cuadrícula.
    QGridLayout *layout_GAIN = new QGridLayout(gainTab);
	// Se crean dos vectores (QVector), uno para almacenar objetos QLineEdit y otro para etiquetas (QLabel). 
    QVector<QLineEdit*> lineEdits_GAIN;
    QVector<QLabel*> labels_GAIN;

//Se crean 8 etiquetas y 8 campos de texto.
    for (int i = 0; i < 8; ++i) {
        QLabel *label = new QLabel("Label " + QString::number(i + 1));
        QLineEdit *lineEdit = new QLineEdit;

        // Aquí se añaden los widgets creados al layout de tipo grid 	
        layout_GAIN->addWidget(label, (i % 4) * 2, i / 4 );
        layout_GAIN->addWidget(lineEdit, (i % 4) * 2 + 1, i / 4);

        // Los campos de texto y las etiquetas se almacenan en sus respectivos vectores para poder acceder a ellos más adelante.
        lineEdits_GAIN.append(lineEdit);
        labels_GAIN.append(label);
    }

   //Se cambia el nombre de las etiquetas y los valores de las cajas
    labels_GAIN[0]->setText("Start Column"); 
    labels_GAIN[1]->setText("Start Row");
    labels_GAIN[2]->setText("L_H Threshold");
    labels_GAIN[3]->setText("L_H Poblation");
    labels_GAIN[4]->setText("End Column");
    labels_GAIN[5]->setText("End Row");
    labels_GAIN[6]->setText("H_L Threshold");
    labels_GAIN[7]->setText("H_L Poblation");     

//Se asignan valores iniciales a los campos de texto, basados en las propiedades del objeto gainModeObj.
    lineEdits_GAIN[0]->setText(QString::number(gainModeObj.sysGainModeROI.startCol)); 
    lineEdits_GAIN[1]->setText(QString::number(gainModeObj.sysGainModeROI.startRow));
lineEdits_GAIN[2]->setText(QString::number(gainModeObj.sysGainModeThresholds.sys_C_low_to_high));    lineEdits_GAIN[3]->setText(QString::number(gainModeObj.sysGainModeThresholds.sys_P_low_to_high));
    lineEdits_GAIN[4]->setText(QString::number(gainModeObj.sysGainModeROI.endCol));
    lineEdits_GAIN[5]->setText(QString::number(gainModeObj.sysGainModeROI.endRow));
  lineEdits_GAIN[6]->setText(QString::number(gainModeObj.sysGainModeThresholds.sys_C_high_to_low));    lineEdits_GAIN[7]->setText(QString::number(gainModeObj.sysGainModeThresholds.sys_P_high_to_low)); 

//Se crean dos botones, uno para aceptar y el otro para borrar
    QPushButton *advAccept = new QPushButton("Accept");
    QPushButton *advClose = new QPushButton("Close");
    //Se agregan widgets a los botones y se les asigna una posición
    layout_GAIN->addWidget(advAccept, 9, 0, 2, 1); 
    layout_GAIN->addWidget(advClose, 9, 1, 2, 2); 

////////////////////////////// Emissivity tab///////////////////////////
//  Se crea un diseño de cuadrícula que organizará los elementos de manera estructurada en filas y columnas dentro de la pestaña llamada emissivityTab
	QGridLayout *layout_Emiss = new QGridLayout(emissivityTab);
	// Se crean vectores para almacenar objetos de tipo QLineEdit y QLabel
	QVector<QLineEdit*> lineEdits_Emiss;
	QVector<QLabel*> labels_Emiss;

 // Se crea una etiqueta con el texto "Label " seguido de un número (del 1 al 8).
    for (int i = 0; i < 8; ++i) {
QLabel *label = new QLabel("Label " + QString::number(i + 1));
// Se crea un campo de texto para que el usuario pueda ingresar o visualizar datos.
        QLineEdit *lineEdit = new QLineEdit;
        // Se crea un campo de texto (QLineEdit) para que el usuario pueda ingresar o visualizar datos.
        layout_Emiss->addWidget(label, (i % 4) * 2, i / 4 );
        layout_Emiss->addWidget(lineEdit, (i % 4) * 2 + 1, i / 4);
	    
// Coloca el campo de texto justo debajo de su correspondiente etiqueta.
        lineEdits_Emiss.append(lineEdit);
	labels_Emiss.append(label);
    }

    //Se cambian los textos de las 8 etiquetas creadas previamente 
    labels_Emiss[0]->setText("Emissivity");
    labels_Emiss[1]->setText("Background Temperature");
    labels_Emiss[2]->setText("Atmospheric Transmision");
    labels_Emiss[3]->setText("Atmospheric Temperature");
    labels_Emiss[4]->setText("Window Transmission");
    labels_Emiss[5]->setText("Window Reflection");
    labels_Emiss[6]->setText("Window Temperature");
    labels_Emiss[7]->setText("Window Reflected Temperature");   
    
    // Establece valores iniciales en los campos de texto con datos provenientes de un objeto Radiometry.
    lineEdits_Emiss[0]->setText(QString::number(Radiometry.sceneEmissivity/8192.0)); 
    lineEdits_Emiss[1]->setText(QString::number(Radiometry.TBkgK/100.0));
    lineEdits_Emiss[2]->setText(QString::number(Radiometry.tauWindow/8192.0));
    lineEdits_Emiss[3]->setText(QString::number(Radiometry.TAtmK/100.0));
    lineEdits_Emiss[4]->setText(QString::number(Radiometry.tauWindow/8192.0));
    lineEdits_Emiss[5]->setText(QString::number(Radiometry.reflWindow/8192.0));
    lineEdits_Emiss[6]->setText(QString::number(Radiometry.TWindowK/100.0));
    lineEdits_Emiss[7]->setText(QString::number(Radiometry.TReflK/100.0)); 
    
    //Se crean dos botones, uno para aceptar cambios y otro para cerrar la ventana o pestaña.
    QPushButton *advAccept1 = new QPushButton("Accept");
    QPushButton *advClose1 = new QPushButton("Close");
   //Se le asigna poscición a los botones creados
    layout_Emiss->addWidget(advAccept1, 9, 0, 2, 1); // Span 4 columns
    layout_Emiss->addWidget(advClose1, 9, 1, 2, 1); // Span 4 columns

////////////////////////////// AGC tab///////////////////////////

// Se crea un diseño de cuadrícula para organizar los elementos dentro de la pestaña llamada agcTab.
    QGridLayout *layout_AGC = new QGridLayout(agcTab);
    //Se crean vectores para almacenar objetos de campos de texto y etiquetas
    QVector<QLineEdit*> lineEdits_ACG;
    QVector<QLabel*> labels_AGC;

//Se crea una etiqueta con el texto "Label " seguido de un número del 0 al 7.
    for (int i = 0; i < 7; ++i) {
        QLabel *label = new QLabel("Label " + QString::number(i + 1));
//Se crea un campo de texto donde el usuario podrá ingresar o ver valores.
        QLineEdit *lineEdit = new QLineEdit;

        // Añade la etiqueta a la cuadrícula en una posición calculada según el índice i
        layout_AGC->addWidget(label, (i % 4) * 2, i / 4 );
        // Coloca el campo de texto debajo de la etiqueta.
        layout_AGC->addWidget(lineEdit, (i % 4) * 2 + 1, i / 4);

        // Se añaden las etiquetas y campos de texto a sus respectivos vectores.
        lineEdits_ACG.append(lineEdit);
	labels_AGC.append(label);
    }
    // Cambia los textos de las etiquetas creadas
    labels_AGC[0]->setText("Start Column");
    labels_AGC[1]->setText("Start Row");
    labels_AGC[2]->setText("Clip Limit High");
    labels_AGC[3]->setText("Dampening Factor ");
    labels_AGC[4]->setText("End Column");
    labels_AGC[5]->setText("End Row");
    labels_AGC[6]->setText("Linear Percent");
   
    // Establece valores iniciales en los campos de texto con datos extraídos de un objeto llamado agcROI 
    lineEdits_ACG[0]->setText(QString::number(agcROI.startCol)); 
    lineEdits_ACG[1]->setText(QString::number(agcROI.startRow));
    lineEdits_ACG[2]->setText(QString::number(agcHeqClipLimitHigh));
    lineEdits_ACG[3]->setText(QString::number(agcHeqDampingFactor));
    lineEdits_ACG[4]->setText(QString::number(agcROI.endCol));
    lineEdits_ACG[5]->setText(QString::number(agcROI.endRow));
    lineEdits_ACG[6]->setText(QString::number(agcHeqLinearPercent));
   //Se crean dos botones y se les asigna una posición
    QPushButton *advAccept2 = new QPushButton("Accept");
    QPushButton *advClose2 = new QPushButton("Close");
    layout_AGC->addWidget(advAccept2, 9, 0, 2, 1);
    layout_AGC->addWidget(advClose2, 9, 1, 2, 1); 

	
///////////////////////link action adv Config///////////////////////////
	
//Esta línea conecta el evento clicked del botón advAccept a una función lambda 
	QObject::connect(advAccept, &QPushButton::clicked, [&lineEdits_GAIN,&lineEdits_Emiss,&lineEdits_ACG]{

		gainModeObj.sysGainModeROI.startCol = lineEdits_GAIN[0]->text().toUInt();
		gainModeObj.sysGainModeThresholds.sys_C_low_to_high = lineEdits_GAIN[2]->text().toUInt();
		gainModeObj.sysGainModeThresholds.sys_P_low_to_high = lineEdits_GAIN[3]->text().toUInt();
		gainModeObj.sysGainModeROI.endCol = lineEdits_GAIN[4]->text().toUInt();
		gainModeObj.sysGainModeROI.endRow = lineEdits_GAIN[5]->text().toUInt();
		gainModeObj.sysGainModeThresholds.sys_C_high_to_low = lineEdits_GAIN[6]->text().toUInt();
		gainModeObj.sysGainModeThresholds.sys_P_high_to_low = lineEdits_GAIN[7]->text().toUInt();
//Convierte el valor de lineEdits_GAIN[6] a un valor flotante, lo convierte a grados Kelvin.
		gainModeObj.sysGainModeThresholds.sys_T_high_to_low = lineEdits_GAIN[6]->text().toFloat() + 273;
//Convierte el valor de lineEdits_GAIN[2] a flotante y lo convierte a Kelvin.
		gainModeObj.sysGainModeThresholds.sys_T_low_to_high = lineEdits_GAIN[2]->text().toFloat() + 273;
//Llama a la función LEP_SetGainConfig para configurar la ganancia con el objeto gainModeObj.
		LEP_SetGainConfig(gainModeObj);
		
		//Se establecen los parámetros radiométricos
		Radiometry.sceneEmissivity = lineEdits_Emiss[0]->text().toFloat()*8192.0; 
		Radiometry.TBkgK = lineEdits_Emiss[1]->text().toFloat()*100.0;
		Radiometry.tauWindow = lineEdits_Emiss[2]->text().toFloat()*8192.0;
		Radiometry.TAtmK = lineEdits_Emiss[3]->text().toFloat()*100.0;
		Radiometry.tauWindow = lineEdits_Emiss[4]->text().toFloat()*8192.0;
		Radiometry.reflWindow = lineEdits_Emiss[5]->text().toFloat()*8192.0;
		Radiometry.TWindowK = lineEdits_Emiss[6]->text().toFloat()*100.0;}
		Radiometry.TReflK = lineEdits_Emiss[7]->text().toFloat()*100.0; 
// Llama a la función LEP_SetRadParms para configurar los parámetros radiométricos con el objeto Radiometry
		LEP_SetRadParms(Radiometry);

	
		//Se establecen AGC
		agcROI.startCol = lineEdits_ACG[0]->text().toUInt();
		agcROI.startRow =lineEdits_ACG[1]->text().toUInt();
		agcHeqClipLimitHigh = lineEdits_ACG[2]->text().toUInt();
		agcHeqDampingFactor = lineEdits_ACG[3]->text().toUInt();
		agcROI.endCol = lineEdits_ACG[4]->text().toUInt();
		agcROI.endRow = lineEdits_ACG[5]->text().toUInt();
		agcHeqLinearPercent = lineEdits_ACG[6]->text().toUInt();
// Llama a la función LEP_SetAGCConfig para configurar el AGC con los parámetros capturados.
		LEP_SetAGCConfig(agcROI, agcHeqClipLimitHigh, agcHeqDampingFactor, agcHeqLinearPercent);

//Muestra una caja de mensaje indicando que la configuración fue exitosa.
	QMessageBox::information(nullptr,"Information","Correct");
	});   

//Conecta el evento clicked de advAccept1 
        QObject::connect(advAccept1, &QPushButton::clicked, [=]() {
		advAccept->clicked();});
//Hace lo mismo con el botón advAccept2.
	QObject::connect(advAccept2, &QPushButton::clicked, [&]() {
		advAccept->clicked();});

	// Conecta el botón advClose para cerrar la ventana advConfigWidget.
	QObject::connect(advClose, &QPushButton::clicked, [&](){
		advConfigWidget->close();
	});	

	// Hace lo mismo con advClose1.
	QObject::connect(advClose1, &QPushButton::clicked, [&](){
		advConfigWidget->close();
	});	

	//Hace lo mismo con advClose2.
	QObject::connect(advClose2, &QPushButton::clicked, [&](){
		advConfigWidget->close();
	});	

// ROI
	// Delara una variable llamada ROI que almacenara la imagen que representa el area de interes
		QImage ROI;
		//Crea una imagen de 640x480 pixeles en formato ARGB32, lo que significa que la imagen admite transparencia y colores en 32 bits.
		ROI = QImage(320*2, 240*2, QImage::Format_ARGB32);
		//Llena la imagen ROI con un color transparente para que tenga un fondo sin color.
		ROI.fill(Qt::transparent);
		//Se crea un objeto QPainter que permite dibujar sobre la imagen ROI.
		QPainter painter(&ROI);
		//Configura el lapiz de dibujo para que el color sea negro y el grosor de las lineas sea de 3 pixeles.
		painter.setPen(QPen(Qt::black,3));
		int x1 = 69*4;
		int y1 = 49*4;
		int x2 = 89*4;
		int y2 = 69*4;

		//Dibuja 4 lineas formando un rectangulo
		//Esa imagen se posiciona en la posicion (0,0)
		painter.drawLine(x1, y1, x2, y1);
		painter.drawLine(x1, y1, x1, y2);
		painter.drawLine(x1, y2, x2, y2);
		painter.drawLine(x2, y1, x2, y2);
		painter.drawImage(0,0,ROI);
		//Termina el proceso de dibujo
		painter.end();
		//Crea un objeto que mostrara la imagen en la interfaz
		QLabel myLabel1(myWidget);
		//Define geometria (posicion y tamanno)
		myLabel1.setGeometry(10, 10, 330*2, 240*2);
		// Convierte la imagen ROI en un QPixmap y lo asigna a la etiqueta myLabel1 para mostrarla en la interfaz. 
		myLabel1.setPixmap(QPixmap::fromImage(ROI));
    
	//Conecta el evento timeout del temporizador AI_timer a una funcion lambda que se ejecutara cada vez que el temporizador expire.
	QObject::connect(AI_timer,  &QTimer::timeout, [&](){
			
		// Define la ruta del archivo CSV
		QString csvFilePath = "/home/Thermal_Camera/output.csv";
		
		int array[64]; //acepta 16 bounding boxes
		int buff = 0;
		
		//Se maneja la lectura del archivo output.csv.
		QFile fileBBox(csvFilePath);
		if (!fileBBox.open(QIODevice::ReadOnly | QIODevice::Text)) {
			qDebug() << "Error: Unable to open the file for reading.";
		}

		   //Crea un objeto que se usara para leer el contenido del archivo
		QTextStream in(&fileBBox);
		//Se almacenara cada linea del archivo CSV mientras se lee.
		QString line;
		
		// Read and output each line of the file
		while (!in.atEnd()) {
			//Lee una linea completa edl archio CSV y la almacena en la variable line.
			line = in.readLine();
			 // Divide la linea en una lista de cadenas utilizando los espacios como separadores.
			QStringList numbersList = line.split(" ");
			for (const QString& number : numbersList) {
				if (buff < 64) {
					array[buff++] = number.toInt();
				}
			}
		}
	
		//Cierra el archivo CSV  que se abrio previamente para la lectura
		fileBBox.close();
		
		int box_count = buff / 4;
		
		if (buff % 4 != 0) //Se valida que sea un multiplo de 4 exacto
		{
			qDebug() << "Error: Bounding boxes incmpletos en CSV";
			return;
		}
		
		if (box_count > 4) box_count = 4;
		
		thread->setBBox(array, box_count);
		
		//Declara una variable QImage para crear la nueva imagen donde se dibujara el area de interes (ROI)
		QImage ROI;
		//Crea una imagen de tamanno 640x480 pixeles con formato ARGB32, que admite transparencia y color de 32 bits.
		ROI = QImage(320*2, 240*2, QImage::Format_ARGB32);
		//Llena la imagen con un color transparente para que el fondo no tenga color.
		ROI.fill(Qt::transparent);
		//Se crea un objeto QPainter para poder dibujar sobre la imagen ROI.
		QPainter painter(&ROI);
		//COnfigura el lapiz de dibujo para usar un color blanco y un grosor de 3 pixeles para las lineas que se dibujaran.
		painter.setPen(QPen(Qt::white,3));
		
		for( int i=0; i < box_count; i++){
			int idx = i*4;
			if (idx + 3 >= 64) break; //Proteccion para evitar segmentation fault
			
			int x1 = array[idx];
			int y1 = array[idx + 1];
			int x2 = array[idx + 2];
			int y2 = array[idx + 3];
			
			if (x1 < 0 || x1 >= 80 || x2 < 0 || x2 > 80 ||
			    y1 < 0 || y1 >= 60 || y2 < 0 || y2 > 60 ||
			    x2 <= x1 || y2 <= y1) {
					qDebug() << "Bounding box invalido" << x1 << y1 << x2 << y2;
					continue;
					}
			 
			 int w = x2 - x1;
			 int h = y2 - y1;
			 
			 painter.drawRect(x1*4, y1*4, w*4, h*4);
			 
			//if (w > 0 && h > 0) {
			//	painter.drawRect(x*4, y*4, w*4, h*4);
			//}
			//Dibuja un rectangulo en la imagen ROI. Las coordenadas x e y se escalan por 4
			//painter.drawRect(array[i]*4,array[i+1]*4,(array[i+2]-array[i])*4,(array[i+3]-array[i+1])*4);
			}

		// Termina el proceso de dibujo
		painter.end();
		// Convierte la imagen ROI en un QPixmap y la asigna a la etiqueta myLabel1, actualizando la interfaz grafica con el nuevo conteniido del ROI.
		myLabel1.setPixmap(QPixmap::fromImage(ROI));
		thread->Set_NROI(x1/4,x2/4,y1/4,y2/4);
	});
	
////////////////////////// init congig ////////////// //////////////////
	//Configura el formato de salida de la camara termica.
	LEP_SetOutputFormat(0);
	//Establece el formato de salida en el hilo thread, usando el mismo formato configurado en la camara termica (valor 0).
	thread->useOutputFormat(0);
			
	//Configura el modo de ganancia de la camara termica a 0.
	LEP_SetGainMode(0);	
		
	//Configura el obturador de la camara
	LEP_SetShutter(0);

	//Inicia el hlo thread
	thread->start();
	//Inicia otro hilo, file
	file->start();
	//Muestra la ventana principl en la interfaz grafica

	myWidget->show();
	
	return a.exec();
}
