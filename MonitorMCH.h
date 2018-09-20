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


const String url = "http://192.168.137.238:8080";
const String local = "TU W01";
String anteriorJson = ""; 
const String apiKey =  "UNLNSI0NA4ZIJ8BA";
const int sensorVibracao = A0;

const int sensorMovimentacao = D7;
int movimentacao;

const int buzzer = D8;


unsigned long previousMillisVibracao = 0;
const long intervalVibracao = 5000;

struct NodeMonitor
    {
        int pinInput;
        int indicacaoAnterior;
        int indicacaoAtual;
        int ledFalha;
        String msg;
    };
    
struct NodeMonitorVibracao
    {
        int pinInput;
        double media;
        int maximo;
        int minimo;
        String msg;
    };
    
    void inicializarVariaveis();
    void onAlerta();
    void offAlerta();
    void resetFalha();
        
        
    void checkVibracao();
    double getVibracao(double *media,int *maximo,int *minimo);
    
    void checkIndicacao();
    void perdaIndicacao(NodeMonitor mch);
    int  getMovimentacao(); 
    
    void postEnviar(NodeMonitor mch);
    void gerarJson(String mensagem); 
    void postDados(String json);
    void postSucesso();
       
#endif
