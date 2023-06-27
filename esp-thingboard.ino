#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define WIFI_AP "NORMA_LARA"
#define WIFI_PASSWORD "97207453301"

#define TOKEN "fOhg5j5hNpRrAYHyHIuk"
char thingsboardServer[] = "iot.ceisufro.cl";
WiFiClient wifiClient;
PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend;

int estadoActual1=0;
int estadoActual2=0;
int estadoUltimo=0;
int contador=0;
float radioEnCm=30.0;   //INGRESAR radio de la rueda en cm
float pi=3.1416;
float perimetroRueda=2*pi*(radioEnCm/100);  //Calcula Perimetro en metros
float distRecorrida;
float distKM=0;
int tiempo1=0;
int tiempo2=0;
int tiempo3=0;
float tiempo4=0;
float velocidad;

void setup(){
  pinMode(14,INPUT);
  Serial.begin(115200);
  InitWiFi();
  client.setServer( thingsboardServer, 1883 );
  lastSend = 0;
}        

void loop(){
  if ( !client.connected() ) {
    reconnect();
  }
  if ( millis() - lastSend > 1000 ) { // Update and send only after 1 seconds
     getDatos();
    lastSend = millis();
  }
  client.loop();     
}

void getDatos(){
       estadoActual1=digitalRead(14);
        delay(10);
        estadoActual2=digitalRead(14);
       if (estadoActual1 == estadoActual2) {
              if (estadoActual1 != estadoUltimo){
                    if (estadoActual1 == HIGH) {
                        contador = contador + 1;
                        Serial.print ("Vueltas ");
                        Serial.println(contador);
                        distancia();
                        VEL();
                    }
              }
        }
        estadoUltimo= estadoActual1;
         if (contador%2 == 0 ) {
              digitalWrite(4, LOW);
        }
        else {
              digitalWrite(4, HIGH);
        }

  Serial.print("distancia: ");
  Serial.print(distRecorrida);
  Serial.print("velocidad: ");
  Serial.print(velocidad);

  // Just debug messages
  Serial.print( "envio distancia y velocidad : [" );
  Serial.print( distRecorrida ); Serial.print( "," );
  Serial.print( velocidad );
  Serial.print( "]   -> " );

  // Prepare a JSON payload string
  String payload = "{";
  payload += "\"distRecorrida\":"; payload += distRecorrida; payload += ",";
  payload += "\"velocidad\":"; payload += velocidad;
  payload += "}";

  // Send payload
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  client.publish( "v1/devices/me/telemetry", attributes );
  Serial.println( attributes );

}

void distancia(){
                        distRecorrida=perimetroRueda*contador;
                        distKM=distRecorrida/1000;
                        if(distRecorrida<=999){
                                Serial.print("Distancia recorrida en m= ");
                                Serial.println(distRecorrida);

                        }
                        else{
                                Serial.print("Distancia recorrida en Km= ");
                                Serial.println(distKM);

                        }
                        
}

void VEL(){
        if (contador%2 == 0 ) {
              tiempo1=millis();
        }
        else {
              tiempo2=millis();
        }
        tiempo3=abs(tiempo2-tiempo1); //hay que pasar el tiempo a hrs
        tiempo4=(((tiempo3/1000.0)/60)/60);
        velocidad=((perimetroRueda/1000)/tiempo4);
        Serial.print("velocidad= ");
        Serial.println(velocidad);        
}


void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}


void reconnect() {
  while (!client.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    if ( client.connect("Velocimetro", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      delay( 5000 );
    }
  }
}