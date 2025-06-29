// Projeto: HuskyCar Challenge - Seguidor de Cor com HuskyLens (UART via SoftwareSerial)
// Adaptado para Arduino UNO R4 WiFi usando pinos 8 (RX) e 9 (TX)
// Autor: João Pavão + Tiago Martins + ChatGPT
// Data: 2025-06-29
// Modo da HuskyLens: Color Recognition
// Driver de motores: L298P

#include <SoftwareSerial.h>
#include "HUSKYLENS.h"

// Comunicação com HuskyLens
SoftwareSerial huskySerial(8, 9);  // RX, TX
HUSKYLENS huskylens;

// Pinos do driver L298P
const int ENA = 5;
const int IN1 = 4;
const int IN2 = 3;
const int ENB = 6;
const int IN3 = 7;
const int IN4 = 8;

// Parâmetros de imagem
const int IMAGE_WIDTH = 320;
const int CENTER_X = IMAGE_WIDTH / 2;

// Parâmetros de controlo
const float KP_DIR = 0.08;
const float KP_DIST = 0.05;
const int MAX_SPEED = 120;
const int MIN_SPEED = 40;
const int TARGET_OBJ_WIDTH = 60;
const int TOLERANCE_WIDTH = 10;

// Média móvel
const int FILTER_WINDOW = 5;
int bufferX[FILTER_WINDOW];
int bufferW[FILTER_WINDOW];
int bufferIndex = 0;
bool bufferCheio = false;

// Estado da HuskyLens
bool huskyOk = false;

// Struct para leitura
struct HuskyData {
  bool learned;
  bool available;
  int rawX;
  int rawW;
};

// ---------- Inicializações ----------

void initSerialAndHusky() {
  Serial.begin(9600);
  huskySerial.begin(9600);
  
  Serial.println(F("[INFO] Inicializando HuskyLens via SoftwareSerial (pinos 8 e 9)..."));
  unsigned long inicio = millis();
  
  while (millis() - inicio < 3000) {
    if (huskylens.begin(huskySerial)) {
      huskyOk = true;
      break;
    }
    delay(200);
  }

  if (!huskyOk) {
    Serial.println(F("[ERRO] Não foi possível conectar à HuskyLens."));
    return;
  }

  huskylens.writeAlgorithm(ALGORITHM_COLOR_RECOGNITION);
  Serial.println(F("[INFO] HuskyLens pronta em modo Color Recognition."));
}

void initMotors() {
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  stopMotors();
}

void initFilterBuffers() {
  for (int i = 0; i < FILTER_WINDOW; i++) {
    bufferX[i] = CENTER_X;
    bufferW[i] = TARGET_OBJ_WIDTH;
  }
  bufferIndex = 0;
  bufferCheio = false;
}

// ---------- Filtragem ----------

int filterX(int x, int w) {
  bufferX[bufferIndex] = x;
  bufferW[bufferIndex] = w;
  bufferIndex++;
  if (bufferIndex >= FILTER_WINDOW) {
    bufferIndex = 0;
    bufferCheio = true;
  }

  int somaX = 0;
  int count = bufferCheio ? FILTER_WINDOW : bufferIndex;
  for (int i = 0; i < count; i++) somaX += bufferX[i];
  return somaX / count;
}

int filterW() {
  int somaW = 0;
  int count = bufferCheio ? FILTER_WINDOW : bufferIndex;
  for (int i = 0; i < count; i++) somaW += bufferW[i];
  return somaW / count;
}

// ---------- Leitura da HuskyLens ----------

HuskyData readHuskyLens() {
  HuskyData data;
  if (!huskylens.request()) {
    data.learned = false;
    data.available = false;
    return data;
  }
  data.learned = huskylens.isLearned();
  data.available = huskylens.available();
  if (data.learned && data.available) {
    HUSKYLENSResult result = huskylens.read();
    data.rawX = result.xCenter;
    data.rawW = result.width;
  } else {
    data.rawX = CENTER_X;
    data.rawW = 0;
  }
  return data;
}

// ---------- Controlo dos motores ----------

void computeMotorSpeeds(int filteredX, int filteredW, int &speedL, int &speedR) {
  int errorX = filteredX - CENTER_X;
  int errorW = TARGET_OBJ_WIDTH - filteredW;
  float corrDir = KP_DIR * errorX;
  float baseSpeed = abs(errorW) <= TOLERANCE_WIDTH ? 0.0 : KP_DIST * errorW;

  float rawLeft = baseSpeed - corrDir;
  float rawRight = baseSpeed + corrDir;

  if (abs(rawLeft) < MIN_SPEED) rawLeft = 0;
  if (abs(rawRight) < MIN_SPEED) rawRight = 0;

  speedL = constrain((int)rawLeft, -MAX_SPEED, MAX_SPEED);
  speedR = constrain((int)rawRight, -MAX_SPEED, MAX_SPEED);
}

void setMotorSpeeds(int speedL, int speedR) {
  if (speedL > 0) {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); analogWrite(ENA, speedL);
  } else if (speedL < 0) {
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); analogWrite(ENA, -speedL);
  } else {
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); analogWrite(ENA, 0);
  }

  if (speedR > 0) {
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); analogWrite(ENB, speedR);
  } else if (speedR < 0) {
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); analogWrite(ENB, -speedR);
  } else {
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); analogWrite(ENB, 0);
  }
}

void stopMotors() {
  analogWrite(ENA, 0); analogWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

// ---------- Ciclo principal ----------

void setup() {
  initSerialAndHusky();
  initMotors();
  initFilterBuffers();
  Serial.println(F("[INFO] Setup completo."));
}

void loop() {
  if (!huskyOk) {
    stopMotors();
    delay(100);
    return;
  }

  HuskyData hd = readHuskyLens();
  if (!hd.learned || !hd.available) {
    Serial.println(F("[INFO] Aguardando cor aprendida visível..."));
    stopMotors();
    delay(100);
    return;
  }

  int filtX = filterX(hd.rawX, hd.rawW);
  int filtW = filterW();
  int velL = 0, velR = 0;
  computeMotorSpeeds(filtX, filtW, velL, velR);
  setMotorSpeeds(velL, velR);

  Serial.print(F("[DEBUG] x=")); Serial.print(hd.rawX);
  Serial.print(F(" filtX=")); Serial.print(filtX);
  Serial.print(F(" w=")); Serial.print(hd.rawW);
  Serial.print(F(" filtW=")); Serial.print(filtW);
  Serial.print(F(" velL=")); Serial.print(velL);
  Serial.print(F(" velR=")); Serial.println(velR);

  delay(20);
}
