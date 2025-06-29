#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

HUSKYLENS huskylens;
SoftwareSerial mySerial(8, 9);  // RX (pino 8), TX (pino 9)

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);

  while (!huskylens.begin(mySerial)) {
    Serial.println(F("Falha ao iniciar o HuskyLens."));
    delay(1000);
  }

  Serial.println(F("HuskyLens pronto em modo UART."));
  huskylens.writeAlgorithm(ALGORITHM_TAG_RECOGNITION);  // <- TAG mode
  Serial.println(F("Modo: Tag Recognition com leitura múltipla."));
}

void loop() {
  if (huskylens.request()) {
    if (!huskylens.available()) {
      Serial.println("Nenhum marcador visível...");
    }

    while (huskylens.available()) {
      HUSKYLENSResult result = huskylens.read();

      // Ignora tags com ID zero (não aprendidas)
      if (result.ID == 0) continue;
      
      // Conversão para coordenadas centradas (0,0)
      int xCart = result.xCenter - 160;  // 160 = centro da imagem
      int yCart = result.yCenter - 120;  // 120 = centro vertical

      // Mostrar os dados no Serial Monitor
      Serial.print("Tag ID=");
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
    Serial.println("Erro ao comunicar com a HuskyLens...");
  }

  delay(300);
}
