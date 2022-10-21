/*=============================================================================
 * Author: Fabian Burgos
 * Date: 19/10/2022 
 * Board: Arduino Uno
 * Entorno de programacion: Arduino - IDE
 *
 * Descripcion: Libreria para envio y recepcion de variable float a thingspeak 
 *              con conexion a internet con modulo ESP8266 con comandos AT
 *===========================================================================*/
#include "Arduino.h"

/*=====================[Prototipos de funciones]================================*/
bool WIFI_conect_thingspeak(String NOMBRE_RED="",String PASS_RED="");
bool enviar_variable_thingspeak(String Api_key, String field, float valor=0.0);
float recibir_variable_float_thingspeak(String Api_key, String field, String channel);
