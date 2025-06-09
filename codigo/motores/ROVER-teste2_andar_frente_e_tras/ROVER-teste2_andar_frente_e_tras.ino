// ============================================================================
//                          ROBÔ COM MOTOR SHIELD L298P
// ============================================================================

// ----------------------
// 1. Mapeamento dos pinos
// ----------------------
// E1/E2: pinos PWM que controlam a velocidade de cada motor
// M1/M2: pinos digitais que definem o sentido de rotação
const int E1 = 10;  // PWM velocidade motor A (direita)
const int M1 = 12;  // Direção motor A
const int E2 = 11;  // PWM velocidade motor B (esquerda)
const int M2 = 13;  // Direção motor B (invertido no código)

// ----------------------
// 2. Configuração de parâmetros
// ----------------------
const int VELOCIDADE = 180; 
// Valor entre 0 e 255. Ajusta para controlar a “suavidade” do movimento.

// ============================================================================
// 3. Configuração inicial
// ============================================================================

void setup() {
  // Configura todos os pinos como saída
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);
  pinMode(E1, OUTPUT);
  pinMode(E2, OUTPUT);

  // --------------------------------------------------------------------------
  // Sequência automática de teste (corre apenas uma vez no arranque)
  // --------------------------------------------------------------------------
  
  // 3.1 – Anda para a frente durante 5 segundos
  andarFrente(VELOCIDADE);
  delay(5000);

  // 3.2 – Paragem com freio ativo durante 4 segundos
  parar();
  delay(4000);

  // 3.3 – Anda para trás durante 3 segundos
  andarTras(VELOCIDADE);
  delay(3000);

  // 3.4 – Para novamente com freio ativo
  parar();
}

// ============================================================================
// 4. Loop principal
// ============================================================================

void loop() {
  // Este robô executa apenas a sequência no setup().
  // Se quiseres um comportamento contínuo, podes chamar funções aqui.
}

// ============================================================================
// 5. Funções de controlo de movimento
// ============================================================================

/**
 * Anda para a frente.
 * @param velocidade Valor PWM (0–255) para controlar a potência dos motores.
 */
void andarFrente(int velocidade) {
  // Define sentido de rotação: HIGH = frente para ambos os motores
  digitalWrite(M1, HIGH);  
  digitalWrite(M2, LOW);   // motor B invertido para alinhar sentido físico
  
  // Aplica PWM para velocidade
  analogWrite(E1, velocidade);
  analogWrite(E2, velocidade);
}

/**
 * Anda para trás.
 * @param velocidade Valor PWM (0–255) para controlar a potência dos motores.
 */
void andarTras(int velocidade) {
  // Define sentido inverso: LOW = trás para motor A, HIGH = trás para motor B
  digitalWrite(M1, LOW);   
  digitalWrite(M2, HIGH);  

  // Aplica PWM para velocidade
  analogWrite(E1, velocidade);
  analogWrite(E2, velocidade);
}

/**
 * Para ambos os motores com freio ativo.
 * Corta o PWM (velocidade) e força as saídas de direção a LOW,
 * gerando um “brake” que segura o rotor dos motores.
 */
void parar() {
  // 1. Desativa o PWM
  analogWrite(E1, 0);
  analogWrite(E2, 0);

  // 2. Força os pinos de direção a LOW para ativar o freio interno da ponte H
  digitalWrite(M1, LOW);
  digitalWrite(M2, LOW);
}

// ============================================================================
// Fim do código
// ============================================================================
