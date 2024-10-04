#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] Aguarde %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("NOME_DA_REDE", "SENHA_DA_REDE"); // nome e senha da rede a se conectar.

}

void loop() {
  // aguarda conexão com wifi
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] Iniciar...\n");
    if (http.begin(client, "http://jigsaw.w3.org/HTTP/connection.html")) {  // inicia o cliente http


      Serial.print("[HTTP] GET...\n");
      // chama a api REST utilizando o metodo get.
      int httpCode = http.GET();

      // o status code vai ser negativo em caso de erro.
      if (httpCode > 0) {
        Serial.printf("[HTTP] GET... status code: %d\n", httpCode);

        // escreve o conteudo retornado, em caso de sucesso.
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else {
        // escreve mensagem como erro em caso de erro.
        Serial.printf("[HTTP] GET... falhou, erro: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }

  delay(10000);
}
