#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>


#define USE_SERIAL Serial
#define Normal   1
#define Reverso -1
#define semIndicacao 0
#define qtdAtributos 4
#define tempoMovimentacao 10
#define mediaVibracao 30
ESP8266WiFiMulti WiFiMulti;

String url = "http://192.168.137.238:8080";
String local = "TU W01";
String anteriorJson = "";

// 0 - input1 / 1 - input2 / 2 - perda indicacao / 3 - LedFalha



int inputMCH_A = D5;
int inputMCH_B = D6;
int falhaMCH_A = D1;
int falhaMCH_B = D2;
int valorAntMCH_A = -1;
int valorAntMCH_B = -1;


int sensorMovimentacao = D7;
const int buzzer = D8;
bool Alerta =  false;

const int sensorVibracao = A0;
double vibracao;
int movimentacao;
unsigned long previousMillisIndicacao = 0;
const long intervalIndicacao = 1000;

unsigned long previousMillisVibracao = 0;
const long intervalVibracao = 1000;



void setup() {

  USE_SERIAL.begin(115200);
  //USE_SERIAL.setDebugOutput(true);


  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();


  pinMode(inputMCH_A, INPUT);
  pinMode(inputMCH_B, INPUT);
  
  pinMode(falhaMCH_A , OUTPUT);
  pinMode(falhaMCH_B , OUTPUT);

  
  //pinMode(sensorMovimentacao, INPUT);
  //pinMode(buzzer, INPUT);
  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }
  resetFalha();
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("DESKTOP-H4", "1234567890");

}
void alerta(){
  
     if(Alerta){ 
          tone(buzzer, 650, 1000);
          delay(100);
          noTone(buzzer);
     }
 
}

int  getMovimentacao() {
  movimentacao =  digitalRead(sensorMovimentacao);
  return movimentacao;
}

double getVibracao() {
  double media = 0;
  for (int i = 0; i < mediaVibracao; i++ ) {
    int sensorValue = analogRead(sensorVibracao);
    media = media +  sensorValue;
  }

  return media / (mediaVibracao * 1.00);
}




bool  perdaIndicacao(int mch) {

  if (!getMovimentacao()) {
    delay(tempoMovimentacao);
    if (!digitalRead(mch))
      return true;
  }
  return false;
}

void geralJson(String mensagem) {

  String json =  "{\"local\" : \"" + local + "\", \"mchA\": " + digitalRead(inputMCH_A) + ", \"mchB\": " + digitalRead(inputMCH_B)  + " , \"vib\": " + vibracao + " , \"mov\": " + movimentacao + ", \"desc\": \"" + mensagem + "\" }";
  postDados(json);
}


void checkIndicacao() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisIndicacao >= intervalIndicacao) {
    previousMillisIndicacao = currentMillis;
    bool valor = perdaIndicacao(inputMCH_A);
    
    if ((valor != valorAntMCH_A)) {
      geralJson("INDICACAO MCH A");
      Alerta = true;
      if(valor)
        digitalWrite(falhaMCH_A, HIGH); 
    }
    valor = perdaIndicacao(inputMCH_B);
    
    if ((valor != valorAntMCH_B)) {
      geralJson("INDICACAO MCH B");
      if(valor)
        digitalWrite(falhaMCH_B, HIGH); 
      Alerta = true;
    }
  }

}

void resetFalha(){
   digitalWrite(falhaMCH_A, LOW); 
   digitalWrite(falhaMCH_B, LOW); 
   Alerta = false;
  }
void checkVibracao() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisVibracao >= intervalVibracao) {
    previousMillisVibracao = currentMillis;
    vibracao = getVibracao();
    Serial.print("Check Vibracao: ");
    Serial.println(vibracao);
    if (getVibracao() > 800)
      geralJson("Nivel Alto de vibracao");
  }
}



void postDados(String json) {

  if (!anteriorJson.equals(json)) {
    if ((WiFiMulti.run() == WL_CONNECTED)) { //Check WiFi connection status

      HTTPClient http;    //Declare object of class HTTPClient

      http.begin(url);      //Specify request destination
      http.addHeader("Content-Type", "application/json");//Specify content-type header

      //int httpCode = http.POST("{\"tes\" : \"okok\"}");   //Send the request
      int httpCode = http.POST(json);   //Send the request
      String payload = http.getString();                  //Get the response payload

      Serial.println(httpCode);   //Print HTTP return code
      Serial.println(payload);    //Print request response payload
      if (httpCode == 200){
        resetFalha();
        valorAntMCH_A =  perdaIndicacao(inputMCH_A);
        valorAntMCH_B =  perdaIndicacao(inputMCH_B);
        anteriorJson = json;
      }
      http.end();  //Close connection

    } else {
      USE_SERIAL.println("Error in WiFi connection");
    }
  }
}

void test() {
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    HTTPClient http;
    USE_SERIAL.print("[HTTP] begin...\n");
    http.begin(url); //HTTP
    USE_SERIAL.print("[HTTP] GET...\n");
    int httpCode = http.GET();

    if (httpCode > 0) {
      USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        USE_SERIAL.println(payload);
      }
    } else {
      USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    postDados("{\"tes\" : \"okok\"}");
  }
}

void loop() {
  checkIndicacao();
  checkVibracao();
  alerta();
  delay(10);
}
