// Se importan las funciones basicas que son necesarias para el uso del sensor termico FLIR LEPTON
#include "leptonSDKEmb32PUB/LEPTON_SDK.h"
#include "leptonSDKEmb32PUB/LEPTON_SYS.h"
#include "leptonSDKEmb32PUB/LEPTON_OEM.h"
#include "leptonSDKEmb32PUB/LEPTON_RAD.h"
#include "leptonSDKEmb32PUB/LEPTON_AGC.h"
#include "leptonSDKEmb32PUB/LEPTON_VID.h"
#include "leptonSDKEmb32PUB/LEPTON_Types.h"

//Se incluyen las herramientas necesarias para la toma de datos capturados por el sensor
#include <QDebug>
#include <cmath>

//Macros se aseguran que el encabezado solo se incluya una vez, evitando conflictos en la compilación.
#ifndef LEPTON_CONFIG
#define LEPTON_CONFIG

//Conecta el software con la cámara Lepton.
void LEP_Connect();
// Realiza una calibración de corrección de plano uniforme (FFC), que ajusta las diferencias entre píxeles para mejorar la imagen.
void LEP_PerformFFC();

//PARA Get Config
// Obtiene los parámetros radiométricos (como los relacionados con la temperatura).
void LEP_GetRadParms(LEP_RAD_FLUX_LINEAR_PARAMS_T_PTR Radiometry);
// Obtiene la configuración actual del modo de ganancia de la cámara.
void LEP_GetGainConfig(LEP_SYS_GAIN_MODE_OBJ_T_PTR gainModeObj);

// Obtiene la configuración de control automático de ganancia (AGC)
void LEP_GetAGCConfig(LEP_AGC_ROI_T_PTR agcROI, LEP_UINT16 
*agcHeqClipLimitHigh, LEP_UINT16 *agcHeqDampingFactor, LEP_UINT16 *agcHeqLinearPercent);

//PARA Basic Config 
//Se controlan y analizan las imágenes térmicas
void LEP_SetEmissivity(LEP_RAD_FLUX_LINEAR_PARAMS_T Radiometry, float emissivity);
void LEP_SetAGCEqu(int option);
void LEP_SetGainMode(int option);
void LEP_SetShutter(int option);.
void LEP_SetROI(LEP_SYS_VIDEO_ROI_T ROI, LEP_RAD_ROI_T ROI_1);
void LEP_ROIStatistics(LEP_SYS_SCENE_STATISTICS_T_PTR ROI_Stats, LEP_RAD_SPOTMETER_OBJ_KELVIN_T_PTR ROI_Stats_1);

// Configura la salida de video en formato RAW de 14 bits, que proporciona datos térmicos sin procesar.
void LEP_SetRaw14();

//PARA Advanced Config
// Ajusta parámetros radiométricos avanzados.
void LEP_SetRadParms(LEP_RAD_FLUX_LINEAR_PARAMS_T Radiometry);
// Establece configuraciones personalizadas del modo de ganancia.
void LEP_SetGainConfig(LEP_SYS_GAIN_MODE_OBJ_T gainModeObj);
// Configura manualmente el AGC.
void LEP_SetAGCConfig(LEP_AGC_ROI_T agcROI, LEP_UINT16 agcHeqClipLimitHigh, LEP_UINT16 agcHeqDampingFactor, LEP_UINT16 agcHeqLinearPercent);

// Reinicia la cámara Lepton.
void LEP_Reboot();

// Finaliza la definición del encabezado (#ifndef LEPTON_CONFIG).
#endif


