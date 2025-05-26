// Projeto: HuskyCar Challenge - Seguidor de Linha com HuskyLens (UART)
// Autores: João Pavão e Tiago Martins (projeto HuskyCar Challenge)
// Código adaptado por ChatGPT com base no trabalho original
// Data: 2025-05-26
// Hardware: Arduino UNO  + HuskyLens (UART) + L298P + 2 motores DC
// Baseado no projeto de Michael Klements (The DIY Life)
// Referência completa: https://www.the-diy-life.com/adding-ai-vision-to-a-robot-car-using-a-huskylens/

/*
CONFIGURAÇÃO DA HUSKYLENS PARA MODO SEGUE-LINHA:

1. Ligar a HuskyLens à alimentação e comunicação UART:
   - VCC → 5V do Arduino
   - GND → GND do Arduino
   - TX → Pino 10 do Arduino (RX do SoftwareSerial)
   - RX → Pino 11 do Arduino (TX do SoftwareSerial)

2. Na HuskyLens:
   - Navegar para o menu com o botão lateral.
   - Selecionar "Algorithm" e escolher "Line Tracking".
   - Apontar a câmara para uma linha (ex: seta preta sobre papel branco).
   - Alinhar a cruz central com a linha e pressionar o botão para aprender o padrão.
   - Após a aprendizagem, a HuskyLens exibirá uma seta azul sobre a linha detetada.
   - A câmara irá seguir essa linha mesmo se ela desaparecer temporariamente do campo de visão.

3. Este código utiliza as coordenadas da seta detetada (xTarget, yTarget)
   para aplicar correções nos motores com base num controlo proporcional (P).
*/

#include <SoftwareSerial.h>
#include "HUSKYLENS.h"

// Cria conexão serial para HuskyLens nos pinos 10 (RX) e 11 (TX)
SoftwareSerial huskySerial(10, 11);
HUSKYLENS huskylens;

// Definição dos pinos do driver L298P
const int ENA = 5; // PWM Motor A (esquerdo)
const int IN1 = 4; // Direção Motor A
const int IN2 = 3;

const int ENB = 6; // PWM Motor B (direito)
const int IN3 = 7; // Direção Motor B
const int IN4 = 8;

// Variáveis de controlo
int velocidadeBase = 90;           // Velocidade normal dos motores (valor base)
int ganhoProporcional = 1;         // Ganho P do controlador proporcional (ajustável)
int larguraImagem = 320;           // Largura da imagem da HuskyLens (resolução horizontal)
int alturaImagem = 240;            // Altura da imagem da HuskyLens (resolução vertical)
int centroImagemX = larguraImagem / 2; // Centro horizontal da imagem (X = 160)
int centroImagemY = alturaImagem / 2;  // Centro vertical da imagem (Y = 120)

void setup() {
  Serial.begin(9600);             // Inicia comunicação serial com o computador
  huskySerial.begin(9600);        // Inicia comunicação UART com a HuskyLens

  // Inicializa a HuskyLens
  while (!huskylens.begin(huskySerial)) {
    Serial.println("Falha na comunicação com a HuskyLens. Verifique conexões e modo UART.");
    delay(1000);
  }

  // Define o algoritmo da HuskyLens como "Line Tracking"
  huskylens.writeAlgorithm(ALGORITHM_LINE_TRACKING);

  // Define os pinos dos motores como saída
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  parar(); // Garante que o robô está parado ao iniciar
}

void loop() {
  // Solicita novos dados da HuskyLens
  if (!huskylens.request()) {
    Serial.println("Sem resposta da HuskyLens.");
    parar(); // Paragem automática se perder a comunicação
    return;
  }

  // Verifica se há algum alvo (linha) detetado
  if (!huskylens.available()) {
    Serial.println("Linha não detetada.");
    parar(); // Paragem automática se não houver linha visível
    return;
  }

  // Lê o resultado atual da HuskyLens
  HUSKYLENSResult resultado = huskylens.read();

  // Transforma coordenadas da imagem para referencial cartesiano (centro = 0,0)
  int xCartesiano = resultado.xTarget - centroImagemX;           // X: positivo à direita, negativo à esquerda
  int yCartesiano = centroImagemY - resultado.yTarget;           // Y: positivo para cima, negativo para baixo

  // Calcula a correção proporcional com base no erro em X (posição horizontal)
  int correcao = xCartesiano * ganhoProporcional;

  // Ajusta a velocidade de cada motor em função da correção
  int velocidadeEsquerda = velocidadeBase + correcao;
  int velocidadeDireita = velocidadeBase - correcao;

  // Garante que os valores ficam entre 0 e 255 (limites do PWM)
  velocidadeEsquerda = constrain(velocidadeEsquerda, 0, 255);
  velocidadeDireita = constrain(velocidadeDireita, 0, 255);

  // Envia os valores de velocidade para os motores
  moverFrente(velocidadeEsquerda, velocidadeDireita);

  // Mostra as variáveis no monitor serial para depuração
  Serial.print("Erro (xCartesiano): "); Serial.print(xCartesiano);
  Serial.print(" | Y: "); Serial.print(yCartesiano);
  Serial.print(" | VE: "); Serial.print(velocidadeEsquerda);
  Serial.print(" | VD: "); Serial.println(velocidadeDireita);

  delay(50); // Aguarda 50ms antes do próximo ciclo (controla a taxa de atualização)
}

// Função para mover o robô para a frente com velocidades individuais para cada lado
void moverFrente(int velE, int velD) {
  // Motor A (esquerda)
  digitalWrite(IN1, HIGH); // Roda para a frente
  digitalWrite(IN2, LOW);
  analogWrite(ENA, velE);  // Aplica velocidade

  // Motor B (direita)
  digitalWrite(IN3, HIGH); // Roda para a frente
  digitalWrite(IN4, LOW);
  analogWrite(ENB, velD);  // Aplica velocidade
}

// Função para parar completamente os dois motores
void parar() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}