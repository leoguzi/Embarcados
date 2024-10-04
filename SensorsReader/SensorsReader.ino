
#include "Arduino.h"
#include "LDR.h"
#include "LED.h"
#include "Thermistor.h"
#include "Button.h"
#include <Servo.h>

// Definição dos pinos
#define LDR_PIN A0
#define THERMISTOR_PIN A1
#define LEDR_PIN_LIGHT 5
#define SERVO_PIN 6
#define LEDR_PIN_FAN 7
#define PUSHBUTTON_PIN 8

// Global variables and defines
#define THRESHOLD_ldr 100
int ldrAverageLight;
// object initialization
LDR ldr(LDR_PIN);
LED ledLight(LEDR_PIN_LIGHT);
LED ledFan(LEDR_PIN_FAN);
Thermistor thermistor(THERMISTOR_PIN);
Servo myServo;
Button pushButton(PUSHBUTTON_PIN);



// define as variaveis de controle dos sensores/atuadores
bool isLightOn, currentIsLightOn, isFanOn, currentIsFanOn, areBlindsOpen, currentAreBlindsOpen = false;

void setup() {
  myServo.attach(SERVO_PIN);
  pushButton.init();
  Serial.begin(9600);
  while (!Serial)
    ;  // aguarda a conexão serial.

  Serial.println("O programa iniciou.");
}

void loop() {
  handleLight();   // metodo para acender e apagar a luz conforme a luminosidade.
  handleFan();     // metodo para ligar o ventilador conforme a temperatura.
  handleBlinds();  // metodo para baixar ou levantar as percianas.
}

void handleLight() {
  int ldrSample = ldr.read();  //mede a luz do ambiente.

  if (ldrSample < 500) {  //verifica se a luminosidade diminuiu
    currentIsLightOn = true;
  } else {
    currentIsLightOn = false;
  }
  if (currentIsLightOn != isLightOn) {
    isLightOn = currentIsLightOn;
    if (isLightOn) {
      ledLight.on();  //liga o led
      Serial.println("Luz ligada!");
    } else {
      ledLight.off();  // desliga o led
      Serial.println("Luz apagada!");
    }
  }
}

void handleFan() {
  float thermistorTempC = thermistor.getTempC();  // por algum motivo a temperatura está lendo ao contrario, mas não impede o funcionamento, visto que só precisamos da variação.

  if (thermistorTempC < 24) {  // verifica a temperatura para ligar o ventilador
    currentIsFanOn = true;
  } else {
    currentIsFanOn = false;
  }

  if (currentIsFanOn != isFanOn) {
    isFanOn = currentIsFanOn;
    if (isFanOn) {
      ledFan.on();  //liga o led
      Serial.println("Ventilador ligado!");
    } else {
      ledFan.off();  //desliga o led
      Serial.println("Ventilador desligado!");
    }
  }
}

void handleBlinds() {
  bool pushButtonVal = pushButton.onChange();
  if(pushButtonVal != 0) {  //verifica se o botão foi pressionado.
    currentAreBlindsOpen = !currentAreBlindsOpen;
  }
  
  if (currentAreBlindsOpen != areBlindsOpen) {
    areBlindsOpen = currentAreBlindsOpen;
    if (areBlindsOpen) {
      myServo.write(180);  //abre as persianas (movimenta servo para a posição 180)
      Serial.println("Persianas abertas!");
    } else {
      myServo.write(0); //fecha as persianas (movimenta servo para a posição 0)
      Serial.println("Persianas fechadas!");
    }
  }
}