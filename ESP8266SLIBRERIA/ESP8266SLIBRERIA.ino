#include <SoftwareSerial.h>

//Librerias para bmp180
#include <SFE_BMP180.h>
#include <Wire.h>

SFE_BMP180 pressure;
String servidor = "192.168.0.100";


SoftwareSerial SerialESP8266(11,10);  //RX, TX

String cadena="";

// Temperatura




void setup() {

  //------------------------------------------------------
  SerialESP8266.begin(19200);
  Serial.begin(19200);
  SerialESP8266.setTimeout(2000);

  SerialESP8266.println("AT");
  if(SerialESP8266.find("OK"))
    Serial.println(F("Respuesta AT correcto"));
  else
    Serial.println(F("Error en ESP8266"));


    //-----Configuración de red-------//Podemos comentar si el ESP ya está configurado

    //ESP8266 en modo estación (nos conectaremos a una red existente)
    SerialESP8266.println("AT+CWMODE=1");
    if(SerialESP8266.find("OK"))
      Serial.println(F("ESP8266 en modo Estacion"));
      
    //Nos conectamos a una red wifi 
    SerialESP8266.println("AT+CWJAP=\"Chichi\",\"75729972\"");
   // SerialESP8266.println("AT+CWJAP=\"CEISUFRO\",\"DCI.2016\"");
    Serial.println(F("Conectandose a la red ..."));
    SerialESP8266.setTimeout(10000); //Aumentar si demora la conexion
    if(SerialESP8266.find("OK"))
      Serial.println(F("WIFI conectado"));
    else
      Serial.println(F("Error al conectarse en la red"));
    SerialESP8266.setTimeout(2000);
    //Desabilitamos las conexiones multiples
    SerialESP8266.println("AT+CIPMUX=0");
    if(SerialESP8266.find("OK"))
      Serial.println(F("Multiconexiones deshabilitadas"));
    
  //------fin de configuracion-------------------


  // Initialize the sensor (it is important to get calibration values stored on the device).

  if (pressure.begin())
    Serial.println(F("BMP180 init success"));
  

  // Get the baseline pressure:
  
  double baseline = getPressure();
  
  Serial.print(baseline);

  delay(1000);

}

void loop() {
       

       

         String valorUv = String(analogRead(0));
         String valorMonoxido = String(analogRead(1));
         String valorTm = String(getPressure());
        
         //---------enviamos las variables al servidor---------------------
  
      //Nos conectamos con el servidor:
      
      SerialESP8266.println("AT+CIPSTART=\"TCP\",\"" + servidor + "\",8888");
      
      if( SerialESP8266.find("OK"))
      {  
    
          Serial.println();
          Serial.println("ESP8266 conectado con el servidor...");             

    
          //Armamos el encabezado de la peticion http
       
 
         String peticionHTTP= "GET /Arduino/ProyectoTUM/public/api/controladores/TUM-01/"+valorTm+"/"+valorMonoxido+"/"+valorUv+"/2.0/2.0 HTTP/1.1\r\nHost: 192.168.0.18\r\n\r\n";

          
     
          //Enviamos el tamaño en caracteres de la peticion http:  
          SerialESP8266.print("AT+CIPSEND=");
          SerialESP8266.println(peticionHTTP.length());
          Serial.print(peticionHTTP.length());
          //esperamos a ">" para enviar la petcion  http
          if(SerialESP8266.find(">")) // ">" indica que podemos enviar la peticion http
          {
            Serial.println(F("Enviando HTTP . . ."));
            SerialESP8266.println(peticionHTTP);
            if( SerialESP8266.find("SEND OK"))
            {  
              Serial.println(F("Peticion HTTP enviada:"));
              
              Serial.println(peticionHTTP);
              Serial.println(F("Esperando respuesta..."));
              
              boolean fin_respuesta=false; 
              long tiempo_inicio=millis(); 
              cadena="";
              
              while(fin_respuesta==false)
              {
                  while(SerialESP8266.available()>0) 
                  {
                      char c=SerialESP8266.read();
                      Serial.write(c);
                      cadena.concat(c);  //guardamos la respuesta en el string "cadena"
                  }
                  //finalizamos si la respuesta es mayor a 500 caracteres
                  if(cadena.length()>500) 
                  {
                    Serial.println(F("La respuesta a excedido el tamaño maximo"));
                    
                    SerialESP8266.println("AT+CIPCLOSE");
                    if( SerialESP8266.find("OK"))
                      Serial.println(F("Conexion finalizada"));
                    fin_respuesta=true;
                  }
                  if((millis()-tiempo_inicio)>100) //Finalizamos si ya han transcurrido 10 seg
                  {
                    Serial.println(F("Tiempo de espera agotado"));
                    SerialESP8266.println("AT+CIPCLOSE");
                    if( SerialESP8266.find("OK"))
                      Serial.println(F("Conexion finalizada"));
                    fin_respuesta=true;
                  }
                  if(cadena.indexOf("CLOSED")>0) //si recibimos un CLOSED significa que ha finalizado la respuesta
                  {
                    Serial.println();
                    Serial.println(F("Cadena recibida correctamente, conexion finalizada"));         
                    fin_respuesta=true;
                  }
              }
    
              
            }
            else
            {
              Serial.println(F("No se ha podido enviar HTTP....."));
              
           }            
          }
      }
      else
      {
        Serial.println(F("No se ha podido conectarse con el servidor"));
       
      }


      delay(1000);
        
  

}


double getPressure()
{
  char status;
  double T;


  status = pressure.startTemperature();
  
  if (status != 0)
  {
    // Wait for the measurement to complete:

    delay(status);


    status = pressure.getTemperature(T);
    

    return T;
    
  }  else Serial.println(F("error al iniciar temperatura"));
}

