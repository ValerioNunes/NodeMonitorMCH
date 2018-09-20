#include "MonitorMCH.h"


struct NodeMonitor mchA, mchB;
struct NodeMonitorVibracao vib;
 
double mediaVibracao;
int maximoVibracao,minimoVibracao;

void inicializarVariaveis() {

  mchA.pinInput = D5;
  mchA.ledFalha = D1;
  mchA.indicacaoAnterior = -1;
  mchA.msg = "INDICACAO MCH A";

  mchB.pinInput = D6;
  mchB.ledFalha = D2;
  mchB.indicacaoAnterior = -1;
  mchB.msg = "INDICACAO MCH B";

  vib.pinInput =  A0;
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
  WiFiMulti.addAP("utechrepXX", "raspberry");
  Serial.println(WiFi.localIP());
}
void onAlerta() {
  digitalWrite( buzzer, HIGH);
}
void offAlerta() {
  digitalWrite( buzzer, LOW);
}

int  getMovimentacao() {
  movimentacao =  digitalRead(sensorMovimentacao);
  return movimentacao;
}

void getVibracao(NodeMonitorVibracao *vib) {
  int i = 1;
  int sensorValue = analogRead(vib->pinInput);
  vib->maximo =  sensorValue;
  vib->minimo =  sensorValue;
  vib->media  =  sensorValue;

  while ((sensorValue > 10) && (i< qtdLeiturasVibracao)) {
    sensorValue = analogRead(vib->pinInput);
    vib->media = vib->media +  sensorValue;
    vib->maximo =(vib->maximo < sensorValue) ? sensorValue : vib->maximo;
    vib->minimo =(vib->minimo > sensorValue) ? sensorValue : vib->minimo;
    i++;
    delay(10);
  }

   vib->media =  vib->media / (i * 1.00);
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
         json += "\"vibmedia\": " + String(vib.media,2) + " , ";
         json += "\"vibmax\": " + String(vib.maximo,DEC) + " , ";
         json += "\"vibmin\": " + String(vib.minimo,DEC) + " , ";
         json += "\"mov\": " + String(movimentacao,DEC) + " , ";
         json += "\"desc\": \"" + mensagem + "\" }";
  Serial.println(json);
  postDados(json);
  
   
}

void postSucesso(String json){
        resetFalha();
        mchA.indicacaoAnterior =  mchA.indicacaoAtual;
        mchB.indicacaoAnterior =  mchB.indicacaoAtual;
        anteriorJson = json;
        offAlerta();
  }
void checkIndicacao() {
    perdaIndicacao(&mchA);
    perdaIndicacao(&mchB);
}

void resetFalha() {
  offAlerta();
}


void checkVibracao(){

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisVibracao >= intervalVibracao) {
    previousMillisVibracao = currentMillis;
    
    getVibracao(&vib);
    Serial.print("Check Vibracao: ");
    Serial.print(vib.media);
    Serial.print(" - ");
    Serial.print(vib.maximo);
    Serial.print(" - ");
    Serial.println(vib.minimo);
    
    if (vib.media > 800)
      gerarJson("Nivel Alto de vibracao");
  }
}

void getDados() {
        const char* server = "api.thingspeak.com";
        WiFiClient client;

                     if (client.connect(server,80))   //   "184.106.153.149" or api.thingspeak.com
                      {  
                            
                             String postStr = apiKey;
                             postStr +="&field1=";
                             postStr += String(vib.media);
                             postStr +="&field2=";
                             postStr +=  String(vib.maximo);
                             postStr +="&field3=";
                             postStr +=  String(vib.minimo);
                             postStr +="&field4=";
                             postStr +=  String(mchA.indicacaoAtual);
                             postStr +="&field5=";
                             postStr +=  String(mchB.indicacaoAtual);                             
                             postStr += "\r\n\r\n";
 
                             client.print("POST /update HTTP/1.1\n");
                             client.print("Host: api.thingspeak.com\n");
                             client.print("Connection: close\n");
                             client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
                             client.print("Content-Type: application/x-www-form-urlencoded\n");
                             client.print("Content-Length: ");
                             client.print(postStr.length());
                             client.print("\n\n");
                             client.print(postStr);
                             postSucesso(" ");
                        }else{
                         Serial.println("Sem conexao com Servidor");
                        }
          client.stop();
          Serial.println("Waiting...");

}



void postDados(String json) {
  
  if (!anteriorJson.equals(json)) {
    onAlerta();

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
      }
      http.end();  //Close connection

    } else {
      USE_SERIAL.println("Error in WiFi connection");
    }
  }
}

void loop() {
  checkIndicacao();
  checkVibracao();
  delay(1);
}
