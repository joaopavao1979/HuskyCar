// Projeto: HuskyCar Challenge - Seguidor de Objeto com HuskyLens (UART)
// Autores: João Pavão e Tiago Martins (projeto HuskyCar Challenge)
// Código adaptado por ChatGPT com base no trabalho de Michael Klements (The DIY Life)
// Data: 2025-05-26
// Hardware: Arduino UNO R4 WiFi + HuskyLens (UART) + L298P + 2 motores DC
// Referência completa: https://www.the-diy-life.com/adding-ai-vision-to-a-robot-car-using-a-huskylens/

/*
CONFIGURAÇÃO DA HUSKYLENS PARA MODO SEGUE-OBJETO:

1. Ligar a HuskyLens à alimentação e comunicação UART:
   - VCC → 5V do Arduino
   - GND → GND do Arduino
   - TX → Pino 10 do Arduino (RX do SoftwareSerial)
   - RX → Pino 11 do Arduino (TX do SoftwareSerial)

2. Na HuskyLens:
   - Navegar até ao menu com o botão lateral.
   - Selecionar "Algorithm" > "Object Tracking".
   - Apontar a câmara para o objeto desejado e pressionar o botão para aprender.
   - Após aprendizagem, a HuskyLens reconhecerá o objeto e mostrará um retângulo sobre ele.

3. Este código permite ao robô seguir o objeto aprendido, ajustando a sua direção e velocidade
   com base na posição e largura do objeto na imagem.
*/

#include <SoftwareSerial.h>
#include "HUSKYLENS.h"

// Define a comunicação UART com a HuskyLens nos pinos 10 e 11
SoftwareSerial huskySerial(10, 11); // RX, TX
HUSKYLENS huskylens;

// Definição dos pinos do driver L298P
const int ENA = 5; const int IN1 = 4; const int IN2 = 3;
const int ENB = 6; const int IN3 = 7; const int IN4 = 8;

// Parâmetros ajustáveis para comportamento do robô
int larguraImagem = 320;                  // Largura da imagem captada pela HuskyLens
int centroImagemX = larguraImagem / 2;    // Ponto central do eixo X da imagem
int objectoAlvoLargura = 50;              // Largura desejada do objeto a seguir
int margemCentro = 20;                    // Margem de tolerância ao redor do centro
int ganhoDirecao = 12;                    // Ganho proporcional para ajuste de direção
int ganhoDistancia = 6;                   // Ganho proporcional para ajuste de aproximação
int velocidadeMaxima = 50;                // Velocidade máxima permitida para os motores
int motorOffset = 25;                     // Correção de assimetria entre os motores

// -----------------------------------------------------------------------------
// Função de configuração inicial (executada uma única vez)
// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);               // Inicia comunicação serial com o computador
  huskySerial.begin(9600);          // Inicia comunicação UART com a HuskyLens

  // Tenta estabelecer comunicação com a HuskyLens
  while (!huskylens.begin(huskySerial)) {
    Serial.println("Falha na comunicação com a HuskyLens.");
    delay(1000);
  }

  // Define o modo de funcionamento da HuskyLens como seguimento de objeto
  huskylens.writeAlgorithm(ALGORITHM_OBJECT_TRACKING);

  // Define os pinos dos motores como saídas
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  parar(); // Garante que o robô está parado ao iniciar
}

// -----------------------------------------------------------------------------
// Função principal de controlo (executada ciclicamente)
// -----------------------------------------------------------------------------
void loop() {
  // Solicita dados atualizados da HuskyLens
  if (!huskylens.request()) {
    Serial.println("Sem resposta da HuskyLens.");
    parar(); return;
  }

  // Verifica se o objeto foi aprendido anteriormente
  if (!huskylens.isLearned()) {
    Serial.println("Objeto não aprendido.");
    parar(); return;
  }

  // Verifica se o objeto está visível no campo de visão
  if (!huskylens.available()) {
    Serial.println("Objeto não visível.");
    parar(); return;
  }

  // Lê os dados do objeto detetado
  HUSKYLENSResult resultado = huskylens.read();

  // Converte a posição horizontal para um sistema cartesiano com origem no centro
  int xCartesiano = resultado.xCenter - centroImagemX;

  // Inicializa variáveis de velocidade para os motores esquerdo e direito
  int velE = 0, velD = 0;

  // -------------------------------------------------------------------------
  // Correção de direção: ajusta os motores se o objeto estiver fora do centro
  // -------------------------------------------------------------------------
  if (xCartesiano < -margemCentro) {
    velE = -ganhoDirecao * abs(xCartesiano);
    velD =  ganhoDirecao * abs(xCartesiano);
  } else if (xCartesiano > margemCentro) {
    velE =  ganhoDirecao * abs(xCartesiano);
    velD = -ganhoDirecao * abs(xCartesiano);
  }

  // -------------------------------------------------------------------------
  // Correção de distância: aproxima-se do objeto se estiver pequeno (longe)
  // -------------------------------------------------------------------------
  if (resultado.width < objectoAlvoLargura) {
    int avanco = ganhoDistancia * (objectoAlvoLargura - resultado.width);
    velE += avanco;
    velD += avanco;
  }

  // -------------------------------------------------------------------------
  // Limitação das velocidades e envio dos valores para os motores
  // -------------------------------------------------------------------------
  velE = constrain(velE, 0, velocidadeMaxima);
  velD = constrain(velD, 0, velocidadeMaxima);

  // Move o robô para a frente com as velocidades calculadas
  moverFrente(velE + motorOffset, velD);

  // Mostra as variáveis para depuração
  Serial.print("xCentro: "); Serial.print(resultado.xCenter);
  Serial.print(" | Largura: "); Serial.print(resultado.width);
  Serial.print(" | VE: "); Serial.print(velE);
  Serial.print(" | VD: "); Serial.println(velD);

  delay(50); // Aguarda 50ms entre ciclos
}

// -----------------------------------------------------------------------------
// Função para mover o robô para a frente com velocidades individuais
// -----------------------------------------------------------------------------
void moverFrente(int velE, int velD) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); analogWrite(ENA, velE);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); analogWrite(ENB, velD);
}

// -----------------------------------------------------------------------------
// Função para parar completamente os motores (em caso de falha ou perda de alvo)
// -----------------------------------------------------------------------------
void parar() {
  analogWrite(ENA, 0); analogWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}
