/*=============================================================================
 * Author: Fabian Burgos
 * Date: 19/10/2022 
 * Board: Arduino Uno
 * Entorno de programacion: Arduino - IDE
 *
 * Descripcion: Codigo ejemplo de uso de libreria para envio y recepcion de  
 *              variable float a thingspeak con conexion a internet con modulo
 *              ESP8266 con comandos AT.
 *===========================================================================*/
 
#include "Esp_8266_thingspeak.h"                                                              //Incluyo librerias

void setup() {
  Serial.begin(9600);                                                                         //Inicializo puerto serie para debug
  pinMode(13,OUTPUT);                                                                         //Pin 13 como salida
  
  Serial.println(F("Conectando al wifi, porfavor espere..."));                                //Escribo en puerto serie 
  
  bool resultado=WIFI_conect_thingspeak("NOMBRE_RED", "CONTRASEÑA");                          //Conecto a red WIFI
  
  if (resultado==true){                                                                       //Si se pudo conectar
    Serial.println(F("Se conecto al wifi con exito"));  
  }else{                                                                                      //Si no se pudo conectar
    Serial.println(F("No se pudo conectar al wifi"));
  }
}

void loop() {
  Serial.println(F("\nBuscando dato en la nube"));                                           //Escribo en puerto serie
  float dato= recibir_variable_float_thingspeak(F("FZFRIFPYVFI1LK4M"), F("4"), F("1898597"));//Recibo dato float del campo "4" del servidor thingspeak con el key "FZFRIFPYVFI1LK4M" canal "1898597"

  if(dato==9999.99){                                                                         //Si no se pudo leer el dato, o los datos consultados no contienen informacion, la funcion retorna 9999.99
    Serial.println(F("Dato erroneo"));                                                       //Escribo en puerto serie
  }else{                                                                                     //Si se pudo leer el dato
    Serial.print(F("Retorno ")); Serial.println(dato);                                       //Envio el dato recibido por serial
    if(dato>0){                                                                              //Si el dato es mayor a cero
      digitalWrite(13,HIGH);                                                                 //Activo salida
    }else{                                                                                   //Sino
      digitalWrite(13,LOW);                                                                  //Apago salida
    }
  }
  delay(500);
  
  float Voltaje=analogRead(A0)*5.0/1023.0;                                                   //Convierto señal analogica a tension
  Serial.println(F("\nSubiendo dato a la nube"));                                            //Escribo en puerto serie
  Serial.println(Voltaje);
  enviar_variable_thingspeak(F("NSYD4SZFHC8LT18P"), F("1"), Voltaje);                        //Envio el valor de voltaje al field "1" del servidor con key "NSYD4SZFHC8LT18P"
  delay(500);
  
}
