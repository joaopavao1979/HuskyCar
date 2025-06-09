// Projeto: HuskyCar Challenge - Seguidor de Cor com HuskyLens (UART)
// Adaptado para Arduino UNO R4 WiFi usando Serial1 (pinos 0 e 1)
// Autor: ChatGPT (adaptado por solicitação do usuário para reconhecimento de cor)
// Data: 2025-06-06
// Hardware: Arduino UNO R4 WiFi + HuskyLens (modo Color Recognition) + L298P + 2 motores DC
// Referência HuskyLens: https://www.the-diy-life.com/adding-ai-vision-to-a-robot-car-using-a-huskylens/

/*
CONFIGURAÇÃO DA HUSKYLENS PARA MODO COLOR RECOGNITION:

1. Ligar a HuskyLens à alimentação e comunicação UART:
   - VCC → 5V do Arduino
   - GND → GND do Arduino
   - TX (verde) → pino 0 do Arduino (RX1, Serial1)
   - RX (azul)  → pino 1 do Arduino (TX1, Serial1)

2. Na HuskyLens:
   - Navegar até ao menu com o botão lateral.
   - Selecionar "Algorithm" → "Color Recognition".
   - Aponte a câmera para a cor desejada e pressione o botão para aprender essa cor.
   - Você pode repetir para aprender até 6 cores distintas (ID 1 a 6), mas para este código
     iremos assumir que só haverá uma cor ensinada (ID 1) e ela será a que o robô seguirá.

3. Conexões Driver L298P ↔ Arduino UNO R4 WiFi:
   - ENA → pino 5   (PWM Motor Esquerdo)
   - IN1 → pino 4   (Direção Motor Esquerdo A)
   - IN2 → pino 3   (Direção Motor Esquerdo B)
   - ENB → pino 6   (PWM Motor Direito)
   - IN3 → pino 7   (Direção Motor Direito A)
   - IN4 → pino 8   (Direção Motor Direito B)
*/

#include "HUSKYLENS.h"

// ------------------------------
// 1. CONFIGURAÇÃO DE PINOS
// ------------------------------

// Serial Monitor (USB) = Serial
// Serial1 (HW-UART)    = pinos digitais 0 (RX1) e 1 (TX1) no UNO R4 WiFi
HUSKYLENS huskylens;  // Usaremos Serial1 para comunicar com a HuskyLens

// Driver L298P – Motor Esquerdo e Motor Direito
const int ENA = 5;   // PWM Motor Esquerdo
const int IN1 = 4;   // Direção Motor Esquerdo A
const int IN2 = 3;   // Direção Motor Esquerdo B

const int ENB = 6;   // PWM Motor Direito
const int IN3 = 7;   // Direção Motor Direito A
const int IN4 = 8;   // Direção Motor Direito B

// ------------------------------
// 2. PARÂMETROS DE IMAGEM & CONTROLE
// ------------------------------

// Assume HuskyLens em modo Color Recognition com largura de imagem 320px
const int IMAGE_WIDTH       = 320;
const int CENTER_X          = IMAGE_WIDTH / 2;

// Ganhos para controle proporcional
const float KP_DIR          = 0.08;   // Ganho direção
const float KP_DIST         = 0.05;   // Ganho distância
const int   MAX_SPEED       = 120;    // Velocidade máxima PWM (0–255)
const int   MIN_SPEED       = 40;     // Velocidade mínima para vencer atrito
const int   TARGET_OBJ_WIDTH = 60;    // Largura alvo (px) do objeto/color blob para distância ideal
const int   TOLERANCE_WIDTH  = 10;    // Faixa de tolerância (px) para não oscilar

// ------------------------------
// 3. FILTRAGEM DAS LEITURAS (MÉDIA MÓVEL)
// ------------------------------

const int FILTER_WINDOW = 5;  // Tamanho do buffer da média móvel
int bufferX[FILTER_WINDOW];   // Buffer circular para xCenter
int bufferW[FILTER_WINDOW];   // Buffer circular para width
int bufferIndex = 0;          // Índice atual de inserção
bool bufferCheio = false;     // Indica se o buffer já foi preenchido uma vez

// ------------------------------
// 4. STRUCT PARA DADOS DA HUSKYLENS (Color Recognition)
// ------------------------------

struct HuskyData {
  bool learned;      // Se a cor foi aprendida
  bool available;    // Se a cor está visível no frame atual
  int rawX;          // xCenter bruto (centro do blob)
  int rawW;          // width bruto (largura do blob)
};

// ------------------------------
// 5. FLAG PARA SABER SE HUSKYLENS CONECTOU
// ------------------------------

bool huskyOk = false;

// ------------------------------
// 6. FUNÇÕES DE INICIALIZAÇÃO
// ------------------------------

/**
 * Inicializa Serial (USB) e Serial1 (HW-UART) para HuskyLens.
 * Tenta conectar por até 3 segundos; se falhar, huskyOk = false,
 * mas não bloqueia o setup. 
 */
void initSerialAndHusky() {
  Serial.begin(9600);
  Serial1.begin(9600);  // Comunicação via UART de hardware: pinos 0 e 1
  
  Serial.println(F("[INFO] Inicializando HuskyLens (Color Recognition) em Serial1..."));
  unsigned long inicio = millis();
  
  // Tenta conectar por até 3000 ms
  while (millis() - inicio < 3000) {
    if (huskylens.begin(Serial1)) {
      huskyOk = true;
      break;
    }
    delay(200);
  }
  
  if (!huskyOk) {
    Serial.println(F("[ERRO] Não consegui conectar à HuskyLens. Seguiremos sem visão."));
    return;
  }
  
  // Se conectado, configura algoritmo de Color Recognition
  huskylens.writeAlgorithm(ALGORITHM_COLOR_RECOGNITION);
  Serial.println(F("[INFO] HuskyLens conectado e em modo Color Recognition."));
  Serial.println(F("[INFO] Certifique-se de ter ensinado a cor desejada (ID 1) na HuskyLens."));
}

/**
 * Configura pinos dos motores como OUTPUT e garante que iniciam parados.
 */
void initMotors() {
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  stopMotors();  // Garante parada inicial
}

/**
 * Inicializa buffers de média móvel com valores neutros:
 *  • bufferX[] = CENTER_X (supõe blob centralizado)
 *  • bufferW[] = TARGET_OBJ_WIDTH (supõe distância ideal)
 */
void initFilterBuffers() {
  for (int i = 0; i < FILTER_WINDOW; i++) {
    bufferX[i] = CENTER_X;
    bufferW[i] = TARGET_OBJ_WIDTH;
  }
  bufferIndex = 0;
  bufferCheio = false;
}

// ------------------------------
// 7. FUNÇÕES DE FILTRAGEM (MÉDIA MÓVEL)
// ------------------------------

/**
 * Insere leituras brutas de xCenter e width nos buffers circulares
 * e retorna a média móvel de xCenter (inteiro).
 *
 * @param x Leitura bruta de xCenter da HuskyLens
 * @param w Leitura bruta de width do blob
 * @return Média móvel inteira de xCenter
 */
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
  for (int i = 0; i < count; i++) {
    somaX += bufferX[i];
  }
  return somaX / count;
}

/**
 * Retorna a média móvel inteira de width (usada para controle de distância).
 *
 * @return Média móvel de width
 */
int filterW() {
  int somaW = 0;
  int count = bufferCheio ? FILTER_WINDOW : bufferIndex;
  for (int i = 0; i < count; i++) {
    somaW += bufferW[i];
  }
  return somaW / count;
}

// ------------------------------
// 8. FUNÇÃO DE LEITURA DA HUSKYLENS
// ------------------------------

/**
 * Lê a HuskyLens via Serial1:
 *  1. Faz huskylens.request() para solicitar novo frame.
 *  2. Checa huskylens.isLearned() e huskylens.available().
 *  3. Se OK, lê result.xCenter e result.width.
 *  4. Senão, marca learned=false ou available=false e retorna valores default.
 *
 * @return HuskyData contendo flags e valores brutos
 */
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

// ------------------------------
// 9. FUNÇÃO DE CÁLCULO PROPORCIONAL
// ------------------------------

/**
 * Calcula as velocidades (podem ser negativas ou positivas)
 * para motor esquerdo (speedL) e motor direito (speedR) com base em:
 *  • filteredX: média móvel de xCenter (0..IMAGE_WIDTH)
 *  • filteredW: média móvel de width (pixels)
 *
 * Utiliza controle proporcional em dois laços:
 * 1. Direção: corrDir = KP_DIR * errorX (erro horizontal)
 * 2. Distância: baseSpeed = KP_DIST * errorW (erro de largura)
 *
 * Speed = baseSpeed ± corrDir → depois aplica:
 *  • Se |raw| < MIN_SPEED → raw = 0 (evita vibração sem força)
 *  • Constrain(raw, -MAX_SPEED..+MAX_SPEED)
 *
 * @param filteredX Média móvel de xCenter
 * @param filteredW Média móvel de width
 * @param &speedL  Referência para armazenar velocidade final do lado esquerdo
 * @param &speedR  Referência para armazenar velocidade final do lado direito
 */
void computeMotorSpeeds(int filteredX, int filteredW, int &speedL, int &speedR) {
  // 9.1. Erros
  int errorX = filteredX - CENTER_X;             // negativo → blob à esquerda
  int errorW = TARGET_OBJ_WIDTH - filteredW;      // positivo → blob longe
  
  // 9.2. Correção direcional proporcional
  float corrDir = KP_DIR * (float)errorX;
  if (corrDir > MAX_SPEED)  corrDir = MAX_SPEED;
  if (corrDir < -MAX_SPEED) corrDir = -MAX_SPEED;
  
  // 9.3. Velocidade base proporcional à distância
  float baseSpeed = KP_DIST * (float)errorW;
  // Se blob dentro da faixa de tolerância, não se aproxima/afasta
  if (abs(errorW) <= TOLERANCE_WIDTH) {
    baseSpeed = 0.0;
  }
  // Caso errorW seja negativo grande, baseSpeed negativo fará recuar
  
  // 9.4. Combina direção e distância
  float rawLeft  = baseSpeed - corrDir;  // motor esquerdo subtrai correção direcional
  float rawRight = baseSpeed + corrDir;  // motor direito soma correção direcional
  
  // 9.5. Evita que motor fique girando abaixo do limiar (vibração inútil)
  if (abs(rawLeft)  < MIN_SPEED) rawLeft  = 0;
  if (abs(rawRight) < MIN_SPEED) rawRight = 0;
  
  // 9.6. Constrói limites finais
  rawLeft  = constrain(rawLeft,  -MAX_SPEED, MAX_SPEED);
  rawRight = constrain(rawRight, -MAX_SPEED, MAX_SPEED);
  
  speedL = (int)rawLeft;
  speedR = (int)rawRight;
}

// ------------------------------
// 10. FUNÇÕES DE CONTROLE DOS MOTORES
// ------------------------------

/**
 * Converte speedL/speedR (podem ser negativos ou positivos)
 * em sinais digitais e PWM para o driver L298P:
 *  • speed > 0  → direção para frente (INx=HIGH, INx+1=LOW; PWM = speed)
 *  • speed < 0  → direção para trás (INx=LOW, INx+1=HIGH; PWM = -speed)
 *  • speed == 0 → para (INx=LOW, INx+1=LOW; PWM = 0)
 *
 * @param speedL Velocidade desejada do motor esquerdo [-MAX_SPEED..+MAX_SPEED]
 * @param speedR Velocidade desejada do motor direito  [-MAX_SPEED..+MAX_SPEED]
 */
void setMotorSpeeds(int speedL, int speedR) {
  // Motor Esquerdo (ENA, IN1, IN2)
  if (speedL > 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, speedL);
  }
  else if (speedL < 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, -speedL);
  }
  else {
    // freio ativo
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
  }

  // Motor Direito (ENB, IN3, IN4)
  if (speedR > 0) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, speedR);
  }
  else if (speedR < 0) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, -speedR);
  }
  else {
    // freio ativo
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 0);
  }
}

/**
 * Para totalmente ambos os motores (freio ativo).
 */
void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// ------------------------------
// 11. SETUP E LOOP PRINCIPAL
// ------------------------------

void setup() {
  initSerialAndHusky();    // Tenta conectar HuskyLens via Serial1 (pinos 0/1)
  initMotors();            // Configura pinos dos motores e garante parada
  initFilterBuffers();     // Inicializa buffers de média móvel
  Serial.println(F("[INFO] Setup completo. Entrando no loop principal."));
}

void loop() {
  // 11.1. Se falhou na HuskyLens, mantém motores parados e fica em loop leve
  if (!huskyOk) {
    stopMotors();
    delay(100);
    return;
  }

  // 11.2. Lê dados da HuskyLens
  HuskyData hd = readHuskyLens();

  // 11.3. Se a cor (blob) não foi aprendida, para motores e retorna
  if (!hd.learned) {
    Serial.println(F("[AVISO] Cor não aprendida. Motores parados."));
    stopMotors();
    delay(100);
    return;
  }

  // 11.4. Se o blob de cor não estiver visível, para motores e retorna
  if (!hd.available) {
    Serial.println(F("[AVISO] Cor não visível. Motores parados."));
    stopMotors();
    delay(100);
    return;
  }

  // 11.5. Filtra leituras (média móvel) de xCenter e width
  int filtX = filterX(hd.rawX, hd.rawW);
  int filtW = filterW();

  // 11.6. Calcula velocidade para cada motor baseado na posição e distância
  int velL = 0, velR = 0;
  computeMotorSpeeds(filtX, filtW, velL, velR);

  // 11.7. Aciona motores conforme velocidade calculada
  setMotorSpeeds(velL, velR);

  // 11.8. Debug: imprime valores relevantes no Serial Monitor
  Serial.print(F("[DEBUG] rawX="));   Serial.print(hd.rawX);
  Serial.print(F("  filtX="));         Serial.print(filtX);
  Serial.print(F("  rawW="));         Serial.print(hd.rawW);
  Serial.print(F("  filtW="));         Serial.print(filtW);
  Serial.print(F("  velL="));         Serial.print(velL);
  Serial.print(F("  velR="));         Serial.println(velR);

  // 11.9. Breve intervalo (~50 Hz)
  delay(20);
}
