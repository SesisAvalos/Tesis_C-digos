//Se incluye LEPTON_CONFIG.h
#include "LEPTON_CONFIG.h"

//Se maneja la comunicación y calibración de la cámara térmica a través de su API 
bool _connected;
LEP_CAMERA_PORT_DESC_T _port;

void LEP_Connect() {
	LEP_OpenPort(1, LEP_CCI_TWI, 400, &_port); //Abre la conexión con la cámara
	_connected = true;
}

void LEP_PerformFFC() {
	if(!_connected) {
		LEP_Connect();
	}
	while(LEP_RunSysFFCNormalization(&_port)); //Corrije el ruido térmico interno del sensor 
	qDebug()<<"FCC Done";

}

//PARA Get Config 
//Se obtienen los parámetros de radiometría
void LEP_GetRadParms(LEP_RAD_FLUX_LINEAR_PARAMS_T_PTR Radiometry){
	if(!_connected) {
		LEP_Connect();
	}
	LEP_GetRadFluxLinearParams(&_port,Radiometry);	
}

//Se obtiene la configuración de ganancia, la ganancia afecta cómo la cámara ajusta la sensibilidad térmica
void LEP_GetGainConfig(LEP_SYS_GAIN_MODE_OBJ_T_PTR gainModeObj){
	if(!_connected) {
		LEP_Connect();
	}
	LEP_GetSysGainModeObj(&_port,gainModeObj);
}

//Se obtienen los valores de AGC (Automatic Gain Control), esto hace que ls imágenes se vean mejor
void LEP_GetAGCConfig(LEP_AGC_ROI_T_PTR agcROI, LEP_UINT16 *agcHeqClipLimitHigh, LEP_UINT16 *agcHeqDampingFactor, LEP_UINT16 *agcHeqLinearPercent){		
	if(!_connected) {
		LEP_Connect();
	}
	
	LEP_GetAgcROI( &_port, agcROI);
	LEP_GetAgcHeqDampingFactor( &_port, agcHeqDampingFactor);
	LEP_GetAgcHeqClipLimitHigh( &_port, agcHeqClipLimitHigh);
	LEP_GetAgcHeqLinearPercent( &_port, agcHeqLinearPercent);
}


//PARA Basic Config
//Se calcula el valor de la emisividad 
//Se corrigen las mediciones de temperatura de la cámara de acuerdo al material que observa
void LEP_SetEmissivity(LEP_RAD_FLUX_LINEAR_PARAMS_T Radiometry, float emissivity){
	if(!_connected) {
		LEP_Connect();
	}
	
	Radiometry.sceneEmissivity=round(8192*emissivity);
	LEP_SetRadFluxLinearParams(&_port,Radiometry);
	LEP_GetRadFluxLinearParams(&_port,&Radiometry);	
}

//Se cambia el formato de salida
void LEP_SetOutputFormat(int option){
	if(!_connected) {
		LEP_Connect();
	}
	
//Se definen variables para controlar los parámetros anteriores si es que están activados
	LEP_RAD_ENABLE_E EnableState_Rad;
	LEP_RAD_ENABLE_E EnableState_TLinear;
	LEP_AGC_ENABLE_E agcEnableState;
	
//Se habilita la radiometría y se aplica en la cámara	
	EnableState_Rad = LEP_RAD_ENABLE; 
//Se configura la cámara de tres formas posibles
	LEP_SetRadEnableState(&_port,EnableState_Rad);
	switch(option){
		case 0: 
			EnableState_TLinear = LEP_RAD_DISABLE; 
			agcEnableState=LEP_AGC_DISABLE; 
			break;
		case 1: 
			EnableState_TLinear = LEP_RAD_ENABLE; 
			agcEnableState=LEP_AGC_DISABLE; 
			break;
		case 2: 
			EnableState_TLinear = LEP_RAD_DISABLE;

			LEP_VID_FOCUS_CALC_ENABLE_E vidFocusCalcEnableState =
					    LEP_VID_FOCUS_CALC_DISABLE;
			LEP_SetVidFocusCalcEnableState( &_port,
                                            vidFocusCalcEnableState);
            
			agcEnableState=LEP_AGC_ENABLE;
            break;
		}
//Se activa la configuración de TLinear 
		LEP_SetRadTLinearEnableState(&_port,EnableState_TLinear);
//Se activa o desactiva AGC
		LEP_SetAgcEnableState( &_port, agcEnableState);
}

//Se configura la política de ajuste de ganancia automática AGC
void LEP_SetAGCEqu(int option){
	if(!_connected) {
		LEP_Connect();
	}
	
	LEP_AGC_POLICY_E agcPolicy = LEP_AGC_HEQ;
	
	switch (option) {
		case 0: //AGC lineal
			agcPolicy = LEP_AGC_LINEAR;
			break;
		case 1: //AGC con ecualización de histograma
			agcPolicy = LEP_AGC_HEQ;
			break;
		default: //Se usa AGC HEQ por defecto
			agcPolicy = LEP_AGC_HEQ;
			break;
	}
//Se aplica la configuración en la cámara 
	LEP_SetAgcPolicy( &_port, agcPolicy );
	
}

//Se configura el modo de ganancia 
void LEP_SetGainMode(int option){
	if(!_connected) { 
		LEP_Connect();
	}
	//Se asigna el modo de ganancia correspondiente
	LEP_SYS_GAIN_MODE_E gainMode = LEP_SYS_GAIN_MODE_HIGH;

    switch (option) {
		case 0:
			gainMode=LEP_SYS_GAIN_MODE_HIGH;
			break;
		case 1:
			gainMode=LEP_SYS_GAIN_MODE_LOW;
			break;
		case 2:
			gainMode=LEP_SYS_GAIN_MODE_AUTO;
			break;
	 }
	 LEP_SetSysGainMode( &_port, gainMode);
	 
}

//Se configura el modo del obturador de la cámara térmica
void LEP_SetShutter(int option){
	if(!_connected) {
		LEP_Connect();
	}
	LEP_SYS_FFC_SHUTTER_MODE_OBJ_T gainModeObj;
	LEP_GetSysFfcShutterModeObj( &_port,&gainModeObj );
	
	switch (option) {
		case 0: //Modo manual
			gainModeObj.shutterMode=LEP_SYS_FFC_SHUTTER_MODE_MANUAL;
			break;
		case 1: //Modo automático
			gainModeObj.shutterMode=LEP_SYS_FFC_SHUTTER_MODE_AUTO;
			break;	
	}
	LEP_SetSysFfcShutterModeObj( &_port, gainModeObj);
}

//PARA ROI Config
//Se configuran las zonas re interés (ROI) 
void LEP_SetROI(LEP_SYS_VIDEO_ROI_T ROI, LEP_RAD_ROI_T ROI_1){
	if(!_connected) {
		LEP_Connect();
	}
	LEP_SetSysSceneRoi(&_port,ROI); //Se establece la región de interés en la cámara térmica
	LEP_SetRadSpotmeterRoi(&_port,ROI_1); //Se establece el ROI en el spotmeter
}
//Se obtienen las estadísticas de la región de interés 
void LEP_ROIStatistics(LEP_SYS_SCENE_STATISTICS_T_PTR ROI_Stats,LEP_RAD_SPOTMETER_OBJ_KELVIN_T_PTR ROI_Stats_1){
	if(!_connected) {
		LEP_Connect();
	}
	LEP_GetSysSceneStatistics(&_port,ROI_Stats); //Se obtienen las estadísticas de la escena
	LEP_GetRadSpotmeterObjInKelvinX100(&_port, ROI_Stats_1); //Se obtienen las mediciones térmicas de un área (en Kelvin multiplicado por 100)
}

//PARA Advanced Config
void LEP_SetRadParms(LEP_RAD_FLUX_LINEAR_PARAMS_T Radiometry){
	if(!_connected) {
		LEP_Connect();
	}
		

	LEP_SetRadFluxLinearParams(&_port,Radiometry); //Se configuran los parámetros de radiometría 
}

//Se configura el modo de ganancia del sistema
void LEP_SetGainConfig(LEP_SYS_GAIN_MODE_OBJ_T gainModeObj){
	if(!_connected) {
		LEP_Connect();
	}
	LEP_SetSysGainModeObj(&_port,gainModeObj);
}

void LEP_SetAGCConfig(LEP_AGC_ROI_T agcROI, LEP_UINT16 agcHeqClipLimitHigh, LEP_UINT16 agcHeqDampingFactor, LEP_UINT16 agcHeqLinearPercent){		
	if(!_connected) {
		LEP_Connect();
	}

//Se definen funciones que ajustan parámetros relacionados con el AGC
	LEP_SetAgcROI( &_port, agcROI);
	LEP_SetAgcHeqDampingFactor( &_port, agcHeqDampingFactor);
	LEP_SetAgcHeqClipLimitHigh( &_port, agcHeqClipLimitHigh);
	LEP_SetAgcHeqLinearPercent( &_port, agcHeqLinearPercent);
}

//Se reinicia el dispitivo
void LEP_Reboot() {
	if(!_connected) {
		LEP_Connect();
	}
	LEP_RunOemReboot(&_port);
	qDebug()<< "camera restart";
}
