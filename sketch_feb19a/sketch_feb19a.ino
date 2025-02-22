const int hallSensor = 2;  
const int ignitionPin = 9;
const int logTimeDelay = 1000;
const int com3Speed = 9600;

volatile unsigned long lastPulseTime = 0;
volatile unsigned long rpm = 0;
volatile unsigned long delayIgnition = 0;

// 🔹 Указываем угол установки датчика (можно менять!)
const int startAngle = 40;  // Например, датчик стоит на 40° до ВМТ

void setup() {
  pinMode(hallSensor, INPUT_PULLUP);
  pinMode(ignitionPin, OUTPUT);
  Serial.begin(com3Speed);

  attachInterrupt(digitalPinToInterrupt(hallSensor), fireIgnition, FALLING);
}

void loop() {
  static unsigned long lastPrintTime = 0;

  if (millis() - lastPrintTime >= logTimeDelay) {
    lastPrintTime = millis();
    Serial.print("RPM: ");
    Serial.print(rpm);
    Serial.print(" | УОЗ: ");
    Serial.print(getAdvanceAngle(rpm));
    Serial.print(" | Задержка: ");
    Serial.println(delayIgnition);
  }
}

void spark_old() {
  // Подаём искру
  digitalWrite(ignitionPin, HIGH);
  delay(100);
  digitalWrite(ignitionPin, LOW);
}

void fireIgnition() {
  // Вычисляем RPM
  calcRPM();

  // Получаем угол опережения в зависимости от RPM
  int ignitionAdvance = getAdvanceAngle(rpm);

  // Получаем delay дял искры
  delayIgnition = calcDelayIgnition(ignitionAdvance);

  // Ждём нужное время перед искрой
  delayMicroseconds(delayIgnition); 

  // искра
  spark();
}

void spark() {
  digitalWrite(ignitionPin, HIGH);
  delayMicroseconds(5000);
  digitalWrite(ignitionPin, LOW);
}

  // 🔹 Рассчитываем RPM
void calcRPM() {
  unsigned long currentTime = micros();
  unsigned long pulseInterval = currentTime - lastPulseTime;

  if (pulseInterval > 0) {
    rpm = 60000000 / pulseInterval;  // Вычисляем RPM
  }
  lastPulseTime = currentTime;
}

  // 🔹 Рассчитываем задержку перед искрой с учётом угла датчика
unsigned long calcDelayIgnition(int ignitionAdvance) {
  int effectiveAdvance = startAngle - ignitionAdvance;  
  if (effectiveAdvance < 0) effectiveAdvance = 0; // Искра не может быть позже ВМТ
  delayIgnition = (effectiveAdvance * 1000000) / (rpm * 6); // Перевод градусов в микросекунды
  return delayIgnition;
}

// 🔹 Функция вычисления угла опережения зажигания (подбираем под мотор)
int getAdvanceAngle(unsigned long rpm) {
  if (rpm < 1000) return map(rpm, 0, 1000, 5, 10);
  if (rpm < 2000) return map(rpm, 1000, 2000, 10, 15);
  if (rpm < 3000) return map(rpm, 2000, 3000, 15, 20);
  if (rpm < 4000) return map(rpm, 3000, 4000, 20, 25);
  if (rpm < 5000) return map(rpm, 4000, 5000, 25, 27);
  if (rpm < 6000) return map(rpm, 5000, 6000, 27, 30);
  if (rpm < 7000) return map(rpm, 6000, 7000, 30, 33);
  if (rpm < 8000) return map(rpm, 7000, 8000, 33, 35);
  if (rpm < 9000) return map(rpm, 8000, 9000, 35, 38);
  
  return 38; // Максимальное значение
}
