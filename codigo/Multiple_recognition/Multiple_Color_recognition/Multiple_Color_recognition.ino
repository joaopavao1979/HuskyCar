#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

HUSKYLENS huskylens;
SoftwareSerial mySerial(8, 9);  // RX, TX

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);

  while (!huskylens.begin(mySerial)) {
    Serial.println(F("Falha ao iniciar o HuskyLens."));
    delay(1000);
  }

  Serial.println("HuskyLens pronto em modo UART.");
  huskylens.writeAlgorithm(ALGORITHM_COLOR_RECOGNITION);
  Serial.println("Modo: Reconhecimento de cores com coordenadas centradas.");
}

void loop() {
  if (huskylens.request()) {
    while (huskylens.available()) {
      HUSKYLENSResult result = huskylens.read();

      // Conversão para coordenadas centradas em (0,0)
      int xCart = result.xCenter - 160;
      int yCart = result.yCenter - 120;

      Serial.print("Cor ID=");
      Serial.print(result.ID);
      Serial.print(" | x=");
      Serial.print(xCart);
      Serial.print(" , y=");
      Serial.print(yCart);
      Serial.print(" | largura=");
      Serial.print(result.width);
      Serial.print(" , altura=");
      Serial.println(result.height);
    }
  } else {
    Serial.println("Sem deteções...");
  }

  delay(500);
}