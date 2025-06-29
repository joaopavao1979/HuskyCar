# Projeto HuskyCar 🚗📸

# HuskyCar Project – Organização de Diretórios

Este repositório contém os diferentes módulos, exemplos e ferramentas utilizadas no desenvolvimento do projeto **HuskyCar Challenge**, baseado em visão computacional com HuskyLens, controlo de motores L298P, e integração com Arduino UNO R4 WiFi.

## Estrutura de Pastas

### 📂 bibliotecas
Contém bibliotecas externas necessárias ao projeto (ex: `HUSKYLENS`, `SoftwareSerial`, entre outras). Importar para a IDE do Arduino antes de compilar.

### 📂 calibracao
Scripts e exemplos dedicados à **calibração de sensores**, motores ou componentes físicos do robô. Inclui sketches auxiliares para testar gamas de movimento ou parâmetros iniciais.

### 📂 comunicacao
Códigos que implementam **protocolos de comunicação** (ex: UART, I2C, comandos seriais entre Arduino e computador ou módulos externos).

### 📂 exemplos
Conjunto de exemplos básicos e pedagógicos para aprendizagem ou testes iniciais com componentes do projeto. Pode incluir casos mínimos de uso com sensores, motores ou a HuskyLens.

### 📂 final
Versões estabilizadas e completas do sistema, preparadas para demonstração, apresentação ou uso competitivo. Aqui estão os códigos prontos para execução integral.

### 📂 motores
Testes e funções isoladas de controlo de **motores DC via L298P**. Inclui sketches de testes manuais, PWM, rotação bidirecional, etc.

### 📂 Multiple_recognition
Códigos que utilizam a HuskyLens em **modo de reconhecimento múltiplo** (ex: várias cores, tags ou rostos). Inclui lógica para distinguir entre diferentes IDs e realizar ações associadas.

### 📂 sensores
Exemplos e testes com sensores adicionais (ex: ultrassónicos, temperatura, luz, etc.), que podem ser usados para complementar a visão do sistema.

### 📂 servo
Controlo de **servomotores** isoladamente. Ideal para testes de precisão, ângulos limite e tempo de resposta.
---

## 📄 Outros ficheiros

### `README.md`
Este ficheiro. Explica a organização do repositório e serve como ponto de entrada ao projeto.
---

## 🧠 Notas finais

Este repositório foi concebido com fins **educativos, exploratórios e competitivos**, permitindo a modularização dos testes e o desenvolvimento incremental de funcionalidades. Recomenda-se manter esta estrutura e documentar cada sketch com um comentário inicial indicando:

- Nome do ficheiro
- Objetivo
- Hardware necessário
- Modo da HuskyLens (se aplicável)
---
## Créditos

Projeto desenvolvido por:
- **João Pavão**
- **Tiago Martins**

Com orientação dos docentes:
- Prof. **Armando B. Mendes** (ML)
- Prof. **Mathias Funck** (Programação)
- Prof. **José Cascalho** (IA e Robótica)
