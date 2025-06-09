// ============================================================================
//              ROBÔ COM L298P – ROTAÇÃO CONTROLADA COM MENU SERIAL
// ============================================================================
// Autor: João Pavão (com apoio do ChatGPT)
// Descrição: Roda no lugar para a direita (sentido anti-horário), com passos de 30º.
// Permite ajustar os parâmetros via Monitor Serial de forma interativa.
// ============================================================================

// ----------------------------
// 1. Mapeamento dos pinos
// ----------------------------
const int E1 = 10;  // PWM motor A (direita)
const int M1 = 12;  // Direção motor A
const int E2 = 11;  // PWM motor B (esquerda)
const int M2 = 13;  // Direção motor B (invertido fisicamente)

// ----------------------------
// 2. Parâmetros configuráveis
// ----------------------------
int passos = 12;                // Nº de passos (ex: 12 → 360º)
int velocidade = 200;           // Velocidade (PWM 0–255)
int duracaoRotacao = 300;       // Tempo de rotação de 30º (ms)
int pausaEntrePassos = 500;     // Tempo entre passos (ms)

// ============================================================================
// 3. Setup – Inicia o menu
// ============================================================================
void setup() {
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);
  pinMode(E1, OUTPUT);
  pinMode(E2, OUTPUT);

  Serial.begin(9600);
  delay(1000); // Espera pela conexão

  Serial.println("\n--- ROBÔ HUSKYCAR – MENU DE ROTAÇÃO ---");
}

// ============================================================================
// 4. Loop principal – Menu interativo via Serial
// ============================================================================
void loop() {
  mostrarMenu();

  while (!Serial.available()) {
    delay(100);
  }

  String opcaoStr = Serial.readStringUntil('\n');
  int opcao = opcaoStr.toInt();

  switch (opcao) {
    case 1:
      Serial.print("Novo número de passos: ");
      passos = lerValor();
      break;

    case 2:
      Serial.print("Nova velocidade (0–255): ");
      velocidade = lerValor();
      break;

    case 3:
      Serial.print("Nova duração da rotação (ms): ");
      duracaoRotacao = lerValor();
      break;

    case 4:
      Serial.print("Nova pausa entre passos (ms): ");
      pausaEntrePassos = lerValor();
      break;

    case 5:
      Serial.println("\n🔁 Executando rotação no lugar (sentido anti-horário)...");
      for (int i = 0; i < passos; i++) {
        Serial.print("🔄 Passo "); Serial.println(i + 1);
        girarDireita();
        delay(pausaEntrePassos);
      }
      parar();
      Serial.println("✅ Rotação concluída.");
      break;

    case 6:
      mostrarParametros();
      break;

    case 0:
      Serial.println("🔄 Menu reiniciado.\n");
      break;

    default:
      Serial.println("⚠️ Opção inválida.");
      break;
  }

  while (Serial.available()) Serial.read(); // limpa o buffer
}

// ============================================================================
// 5. Funções auxiliares de menu
// ============================================================================
void mostrarMenu() {
  Serial.println("\n==========================");
  Serial.println("Escolhe uma opção:");
  Serial.println("1. Alterar número de passos");
  Serial.println("2. Alterar velocidade (PWM)");
  Serial.println("3. Alterar tempo de rotação (ms)");
  Serial.println("4. Alterar pausa entre passos (ms)");
  Serial.println("5. Executar rotação");
  Serial.println("6. Mostrar parâmetros atuais");
  Serial.println("0. Repetir menu");
  Serial.print(">> ");
}

void mostrarParametros() {
  Serial.println("\n--- PARÂMETROS ATUAIS ---");
  Serial.print("🔄 Passos: "); Serial.println(passos);
  Serial.print("⚡ Velocidade: "); Serial.println(velocidade);
  Serial.print("⏱️ Duração da rotação: "); Serial.print(duracaoRotacao); Serial.println(" ms");
  Serial.print("⏸️ Pausa entre passos: "); Serial.print(pausaEntrePassos); Serial.println(" ms");
}

int lerValor() {
  while (!Serial.available()) {
    delay(100);
  }

  String entrada = Serial.readStringUntil('\n'); // Espera até Enter
  int valor = entrada.toInt();
  Serial.println(valor); // Confirma o valor
  return valor;
}

// ============================================================================
// 6. Funções de movimento do robô
// ============================================================================
void girarDireita() {
  Serial.println("🛞 A girar para a DIREITA (sentido anti-horário)");

  // Inversão dos sentidos para girar na direção certa
  digitalWrite(M1, LOW);   // Motor A (direita) → trás
  digitalWrite(M2, LOW);   // Motor B (esquerda) → frente (ajustado)

  analogWrite(E1, velocidade);
  analogWrite(E2, velocidade);

  delay(duracaoRotacao);
  parar();
}

void parar() {
  analogWrite(E1, 0);
  analogWrite(E2, 0);
  digitalWrite(M1, LOW);
  digitalWrite(M2, LOW);
}
