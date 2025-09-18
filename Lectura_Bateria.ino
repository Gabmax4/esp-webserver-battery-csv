#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <FastLED.h>
/////////////////////////////////////////////////////////////////////////////////
// Actualización OTA
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//////////////////////////////////////////////////////////////////////////////////
#define NUM_LEDS 2      //Se define el numero de leds RGB que se utilizaran
#define DATA_PIN 5      //Se define el pin por el cual se trasmitiran los datos
CRGB leds[NUM_LEDS];    //se define la cantidad de leds con esta funcion

int Modo_Fiesta=0;      //Variable encargada de activar el modo fiesta en la pulsera

//WiFi 
const char* ssid = "Totalplay-19A2";          //SSID donde se conectara la pulsera
const char* password = "19A294AErC4fqSkj";    //Contraseña 

//variables del metodo GET los cuales tienen los niveles de intensidad
const char* intensidad_ledverde = "intensidadverde";
const char* intensidad_ledrojo = "intensidadrojo";
const char* intensidad_ledamarillo = "intensidadamarillo";
const char* intensidad_ledblanco = "intensidadblanco";
const char* intensidad_ledfiesta = "intensidadfiesta";
const char* tiempo_hibernacion = "tiempo";
const char* delay_fiesta = "delayfiesta";   //Esta variable contiene el tiempo del delay del modo fiesta

String inputMessage;             //Con este string leemos la variable que tiene el GET request
String inputParam;               //Con este string se lee el parametro de que tiene la variable en el GET request

//Variables de tipo entero donde se almacenara el valor de intensidad con que prendera cada comando
int nivel_intensidad_verde;
int nivel_intensidad_rojo;
int nivel_intensidad_amarillo;
int nivel_intensidad_blanco;
int nivel_intensidad_fiesta;
int nivel_tiempo_hibernacion;
int nivel_delay_fiesta;       //Entero que define el delay del modo fiesta

int deep_sleep=0;

//Crea un WebServer Asycrono en el puerto 80
AsyncWebServer server(80);

//Funcion encargada de la leectura de la vateria
String lectura_bateria() {
  float t = analogRead(A0);         //Realiza la leectura en el pinA0
    Serial.println(t);              //Imprime en el monitor serial el valor leido
    return String(t);               //devuelve el valor leido como un tring cuando es llamada la funcion
  
}


void setup(){
  //Inicializacion del puerto serial
  Serial.begin(115200);                                     //Puesto a 11520 bautios
  WiFi.mode(WIFI_STA);
  //Inicializa el led que viene en la placa 
  pinMode(LED_BUILTIN, OUTPUT);

  //Se define que tipo de leds RGB se estan usando, el pin por el cual se trasmitira el dato y el numero de leds
  FastLED.addLeds<WS2813, DATA_PIN, RGB>(leds, NUM_LEDS);
  
  bool status;

  //Inicializacion de la memoria interna SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");    //Si no se puede inicializar mostrara este mensaje
    return;
  }

  //Conectando a el WiFi
  Serial.println("");
  Serial.println("");
  Serial.println("Estableciendo conexion WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("...");
    digitalWrite(LED_BUILTIN, LOW);              //El led se enciende para indicar que no se ha conectado al WiFi
  }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  NOMBRE OTORGADO AL CHIP ESP8866
  ArduinoOTA.setHostname("Pufita_Pulsera");


ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
 



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Imprime la IP local del ESP8266
  Serial.println("");
  Serial.print("La direccion IP es: ");
  Serial.println(WiFi.localIP());
  Serial.print("La direccion MAC es: ");
  Serial.println(WiFi.macAddress());
  //MDNS.begin("pulsera001");     //Inicializa el multiDNS y pone su direccion con pulsera001.local
  //MDNS.addService("http", "tcp", 80); //dice que el el metodo que utlizara "http" metodo de comunicacion "tcp" y el puerto "80"

  //Serial.println("La paguina web es: http://pulsera001.local");
  digitalWrite(LED_BUILTIN, HIGH);              //El led se apaga para indicar que la coneccion fue exitosa

  //Inicia la coneccion con el cliente (Navegador) y le manda el HTML que esta en la memoria SPIFFS
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });

  //Manda el nivel de bateria leido por el ESP8266
  server.on("/battery", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", lectura_bateria().c_str());
  });

//ACCIONES DE LOS BOTONES


//ENCENDER BLANCO
  server.on("/ledblanco", HTTP_GET, [](AsyncWebServerRequest *request){   //Lee la primera parte del GET y verifica que contenga /ledblanco al inicio
        Serial.println("SE LEYO /ledblanco");                             //Manda un mensaje al monitor serial cuando se detecto el GET request
        if (request->hasParam(intensidad_ledblanco)) {                    //Revisa si existe una variable llamada "intensidadblanco" ya que intensidad_ledblanco="intensidadblanco"
        inputParam = request->getParam(intensidad_ledblanco)->value();    //Obtiene el valor que contiene la variable 
        inputMessage = intensidad_ledblanco;                              //Dice el nombre de la variable
        nivel_intensidad_blanco = inputParam.toInt();                     //Convercion a entero y se guarda en la variable de nivel_intensidad_blanco

        //Muestra la variable leida y su valor en el monitor serial
        Serial.print("Variable: ");
        Serial.println(inputMessage);
        Serial.print("Valor de la variable = ");
        Serial.println(inputParam);
        }
        Modo_Fiesta=0;          //Es necesario esta linea para que no inicie el modo fiesta si ya se habia iniciado con anterioridad
        leds[0] = CRGB(nivel_intensidad_blanco,nivel_intensidad_blanco,nivel_intensidad_blanco);    //se le pasa el valor de la intencidad que va de 0 a 255 y se declara en (Verde,Rojo,Azul)
        leds[1] = CRGB(nivel_intensidad_blanco,nivel_intensidad_blanco,nivel_intensidad_blanco);
        FastLED.show();         //Enciende los leds
        request->send(200, "text/plain");   //Manda un mensaje 200 diciendo que todo salio bien
    });

//MODO FIESTA
  server.on("/ledfiesta", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("SE LEYO EL MODO FIESTA!!");
        if (request->hasParam(intensidad_ledfiesta)) {
        inputParam = request->getParam(intensidad_ledfiesta)->value();
        inputMessage = intensidad_ledfiesta;
        nivel_intensidad_fiesta = inputParam.toInt();
        Serial.print("Variable: ");
        Serial.println(inputMessage);
        Serial.print("Valor de la variable = ");
        Serial.println(inputParam); 
        }
        if (request->hasParam(delay_fiesta)) {
        inputParam = request->getParam(delay_fiesta)->value();
        inputMessage = delay_fiesta;
        nivel_delay_fiesta = inputParam.toInt();
        Serial.print("Variable: ");
        Serial.println(inputMessage);
        Serial.print("Valor de la variable = ");
        Serial.println(inputParam); 
        }        
        Modo_Fiesta=1;                               //Inicia el modo fiesta declarado en el loop
        request->send(200, "text/plain");
    });

//MODO HIBERNAR
  server.on("/hibernar", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("SE LEYO /hibernar");
        if (request->hasParam(tiempo_hibernacion)) {
        inputParam = request->getParam(tiempo_hibernacion)->value();
        inputMessage = tiempo_hibernacion;
        nivel_tiempo_hibernacion = inputParam.toInt();
        Serial.print("Variable: ");
        Serial.println(inputMessage);
        Serial.print("Valor de la variable = ");
        Serial.println(nivel_tiempo_hibernacion); 
        }        
        Modo_Fiesta=0;                               //Inicia el modo fiesta declarado en el loop
        request->send(200, "text/plain");
        unsigned long previousMillis = 0;
        const long interval = nivel_tiempo_hibernacion;
        deep_sleep=1;
        Serial.println("Me voy a reiniciar");
        ESP.restart();
    });


//APAGAR TODO
  server.on("/led=todooff", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("SE LEYO led=todooff");
        Modo_Fiesta=0;
        leds[0] = CRGB(0,0,0);
        leds[1] = CRGB(0,0,0);
        FastLED.show();
        request->send(200, "text/plain");
    });
    
//DORMIR
server.on("/dormir", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("SE LEYO DORMIR");
        Serial.println("La pulsera se fue a dormir");
        Modo_Fiesta=0;
        request->send(200, "text/plain");
        ESP.deepSleep(0);
    });

//ENCENDER AMARILLO
    server.on("/ledamarillo", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("SE LEYO ledamarillo");
        if (request->hasParam(intensidad_ledamarillo)) {
        inputParam = request->getParam(intensidad_ledamarillo)->value();
        inputMessage = intensidad_ledamarillo;
        nivel_intensidad_amarillo = inputParam.toInt();
        Serial.print("Variable: ");
        Serial.println(inputMessage);
        Serial.print("Valor de la variable = ");
        Serial.println(inputParam);
        }
        Modo_Fiesta=0;
        leds[0] = CRGB(nivel_intensidad_amarillo,nivel_intensidad_amarillo,0);
        leds[1] = CRGB(nivel_intensidad_amarillo,nivel_intensidad_amarillo,0);
        FastLED.show();
        request->send(200, "text/plain");
    });

//ENCENDER ROJO
    server.on("/ledrojo", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("SE LEYO /ledrojo");
        if (request->hasParam(intensidad_ledrojo)) {
        inputParam = request->getParam(intensidad_ledrojo)->value();
        inputMessage = intensidad_ledrojo;
        nivel_intensidad_rojo = inputParam.toInt();
        Serial.print("Variable: ");
        Serial.println(inputMessage);
        Serial.print("Valor de la variable = ");
        Serial.println(inputParam);
        }
        Modo_Fiesta=0;
        leds[0] = CRGB(0,nivel_intensidad_rojo,0);
        leds[1] = CRGB(0,nivel_intensidad_rojo,0);
        FastLED.show();
        request->send(200, "text/plain");
    });

//ENCENDER VERDE 
    server.on("/ledverde", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("SE LEYO /ledverde");
        if (request->hasParam(intensidad_ledverde)) {
        inputParam = request->getParam(intensidad_ledverde)->value();
        inputMessage = intensidad_ledverde;
        nivel_intensidad_verde = inputParam.toInt();        
        Serial.print("Variable: ");
        Serial.println(inputMessage);
        Serial.print("Valor de la variable = ");
        Serial.println(inputParam);
        }
        Modo_Fiesta=0;
        leds[0] = CRGB(nivel_intensidad_verde,0,0);
        leds[1] = CRGB(nivel_intensidad_verde,0,0);
        FastLED.show();
        request->send(200, "text/plain");
    });

  //Inicia el Servidor 
  server.begin();
}
 
void loop(){  

  //MDNS.update(); //para que funcione el DNS 
  
  //Valores para hacer colores randoms
  int valor_1, valor_2, valor_3,valor_4, valor_5, valor_6;

  //Verifica el que el Modo fiesta halla sido ejecutado
  if(Modo_Fiesta==1){
    valor_1=random(nivel_intensidad_fiesta);   //Pone el nivel de intensidad como valor maximo donde el valor ira de 0 hasta el valor que tenga nivel_intensidad_fiesta que como maximo es 255
    valor_2=random(nivel_intensidad_fiesta);
    valor_3=random(nivel_intensidad_fiesta);
    valor_4=random(nivel_intensidad_fiesta);
    valor_5=random(nivel_intensidad_fiesta);
    valor_6=random(nivel_intensidad_fiesta);
    leds[0] = CRGB(valor_1,valor_2,valor_3);
    leds[1] = CRGB(valor_4,valor_5,valor_6);
    FastLED.show();
    delay(nivel_delay_fiesta);                //hace el cambio de colores en milisegundos dependiendo del lo que tenga la variable nivel_delay_fiesta en ella 
    }
  if(deep_sleep==1){
     ESP.deepSleep(nivel_tiempo_hibernacion*1000);
     delay(100);
    }  
///////////////////////////////////////////////////////////////////////////7
 // lOOPITA EL OTA MADRE
 ArduinoOTA.handle();
///////////////////////////////////////////////////////////////////////////////


    
}
