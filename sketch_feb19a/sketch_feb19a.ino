const int hallSensor = 2;  // Датчик Холла
const int ignitionPin = 9;  // Катушка зажигания

volatile unsigned long lastPulseTime = 0;
volatile unsigned long rpm = 0;
volatile unsigned long delayIgnition = 0;

void setup() {
  pinMode(hallSensor, INPUT_PULLUP);
  pinMode(ignitionPin, OUTPUT);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(hallSensor), fireIgnition, FALLING);
}

void loop() {
  static unsigned long lastPrintTime = 0;

  if (millis() - lastPrintTime >= 1000) {
    lastPrintTime = millis();
    Serial.print("RPM: ");
    Serial.print(rpm);
    Serial.print(" | Delay: ");
    Serial.println(delayIgnition);
  }
}

void fireIgnition() {
  unsigned long currentTime = micros();
  unsigned long pulseInterval = currentTime - lastPulseTime;

  if (pulseInterval > 0) {
    rpm = 60000000 / pulseInterval;  // Рассчитываем RPM
  }
  
  lastPulseTime = currentTime;

  // Определяем угол опережения зажигания (примерная таблица)
  int ignitionAdvance = 10; // По умолчанию 10 градусов
  if (rpm > 1000) ignitionAdvance = 15;
  if (rpm > 2000) ignitionAdvance = 20;
  if (rpm > 3000) ignitionAdvance = 25;
  if (rpm > 4000) ignitionAdvance = 30; // Максимальное опережение 30°

  // Рассчитываем задержку перед зажиганием
  delayIgnition = (ignitionAdvance * 1000000) / (rpm * 6);

  delayMicroseconds(delayIgnition); // Задержка перед искрой

  // Подаём искру
  digitalWrite(ignitionPin, HIGH);
  delayMicroseconds(5000); // Время зарядки катушки
  digitalWrite(ignitionPin, LOW);
}