// Bibliotecas necessárias para comunicar com o HuskyLens e usar porta serial alternativa
#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

// Criação da porta serial virtual nos pinos 10 (RX) e 11 (TX)
SoftwareSerial mySerial(8, 9); 
HUSKYLENS huskylens; // Instância do objeto HuskyLens

void setup() {
  // Inicia a comunicação com o computador (porta USB)
  Serial.begin(115200);     

  // Inicia a comunicação com o HuskyLens (Serial 9600 configurado no próprio sensor)
  mySerial.begin(9600);     

  // Tenta estabelecer comunicação com o HuskyLens
  while (!huskylens.begin(mySerial)) {
    Serial.println("Erro ao iniciar HuskyLens. Verifica a ligação e protocolo Serial 9600.");
    delay(1000);
  }
  Serial.println("HuskyLens pronto!");
}

void loop() {
  // Solicita dados ao HuskyLens
  if (!huskylens.request()) {
    Serial.println("Erro ao pedir dados do HuskyLens.");
  }
  // Verifica se algo foi aprendido no modo atual (ex: cor ou objeto)
  else if (!huskylens.isLearned()) {
    Serial.println("Nada aprendido. Pressiona o botão no HuskyLens para ensinar.");
  }
  // Verifica se há algum bloco/objeto visível
  else if (!huskylens.available()) {
    Serial.println("Nenhum objeto detetado.");
  }
  else {
    // Enquanto houver objetos visíveis
    while (huskylens.available()) {
      HUSKYLENSResult result = huskylens.read(); // Lê o resultado atual

      // Transforma coordenadas do HuskyLens em sistema cartesiano
      // Centro da imagem: (160, 120) → agora passa a ser (0, 0)
      int x_c = result.xCenter - 160;
      int y_c = result.yCenter - 120;

      // Envia os dados transformados para o computador via USB
      Serial.print("Cartesiano:");
      Serial.print("x=");
      Serial.print(x_c);
      Serial.print(",y=");
      Serial.print(y_c);
      Serial.print(",largura=");
      Serial.print(result.width);
      Serial.print(",altura=");
      Serial.print(result.height);
      Serial.print(",ID=");
      Serial.println(result.ID);
    }
  }

  // Aguarda um pouco antes de repetir (ajustável)
  delay(500);
}
