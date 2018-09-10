#include "MonitorMCH.h"


struct NodeMonitor mchA, mchB;
double mediaVibracao;
int maximoVibracao,minimoVibracao;
WiFiServer server(80);
void inicializarVariaveis() {

  mchA.pinInput = D5;
  mchA.ledFalha = D1;
  mchA.indicacaoAnterior = -1;
  mchA.msg = "INDICACAO MCH A";

  mchB.pinInput = D6;
  mchB.ledFalha = D2;
  mchB.indicacaoAnterior = -1;
  mchB.msg = "INDICACAO MCH B";
  
}


void setup() {
  inicializarVariaveis();
  USE_SERIAL.begin(115200);
  //USE_SERIAL.setDebugOutput(true);


  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  
  pinMode(mchA.pinInput, INPUT);
  pinMode(mchA.ledFalha, OUTPUT);
  pinMode(mchB.pinInput, INPUT);
  pinMode(mchB.ledFalha, OUTPUT);


  pinMode(sensorMovimentacao, INPUT);
  pinMode(buzzer, OUTPUT);

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }
  resetFalha();
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("DESKTOP-H4", "1234567890");
  WiFiMulti.addAP("valerio", "valerioarduinobreno");
  Serial.println(WiFi.localIP());
}
void alerta() {
  tone(buzzer, 650, 1000);
  delay(100);
  noTone(buzzer);
}

int  getMovimentacao() {
  movimentacao =  digitalRead(sensorMovimentacao);
  return movimentacao;
}

double getVibracao(double *media,int *maximo,int *minimo) {
  int i = 1;
  int sensorValue = analogRead(sensorVibracao);
  *maximo =  sensorValue;
  *minimo =  sensorValue;
  *media  =  sensorValue;

  while ((sensorValue > 10) && (i< qtdLeiturasVibracao)) {
    sensorValue = analogRead(sensorVibracao);
    *media = *media +  sensorValue;
    *maximo =(*maximo < sensorValue) ? sensorValue : *maximo;
    *minimo =(*minimo > sensorValue) ? sensorValue : *minimo;
    i++;
    delay(10);
  }

  *media = *media / (i * 1.00);
}

void  perdaIndicacao(NodeMonitor *mch) {

  mch->indicacaoAtual =  ((!getMovimentacao()) && (!digitalRead(mch->pinInput)))? 1 : 0;
  postEnviar(*mch);
}

void postEnviar(NodeMonitor mch){
  
    if ((mch.indicacaoAtual != mch.indicacaoAnterior)) {
      if (mch.indicacaoAtual)
        digitalWrite(mch.ledFalha, HIGH);
      gerarJson(mch.msg);
    }
}


void gerarJson(String mensagem) {

  String json =  "{\"local\" : \"" + local + "\" , ";
         json += " \"mchA\": " + String(digitalRead(mchA.pinInput),DEC) + " , ";
         json += " \"mchB\": " + String(digitalRead(mchB.pinInput),DEC)  + " , ";
         json += "\"vibmedia\": " + String(mediaVibracao,2) + " , ";
         json += "\"vibmax\": " + String(maximoVibracao,DEC) + " , ";
         json += "\"vibmin\": " + String(minimoVibracao,DEC) + " , ";
         json += "\"mov\": " + String(movimentacao,DEC) + " , ";
         json += "\"desc\": \"" + mensagem + "\" }";
  postDados(json);
}

void postSucesso(String json){
        resetFalha();
        mchA.indicacaoAnterior =  mchA.indicacaoAtual;
        mchB.indicacaoAnterior =  mchB.indicacaoAtual;
        anteriorJson = json;
  }
void checkIndicacao() {
    perdaIndicacao(&mchA);
    perdaIndicacao(&mchB);
}

void resetFalha() {
  digitalWrite(mchA.ledFalha, LOW);
  digitalWrite(mchB.ledFalha, LOW);
}


void checkVibracao() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisVibracao >= intervalVibracao) {
    previousMillisVibracao = currentMillis;
    
    getVibracao(&mediaVibracao,&maximoVibracao,&minimoVibracao);
    Serial.print("Check Vibracao: ");
    Serial.print(mediaVibracao);
    Serial.print(" - ");
    Serial.print(maximoVibracao);
    Serial.print(" - ");
    Serial.print(minimoVibracao);
    Serial.println(" - ");
    if (mediaVibracao > 800)
      gerarJson("Nivel Alto de vibracao");
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
      if (httpCode == 200) {
        postSucesso(json);
        alerta();
      }
      http.end();  //Close connection

    } else {
      USE_SERIAL.println("Error in WiFi connection");
    }
    alerta();
  }
}




void loop() {
  checkIndicacao();
  checkVibracao();
  delay(1);
}
