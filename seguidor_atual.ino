  #include <QTRSensors.h>
//Definição dos motores 

#define INT1 A1
#define INT2 A2
#define INT3 12
#define INT4 13
#define ENA 3
#define ENB 11

//Definição dos sensores

QTRSensors qtr;

const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

//Sensores de 90 graus 
int detector_90_direita = A3;
int detector_90_esquerda = A4;

//Definições do pid

float Kp = 55; // Ganho proporcional
float Ki = 0.05; // Ganho integral
float Kd = 0.3; // Ganho derivativo

float integral = 0; // Valor da integral
float prop = 0; // Valor proporcional
float derivativo = 0; // Valor do derivativo
float PID = 0; // Valor do controle PID
int U_erro = 0; // Último valor de erro

//Definições das velocidades inicias
int velocidadeInicial = 70;
float avgSpeedESQ = velocidadeInicial; // Velocidade média do motor esquerdo
float avgSpeedDIR = velocidadeInicial; // Velocidade média do motor direito
int incremento = 5;

void setup() {

  Serial.begin(9600);
  configurarSensor();
  pinMode(INT1, OUTPUT);
  pinMode(INT2, OUTPUT);
  pinMode(INT3, OUTPUT);
  pinMode(INT4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(detector_90_direita,INPUT);
  pinMode(detector_90_esquerda, INPUT);

  

}

void loop() {

  
  uint16_t position = qtr.readLineBlack(sensorValues);
  float leituraMapeada = mapearValores(position);
  bool leitura_90_direita = ler_90_direita();
  bool leitura_90_esquerda = ler_90_esquerda();
  Serial.println(leituraMapeada);
  testar_90_graus(leitura_90_direita , leitura_90_esquerda);
  if (leituraMapeada == 0) {
    setVelocidade(velocidadeInicial); // Define uma velocidade inicial
    frente(); // Move para frente
  } else {
    // Calcula o PID e ajusta as velocidades dos motores
    float pidOutput = calculaPID(leituraMapeada);
    ajustarVelocidadePID(pidOutput);
 }


    // ajustarTrajetoria(leituraMapeada);
    // frente();
 }


//Funções do sensor 

void configurarSensor(){
    // configure the sensors
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){2, 4, 5, 6, 7, 8, 9, 10}, SensorCount);
  qtr.setEmitterPin(A0);

  delay(500);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // turn on Arduino's LED to indicate we are in calibration mode

  // 2.5 ms RC read timeout (default) * 10 reads per calibrate() call
  // = ~25 ms per calibrate() call.
  // Call calibrate() 400 times to make calibration take about 10 seconds.
  for (uint16_t i = 0; i < 400; i++)
  {
    qtr.calibrate();
  }
  digitalWrite(LED_BUILTIN, LOW); // turn off Arduino's LED to indicate we are through with calibration

  // print the calibration minimum values measured when emitters were on
  Serial.begin(9600);
  for (uint8_t i = 0; i < SensorCount; i++)
  {
    Serial.print(qtr.calibrationOn.minimum[i]);
    Serial.print(' ');
  }
  Serial.println();

  // print the calibration maximum values measured when emitters were on
  for (uint8_t i = 0; i < SensorCount; i++)
  {
    Serial.print(qtr.calibrationOn.maximum[i]);
    Serial.print(' ');
  }
  Serial.println();
  Serial.println();
  delay(1000);
}

float mapearValores(int leitura) {
  const int minSensorValue = 1000; // Valor mínimo do sensor
  const int maxSensorValue = 6000; // Valor máximo do sensor
  const int mappedMinValue = -8; // Valor mapeado correspondente ao mínimo
  const int mappedMaxValue = 8; // Valor mapeado correspondente ao máximo
  const int zeroSensorValue = 3500; // Valor do sensor correspondente a zero

  // Mapeia o valor do sensor para o intervalo desejado
  float mappedValue = map(leitura, minSensorValue, maxSensorValue, mappedMinValue, mappedMaxValue);

  // Subtrai o valor de zeroSensorValue para fazer 3500 corresponder a 0
  mappedValue -= map(zeroSensorValue, minSensorValue, maxSensorValue, mappedMinValue, mappedMaxValue);

  return mappedValue;
}

//Funções de movimentação 

void ligar(int pino){
  digitalWrite(pino,HIGH);
}

void desligar(int pino){
  digitalWrite(pino,LOW);
}

void frente_motorA(){
  ligar(INT2);
  desligar(INT1);
}

void frente_motorB(){
    ligar(INT3);
    desligar(INT4);
}

void frente(){
  frente_motorA();
  frente_motorB();
}

void tras_motorA(){
  ligar(INT1);
  desligar(INT2);
  
}
void tras_motorB(){
  ligar(INT4);
  desligar(INT3);
}

void tras(){
  tras_motorA();
  tras_motorB();
}

void parar_motorA(){
  velocidade_motorA(0);
}
void parar_motorB(){
  velocidade_motorB(0);
}
void parar(){
  parar_motorA();
  parar_motorB();
}
void girar_eixo_direita(){
  frente_motorA();
  tras_motorB();
}
void girar_eixo_esquerda(){
  frente_motorB();
  tras_motorA();
}
void velocidade_motorA(int velocidade){
  analogWrite(ENA,velocidade);
}
void velocidade_motorB(int velocidade){
  analogWrite(ENB,velocidade);
}
void setVelocidade(int velocidade){
  velocidade_motorA(velocidade);
  velocidade_motorB(velocidade);
}
void virar_direita(){
 frente_motorA();
 parar_motorB();
 delay(1000);
 parar();
}
void virar_esquerda(){
 frente_motorB();
 parar_motorA();
 delay(1000);
 parar();
}
void testar_90_graus(bool leituraDireita, bool leituraEsqueda){
  
  if(leituraDireita){
    setVelocidade(velocidadeInicial-20);
    delay(20);
    frente();
    girar_eixo_direita();
    delay(20);
    delay(335);
    parar();
  }
  if(leituraEsqueda){
    setVelocidade(velocidadeInicial-20);
    delay(20);
    frente();
    delay(20);
    girar_eixo_esquerda();
    delay(335);
    parar();
  }
}
float calculaPID(int erro){
  if(erro==0){
    integral = 0;
  }
  
  prop = erro;
  integral = integral + erro;
  if(integral > 255){
    integral = 255;
  }
  else if(integral < -255){
    integral = -255;
  }
  derivativo = erro - U_erro;
  PID = ((Kp * prop) + (Ki * integral) + (Kd * derivativo));
  U_erro = erro;
  return PID;
}

void ajustarVelocidadePID(float PID) {
  float velesq, veldir; // Velocidades dos motores esquerdo e direito

  if (PID >= 0) { // Vira para a direita
    velesq = avgSpeedESQ;
    veldir = avgSpeedDIR - PID;
    
  } else { // Vira para a esquerda
    
    velesq = avgSpeedESQ + abs(PID);
    veldir = avgSpeedDIR;
  }

  // Define as velocidades dos motores

  // Serial.print(velesq);
  // Serial.print("  ");
  // Serial.print(veldir);
  velocidade_motorA(velesq);
  velocidade_motorB(veldir);
}
void ajustarTrajetoria(float leitura){
  int ajuste = abs(leitura) * incremento;
  if(leitura == 0){
    // Se a leitura é zero, mantém as velocidades dos motores iguais para seguir reto
    setVelocidade(velocidadeInicial);
  }
  else if(leitura < 0){
    // Se a leitura é negativa, o carrinho deve corrigir para a direita
    // Isso significa aumentar a velocidade do motor esquerdo e manter ou reduzir a do motor direito
    velocidade_motorA(velocidadeInicial + ajuste); // Aumenta a velocidade do motor esquerdo
    velocidade_motorB(max(velocidadeInicial - ajuste, 0)); // Reduz a velocidade do motor direito, mas não permite valores negativos
  }
  else {
    // Se a leitura é positiva, o carrinho deve corrigir para a esquerda
    // Isso significa manter ou reduzir a velocidade do motor esquerdo e aumentar a do motor direito
    velocidade_motorA(max(velocidadeInicial - ajuste, 0)); // Reduz a velocidade do motor esquerdo, mas não permite valores negativos
    velocidade_motorB(velocidadeInicial + ajuste); // Aumenta a velocidade do motor direito
  }
}
bool ler_90_direita(){
  return digitalRead(detector_90_direita);
}
bool ler_90_esquerda(){
  return digitalRead(detector_90_esquerda);
}