#ifndef MonitorMCH
#define MonitorMCH
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#define tempoMovimentacao 10
#define qtdLeiturasVibracao 30
#define USE_SERIAL Serial
ESP8266WiFiMulti WiFiMulti;


String url = "http://192.168.0.29:8080";
String local = "TU W01";
String anteriorJson = ""; 

const int sensorVibracao = A0;



int sensorMovimentacao = D7;
int movimentacao;

const int buzzer = D8;


unsigned long previousMillisVibracao = 0;
const long intervalVibracao = 1000;

struct NodeMonitor
    {
        int pinInput;
        int indicacaoAnterior;
        int indicacaoAtual;
        int ledFalha;
        String msg;
    };
    
        void inicializarVariaveis();
        void alerta();
        int  getMovimentacao();
        double getVibracao();
        void  perdaIndicacao(NodeMonitor mch);
        void postEnviar(NodeMonitor mch);
        void gerarJson(String mensagem);
        void postSucesso();
        void checkIndicacao();
        void resetFalha();
        void checkVibracao();
        void postDados(String json);
        void test();

 
#endif
