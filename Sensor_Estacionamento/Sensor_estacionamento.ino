// Pinos do Sensor
const int trigPin = 12;
const int echoPin = 11;

// Pinos dos LEDs (Ordem: Verde, Verde, Amarelo, Amarelo, Vermelho, Vermelho)
// Podes mudar os números se ligares noutros pinos
const int ledPins[] = {2, 3, 4, 5, 6, 7}; 
const int numLeds = 6;

long duration;
int distance;

void setup() {
  // Iniciar Monitor Série (para veres a distância no PC se precisares)
  Serial.begin(9600);

  // Configurar pinos do Sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Configurar pinos dos LEDs com um ciclo "for" (para não escrever 6 linhas iguais)
  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
}

void loop() {
  // 1. Calcular a distância
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  // Mostra a distância no computador (opcional)
  Serial.print("Distancia: ");
  Serial.print(distance);
  Serial.println(" cm");

  // 2. Controlar os LEDs
  // Primeiro, apagamos todos para garantir que não ficam luzes "presas"
  for (int i = 0; i < numLeds; i++) {
    digitalWrite(ledPins[i], LOW);
  }

  // Agora acendemos dependendo da distância
  // A lógica é: se a distância for MENOR que X, acende o LED correspondente
  
  if (distance < 60) {
    digitalWrite(ledPins[0], HIGH); // Verde 1 (35cm)
  }
  if (distance < 50) {
    digitalWrite(ledPins[1], HIGH); // Verde 2 (30cm)
  }
  if (distance < 40) {
    digitalWrite(ledPins[2], HIGH); // Amarelo 1 (25cm)
  }
  if (distance < 30) {
    digitalWrite(ledPins[3], HIGH); // Amarelo 2 (20cm)
  }
  if (distance < 20) {
    digitalWrite(ledPins[4], HIGH); // Vermelho 1 (15cm)
  }
  if (distance < 10) {
    digitalWrite(ledPins[5], HIGH); // Vermelho 2 (10cm - PERIGO!)
  }
  
  // Se estiver MUITO perto (colisão iminente < 5cm), fazemos os vermelhos piscar
  if (distance < 5 && distance > 0) {
     digitalWrite(ledPins[4], LOW);
     digitalWrite(ledPins[5], LOW);
     delay(100);
     digitalWrite(ledPins[4], HIGH);
     digitalWrite(ledPins[5], HIGH);
     delay(100);
  } else {
     delay(100); // Pequena pausa normal
  }
}