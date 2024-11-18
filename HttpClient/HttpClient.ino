#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "LDR.h"
#include "LED.h"
#include "Button.h"
#include <Servo.h>
#include <string>

// Definição dos pinos
#define LDR_PIN A0
#define LEDR_PIN_LIGHT D4
#define SERVO_PIN D3
#define PUSHBUTTON_PIN D2

// Global variables and defines
#define THRESHOLD_ldr 100
unsigned long UpdateNoIPCycle = 4000;
unsigned long UpdateNoIPLastMillis = 0;
const String BASE_API = "http://ec2-18-227-49-45.us-east-2.compute.amazonaws.com:8080/";


// object initialization
LDR ldr(LDR_PIN);
LED ledLight(LEDR_PIN_LIGHT);
Servo myServo;
Button pushButton(PUSHBUTTON_PIN);
ESP8266WiFiMulti WiFiMulti;

// define as variaveis de controle dos sensores/atuadores
bool isLightOn, lightManualMode, areBlindsOpen = false;

void setup() {
  myServo.attach(SERVO_PIN);
  pushButton.init();
  Serial.begin(115200);
  while (!Serial)
    ;  // aguarda a conexão serial.

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] Aguarde %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("AP 1003", "naoconecte");  // nome e senha da rede a se conectar.
}

void loop() {
  // aguarda conexão com wifi
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    handleLight();   // metodo para acender e apagar a luz conforme a luminosidade.
    handleBlinds();  // metodo para baixar ou levantar as percianas quando o botão é pressionado.
    if (cycleCheck(&UpdateNoIPLastMillis, UpdateNoIPCycle)) {
      updatePersiana();  // atualização recorrente da persiana
      updateLampada();   // atualização recorrente da lampada
    }
  }
}

void handleLight() {
  int ldrSample = ldr.read();
  if (!lightManualMode) {                 //mede a luz do ambiente.
    if (!isLightOn && ldrSample < 700) {  //verifica se a luminosidade diminuiu
      put("lampada/acender");
      //Serial.println("Lampada ligada auto!");
      updateLampada();
    } else if (isLightOn && ldrSample > 700) {
      put("lampada/apagar");
      //Serial.println("Lampada desligada auto!");
      //delay(1000);
      updateLampada();
    }
  }
}

// busca status da persiana
void handleBlinds() {
  bool pushButtonVal = pushButton.onChange();
  if (pushButtonVal != 0) {  //verifica se o botão foi pressionado.
    Serial.println("Botão pressionado!");
    if (!areBlindsOpen) {
      put("persiana/abrir");
    } else {
      put("persiana/fechar");
    }
    updatePersiana();
  }
}

// busca status da lampada
void updateLampada() {
  lightManualMode = get("lampada/modo");
  isLightOn = get("lampada/status");
  if (isLightOn) {
    ledLight.on();
    //Serial.println("Lampada ligada!");
  } else {
    ledLight.off();
    //Serial.println("Lampada desligada!");
  }
}

// busca status da persiana
void updatePersiana() {
  bool areBlindsOpen = get("persiana/status");
  if (areBlindsOpen) {
    myServo.write(180);
    //Serial.println("Persianas abertas!");
  } else {
    myServo.write(0);
    //Serial.println("Persianas fechadas!");
  }
}

boolean cycleCheck(unsigned long *UpdateNoIPLastMillis, unsigned long UpdateNoIPCycle) {
  unsigned long currentMillis = millis();
  if (currentMillis - *UpdateNoIPLastMillis >= UpdateNoIPCycle) {
    *UpdateNoIPLastMillis = currentMillis;
    return true;
  } else {
    return false;
  }
}

boolean get(String path) {
  WiFiClient client;
  HTTPClient http;
  if (http.begin(client, BASE_API + path)) {  // inicia o cliente http
    // chama a api REST utilizando o metodo get.
    int httpCode = http.GET();
    // o status code vai ser negativo em caso de erro.
    if (httpCode > 0) {
      //Serial.printf("[HTTP] GET... status code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        // Converte o payload para booleano
        if (payload == "true") {
          http.end();
          return true;
        } else if (payload == "false") {
          http.end();
          return false;
        }
      }
    } else {
      // escreve mensagem como erro em caso de erro.
      Serial.printf("[HTTP] GET... falhou, erro: %s\n", http.errorToString(httpCode).c_str());
      http.end();
      return false;
    }
  }
  Serial.printf("[HTTP} Unable to connect\n");
  http.end();
  return false;
}

void put(String path) {
  WiFiClient client;
  HTTPClient http;
  if (http.begin(client, BASE_API + path)) {  // inicia o cliente http
    // chama a api REST utilizando o metodo put.
    int httpCode = http.PUT("");

    // o status code vai ser negativo em caso de erro.
    if (httpCode > 0) {
      // escreve o conteudo retornado, em caso de sucesso.
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        //String payload = http.getString();
      }
    } else {
      // escreve mensagem como erro em caso de erro.
      Serial.printf("[HTTP] PUT... falhou, erro: %s\n", http.errorToString(httpCode).c_str());
      http.end();
    }
  } else {
    Serial.printf("[HTTP} Unable to connect\n");
    http.end();
  }
}
