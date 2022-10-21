/*=============================================================================
 * Author: Fabian Burgos
 * Date: 19/10/2022 
 * Board: Arduino Uno
 * Entorno de programacion: Arduino - IDE
 *
 * Descripcion: Libreria para envio y recepcion de variable float a thingspeak 
 *              con conexion a internet con modulo ESP8266 con comandos AT
 *===========================================================================*/
 
/*=====================[ Inclusiones ]============================*/
#include "Arduino.h"
#include "Esp_8266_thingspeak.h"
#include <SoftwareSerial.h>
#include <stdio.h>
#include <string.h>

SoftwareSerial ESP8266_Serial(3, 2); // RX , TX

/*=====================[Definiciones]================================*/
#define OK 1
#define NOTOK 2
#define TIMEOUT 3
#define debug   0

/*=====================[Implementaciones]==============================*/

/*========================================================================
  Funcion: Moduleread()
  Descripcion: Obtiene dato del software serial
  Sin parametro de entrada
  Retorna:     Data obtenido en tipo string
  ========================================================================*/  
String Moduleread() {
  String reply = "";
  if (ESP8266_Serial.available())  {
    reply = ESP8266_Serial.readString();
  }
  return reply;
}

/*========================================================================
  Funcion: ModulewaitFor
  Descripcion: Espera hasta obtener el dato deseado o hasta que pase un tiempo determinado
  Parametro de entrada: String response1: Texto que se desea encontrar en la respuesta del modulo al comando AT
                        String response2: Texto alternativo que se desea encontrar en la respuesta del modulo al comando AT
                        unsigned int timeOut: Tiempo de espera necesario para verificar si se encuentran las palabras indicadas.
                                              En caso de superar este tiempo, se retorna 0.
  Retorna:     1 = si se encontro el dato deseado
               0 = si no se encontro el dato deseado y paso el tiempo indicado
  ========================================================================*/  
byte ModulewaitFor(String response1, String response2, unsigned int timeOut) {
  unsigned long entry = millis();
  String reply = Moduleread();
  byte retVal = 99;
  do {
    reply = Moduleread();
    if (reply != "") {
      if(debug==1){
          Serial.print((millis() - entry));
          Serial.print(F(" ms "));
          Serial.println(reply); 
        }
    }
  } while ((reply.indexOf(response1) + reply.indexOf(response2) == -2) && millis() - entry < timeOut );
  if ((millis() - entry) >= timeOut) {
    retVal = TIMEOUT;
  } else {
    if (reply.indexOf(response1) + reply.indexOf(response2) > -2) retVal = OK;
    else retVal = NOTOK;
  }
  return retVal;
}


/*========================================================================
  Funcion: Modulecommand
  Descripcion: Envia el comando AT y espera hasta obtener el dato deseado o hasta que pase un tiempo determinado y reintenta x veces
  Parametro de entrada: String command:   Comando AT a enviar al modulo
                        String response1: Texto que se desea encontrar en la respuesta del modulo al comando AT
                        String response2: Texto alternativo que se desea encontrar en la respuesta del modulo al comando AT
                        unsigned int timeOut: Tiempo de espera necesario para verificar si se encuentran las palabras indicadas.
                                              En caso de superar este tiempo, se retorna 0.
                        int repetitions:  Cantidad de veces que se desea repetir el proceso de busqueda.
  Retorna:     1 = si se encontro el dato deseado
               0 = si no se encontro el dato deseado y paso el tiempo indicado
  ========================================================================*/  
byte Modulecommand(String command, String response1, String response2, int timeOut, int repetitions) {
  if(debug==1){Serial.println(command);}
  ESP8266_Serial.println(command);
  byte returnValue = NOTOK;
  byte count = 0;
  while (count < repetitions && returnValue != OK) {
    if (ModulewaitFor(response1, response2, timeOut) == OK) {
      returnValue = OK;
    } else {
      returnValue = NOTOK;
    }
    count++;
  }
  return returnValue;
}


/*========================================================================
  Funcion: Modulecommand_sinespuesta
  Descripcion: Envia el comando AT
  Parametro de entrada: String command:   Comando AT a enviar al modulo
  Retorna:     1
  ========================================================================*/  
byte Modulecommand_sinespuesta(String command) {
  if(debug==1){Serial.println(command);}
  ESP8266_Serial.println(command);
  byte returnValue = OK;
  return returnValue;
}

/*========================================================================
  Funcion: WIFI_conect_thingspeak
  Descripcion: Conecta el modulo a una red wifi mediante comando AT
  Parametro de entrada: String NOMBRE_RED: Nombre de la red a la cual se desea conectar
                        String PASS_RED:   Contraseña de la red a la cual se desea conectar.
  Retorna:     true:  Si se pudo conectar
               fals:  Si no se pudo conectar
  ========================================================================*/ 
bool WIFI_conect_thingspeak(String NOMBRE_RED="",String PASS_RED=""){
  ESP8266_Serial.begin(9600);
  int res1=Modulecommand(F("AT+CWMODE=3"), "OK", "yy", 10000, 2);                                                  //Modo cliente Y AP 
  int res2=Modulecommand("AT+CWJAP=\""+NOMBRE_RED+"\",\""+PASS_RED+"\"", "OK", "yy", 10000, 2);                    //Conectando a wifi
  int res3=Modulecommand(F("AT+CIPMUX=1"), "OK", "yy", 10000, 2);                                                  //Multiples conexiones
  if((res1+res2+res3)==3){
    return true;
  }else{
    return false;
  }
}


/*========================================================================
  Funcion: enviar_variable_thingspeak
  Descripcion: Envia una variable float a un campo especifico del servidor thingspeak
  Parametro de entrada: String Api_key: Api_kay del thingspeak
                        String fiel:    Campo al cual se quiere modificar el valor
                        float valor:    Valor a enviar al servidor.
  Retorna:     true:  Si se pudo enviar
               false:  Si no se pudo enviar
  ========================================================================*/ 
bool enviar_variable_thingspeak(String Api_key, String field, float valor=0.0){
    String texto_enviar="GET /update?api_key="+Api_key+"&field"+field+"="+(String) valor+" HTTP/1.1\r\nHost: api.thingspeak.com\r\nConnection: close\r\n\r\n";
    Modulecommand(F("AT+CIPSTART=3,\"TCP\",\"api.thingspeak.com\",80"), "OK", "yy", 5000, 2);           //Conecto a servidor por TCP por conexion 1
    Modulecommand("AT+CIPSEND=3,"+(String)(texto_enviar.length()+3), "OK", ">", 5000, 2);               //Establesco la cantidad de datos a enviar por conexion 1
    Modulecommand_sinespuesta(texto_enviar);                                                            //Envio datos
    byte resultado = Modulecommand(" ", "200 OK", "yy", 500, 5);        
    //Modulecommand("AT+CIPCLOSE=3", "OK", "yy", 5000, 5);                       
    if(resultado==OK){
      return true;
    }else{
      return false;
    }
    
}


/*========================================================================
  Funcion: recibir_variable_float_thingspeak
  Descripcion: Recibe una variable float de un campo especifico del servidor thingspeak
  Parametro de entrada: String Api_key: Api_kay del thingspeak
                        String fiel:    Campo al cual se quiere modificar el valor
                        String channel: Canal del cual se desea obtener el valor del servidor thingspeak
  Retorna:     1:  Si se pudo enviar
               0:  Si no se pudo enviar
  ========================================================================*/ 
float recibir_variable_float_thingspeak(String Api_key, String field, String channel){
    int cant_resultados=3;                                                                              //Cantidad de resultados que se quieren preguntar al servidor
    String texto_enviar="GET /channels/"+channel+"/fields/"+field+".csv?api_key="+Api_key+"&results="+(String)cant_resultados+" HTTP/1.1\r\nHost: api.thingspeak.com\r\nConnection: close\r\n\r\n";
    Modulecommand(F("AT+CIPSTART=1,\"TCP\",\"api.thingspeak.com\",80"), "OK", "yy", 5000, 2);           //Conecto a servidor por TCP por conexion 4
    Modulecommand("AT+CIPSEND=1,"+(String)(texto_enviar.length()+2), "OK", ">", 5000, 2);               //Establesco la cantidad de datos a enviar por conexion 4
    Modulecommand_sinespuesta(texto_enviar);                                                            //Envio datos
    float variable=9999.99;                                                                             //Variable a retornar
    bool estado=true;                                                                                   //Variable para manejar el while
    
    while(estado){
      if(ESP8266_Serial.available())  {                                                                 //Obtengo respuesta del servidor
        delay(100);
        String reply = ESP8266_Serial.readString();
        
        if (reply.indexOf("created_at")>=0){                                                            //Si la respuesta contiene "created_at"
          int posicion_inicial=reply.indexOf("created_at");                                             //Obtengo posicion de la palabra "created_at"
        
          reply.remove(0,posicion_inicial);                                                             //Borro mensaje recibido hasta el texto "created_at"
          reply.remove(reply.indexOf("+IPD"));                                                          //Borro mensaje recibido desde el texto "+IPD" hasta el final
          reply.trim();                                                                                 //Borro espacios en blanco
          //Serial.println(reply);
          /* Hasta aca obtengo mensaje tipo:
           * created_at,entry_id,field4
           * 2022-10-19 19:37:00 UTC,30,
           * 2022-10-19 19:40:33 UTC,31,
           * 2022-10-19 19:42:38 UTC,32,
           */
           //Modifico el string con los datos
           reply.remove(0,reply.indexOf("\n")+1);                                                       //Remuevo primera linea "created_at,entry_id,field4"
           reply.replace(" ", "");                                                                      //Borro espacios vacios
           reply.replace("\r", "");                                                                     //Borro retorno de carros
           if(debug==1){Serial.println(reply);}
           
           //Verifico que halla recibido correctamente el string contando la cantidad de comas que deberian llegar
           int cant_comas=0;
           for(int i=0; i<reply.length(); i++){
             if(reply[i] == ','){
                cant_comas++;
              }
            }
            
            if(cant_comas!=(2*cant_resultados)){                                                       //El formato que devuelve el servidor son dos comas por cada resultado
              Serial.println(F("Error, verificar que se tenga buena conexion a internet"));
              break;
            }
            
           //Ahora separo cada fila
           String Separado[cant_resultados];
           for(int i=0; i<cant_resultados; i++){
            Separado[i]=reply.substring(0, reply.indexOf("\n"));                                       //Obtengo solo la primer fila
            reply.remove(0,reply.indexOf("\n")+1);                                                     //Remuevo la primer fila que ya pase a Separado[i]  
           }

           /*
           //Ahora separo las columnas
           String Columnas[8][3];
           for(int i=0; i<cant_resultados; i++){
            Columnas[i][0]=Separado[i].substring(0, Separado[i].indexOf(","));                        //Obtengo la columna de la fila i
            Separado[i].remove(0,Separado[i].indexOf(",")+1);                                         //Remuevo la columna de la fila i   
            Columnas[i][1]=Separado[i].substring(0, Separado[i].indexOf(","));                        //Obtengo la columna de la fila i
            Separado[i].remove(0,Separado[i].indexOf(",")+1);                                         //Remuevo la columna de la fila i 
            Columnas[i][2]=Separado[i];
           }
           //Ahora separo las columnas
           String Columnas[cant_resultados][3];
           for(int i=0; i<cant_resultados; i++){
            Columnas[i][0]=Separado[i].substring(0, Separado[i].indexOf(","));                        //Obtengo la columna de la fila i
            Separado[i].remove(0,Separado[i].indexOf(",")+1);                                         //Remuevo la columna de la fila i   
            Columnas[i][1]=Separado[i].substring(0, Separado[i].indexOf(","));                        //Obtengo la columna de la fila i
            Separado[i].remove(0,Separado[i].indexOf(",")+1);                                         //Remuevo la columna de la fila i 
            Columnas[i][2]=Separado[i];
           }

           //Muestro los valores separados
           if(debug==1){
             for(int v=0; v<cant_resultados; v++){
               Serial.print(v); Serial.print(" - "); Serial.println(Columnas[v][2]);
             }
           }
           
           //Verifico si existe dato y mando el mas actual de los 3
           for(int k=cant_resultados-1; k>=0; k--){
             if(Columnas[k][2]!=""){
                variable=Columnas[k][2].toFloat();
                break;
             }
           }
           */
           
           
           for(int i=0; i<cant_resultados; i++){
            Separado[i].substring(0, Separado[i].indexOf(","));                        //Obtengo la columna de la fila i
            Separado[i].remove(0,Separado[i].indexOf(",")+1);                                         //Remuevo la columna de la fila i   
            Separado[i].substring(0, Separado[i].indexOf(","));                        //Obtengo la columna de la fila i
            Separado[i].remove(0,Separado[i].indexOf(",")+1);                                         //Remuevo la columna de la fila i 
            }
            
            if(debug==1){
             for(int v=0; v<cant_resultados; v++){
               Serial.print(v); Serial.print(" - "); Serial.println(Separado[v]);
             }
            }

            //Verifico si existe dato y mando el mas actual de los 3
            for(int k=cant_resultados-1; k>=0; k--){
              if(Separado[k]!=""){
                 variable=Separado[k].toFloat();
                 break;
              }
            }
           
          estado=false;     //Salgo del While
        }
      }  
    }  
    
    return variable;
}
