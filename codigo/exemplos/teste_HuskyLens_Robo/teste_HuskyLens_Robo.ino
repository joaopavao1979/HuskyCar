/***************************************************
                HUSKYLENS Robot
****************************************************/

/***********Notice and Trouble shooting***************
 1.Connection and Diagram can be found here
 <https://wiki.dfrobot.com/HUSKYLENS_V1.0_SKU_SEN0305_SEN0336#target_23>
 2.This code is tested on Arduino Uno, Leonardo, Mega boards.
 ****************************************************/

// Importação de Bibliotecas
#include "HUSKYLENS.h"
#include "SoftwareSerial.h"
#include <Servo.h>

// Criação das Variáveis para controle da câmara e dos motores
HUSKYLENS huskylens;
SoftwareSerial huskySerial(2, A0); // RX, TX            //HUSKYLENS green line >> Pin 2; blue line >> Pin A0
Servo servoMotor;                                       //Servo orange line >> Pin 9; red line >> 5V; brown line >> GND

// Pinos
int E1 = 10; 
int E2 =11;
int M1 = 12;
int M2 = 13;

// Estados
int last_move = 0;                  // 0 - not moved; 1 - Left; 2 - Right; 3 - Front; 4 - Back;
bool recovering = false;

// Parâmetros da imagem da HuskyLens
const int imgWidth = 320;
const int imgHeight = 240;

// Zona morta
const int deadZoneX = 25;  // esquerda/direita
const int deadZoneY = 20;  // aproximação

void setup() {
    pinMode(M1, OUTPUT);  
    pinMode(M2, OUTPUT);
    pinMode(E1, OUTPUT);  
    pinMode(E2, OUTPUT);

    Serial.begin(9600);
    huskySerial.begin(9600);

    while (!huskylens.begin(huskySerial)){
        Serial.println(F("Begin failed!"));
        Serial.println(F("1. Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
        Serial.println(F("2. Please recheck the connection."));
        delay(500);
    }
    
    Serial.println(F("Begin started!"));
}

void loop() {
    if (!huskylens.request()){ // Verificação se a câmara está conectada
        Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
        stop();
    }else if(!huskylens.isLearned()){ // Verificação se a câmara está com o estado aprendido
        Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
        stop();
    }else if(!huskylens.available()){ // Verificação se o obejecto aprendido pela câmara está visivel
        Serial.println(F("No block or arrow appears on the screen!"));
        recoverLostTarget();
        delay(500);
        stop();
        delay(600); 
    }else if(huskylens.available()){ // Verificação quando o objecto está em área visivel pela câmara
        Serial.println(F("Object found!"));
        HUSKYLENSResult result = huskylens.read();
        checkResult(result);
        delay(500);
        stop();
        delay(600); 
    }
}

// Função para verificar o resultado dos dados enviados pela câmara
void checkResult(HUSKYLENSResult result){
    if (result.command == COMMAND_RETURN_BLOCK){ // Câmara no modo objecto
        // Cálculos para centrar as coordenadas ao centro da câmara
        int newX = result.xCenter - (imgWidth / 2);
        int newY = result.yCenter - (imgHeight / 2);
        Serial.println(String()+F("Block:xCenter=")+newX+F(",yCenter=")+newY+F(",width=")+result.width+F(",height=")+result.height+F(",ID=")+result.ID);
        
        doMovement(newX, newY);
        recovering = false;

    }else if (result.command == COMMAND_RETURN_ARROW){ // Câmara no modo seta
        // Cálculos para centrar as coordenadas ao centro da câmara
        int newXOrigin = result.xOrigin - (imgWidth / 2);
        int newYOrigin = result.yOrigin - (imgHeight / 2);
        int newXTarget = result.xTarget - (imgWidth / 2);
        int newYTarget = result.yTarget - (imgHeight / 2);
        Serial.println(String()+F("Arrow:xOrigin=")+newXOrigin+F(",yOrigin=")+newYOrigin+F(",xTarget=")+newXTarget+F(",yTarget=")+newYTarget+F(",ID=")+result.ID);

        doMovement(newXTarget, newYTarget);
        recovering = false;
    }else{ // Sem nenhum modo ou objecto aprendido
        Serial.println("Object unknown!");
    }
}

// função para movimentar o carro
void doMovement(int x, int y){
    bool xOffCenter = abs(x) > deadZoneX;
    bool yTooCloseOrFar = abs(y) > deadZoneY;

    if (xOffCenter && yTooCloseOrFar) { // se o objecto estiver desviado do centro nas coordenadas x e y ajusta o movimento (tem em atenção uma margem de segurança "deadZoneX" e "deadZoneY")
        if(x > 0){
            turnRight();
        }else{
            turnLeft();
        }
        
        if(y > 0){
            goBack();
        }else{
            goFront();
        }
    }else if (xOffCenter) { // se o objecto estiver desviado do centro nas coordenadas x ajusta o movimento (tem em atenção uma margem de segurança "deadZoneX")
        if (x > 0){
            turnRight();
        }else{
            turnLeft();
        }

    }else if (yTooCloseOrFar) { // se o objecto estiver desviado do centro nas coordenadas y ajusta o movimento (tem em atenção uma margem de segurança "deadZoneY")
        if (y > 0){
            goBack();
        }else{
            goFront();
        }
    }else { // Objecto está ao centro
        stop();
        Serial.println("Aligned and at the ideal distance. Stopping.");
    }
}

// função para regressar para identificar quando um objecto saiu do ecrã, ajusta o movimento para voltar atrás e recuperar a posição do ecrã
// necessário devido à limitação dos motores usados, não é possível limitar mais a velocidade do motor, que por vezes ainda é demasiado rápido
void recoverLostTarget() {
    if (!recovering) {
        recovering = true;
        Serial.println("Iniciar varrimento para recuperação.");
        Serial.println(last_move);
    }

    if (last_move == 1) {
        turnRight();
    }else if (last_move == 2) {
        turnRight();
    }else if (last_move == 3) {
        goBack();
    }else if (last_move == 4) {
        goFront();
    } else {
        turnRight();
        recovering = false;
    }
}

// função para movimentar o servo motor - Não usada no momento
void doMovementServo(int x, int y){
    int angulo = map(x, -160, 160, 170, 0); // Mapeia x para o ângulo do servo
    servoMotor.write(angulo);
    delay(1000);
}

// Função para movimentar o carro para a esquerda
void turnLeft(){
    digitalWrite(M1, HIGH);  
    digitalWrite(M2, HIGH);      
    analogWrite(E1, 0);         // PWM regula a velocidade
    analogWrite(E2, 110);       // PWM regula a velocidade
    if (!recovering) {
        last_move = 1;
    }
}

// Função para movimentar o carro para a direita
void turnRight(){
    digitalWrite(M1, HIGH);  
    digitalWrite(M2, HIGH);      
    analogWrite(E1, 120);       // PWM regula a velocidade
    analogWrite(E2, 0);         // PWM regula a velocidade
    if (!recovering) {
        last_move = 2;
    }
}

// Função para movimentar o carro para a frente
void goFront(){
    digitalWrite(M1, HIGH);  
    digitalWrite(M2, HIGH);      
    analogWrite(E1, 120);         // PWM regula a velocidade
    analogWrite(E2, 110);       // PWM regula a velocidade
    if (!recovering) {
        last_move = 3;
    }
}

// Função para movimentar o carro para trás
void goBack(){
    digitalWrite(M1, LOW);  
    digitalWrite(M2, LOW);      
    analogWrite(E1, 120);       // PWM regula a velocidade
    analogWrite(E2, 110);         // PWM regula a velocidade
    if (!recovering) {
        last_move = 4;
    }
}

// Função para parar o carro
void stop(){
    digitalWrite(M1, LOW);  
    digitalWrite(M2, LOW);      
    analogWrite(E1, 0);         // PWM regula a velocidade
    analogWrite(E2, 0);       // PWM regula a velocidade
}