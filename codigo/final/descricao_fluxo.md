# Descrição do Fluxo do Código Final

Este ficheiro descreve a lógica do código `huskycar_completo.ino` utilizado na versão final do HuskyCar.

## Objetivo

Integrar os principais módulos desenvolvidos:
- Controlo de motores
- Visão computacional com HuskyLens
- Reações automáticas com base em deteções visuais

## Estrutura

1. `setup()`:
   - Inicializa a comunicação Serial
   - Configura motores, sensores e a HuskyLens

2. `loop()`:
   - O carro segue uma linha usando a HuskyLens
   - Se detetar um rosto → para o robô
   - Se encontrar um objeto → desvia
   - Se a linha desaparecer → procura girando lentamente

## Futuras melhorias

- Implementar timer para tempo de resposta
- Adicionar função de segurança para parar após inatividade
