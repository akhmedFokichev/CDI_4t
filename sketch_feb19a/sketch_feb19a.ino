const int hallSensor = 2;  
const int ignitionPin = 9;
const int logTimeDelay = 1000;
const int com3Speed = 9600;

// Указываем угол установки датчика (например, 40° до ВМТ)
const int startAngle = 40;  

volatile unsigned long lastPulseTime = 0;
volatile unsigned long rpm = 0;
volatile unsigned long delayIgnition = 0;
volatile unsigned long lastInterruptTime = 0;

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

// 🔹 Прерывание по сигналу датчика Холла
void fireIgnition() {
  unsigned long currentTime = micros();

  // Анти-дребезг (игнорируем слишком частые срабатывания < 250 мкс)
  if (currentTime - lastInterruptTime < 250) return;
  lastInterruptTime = currentTime;

  // Вычисляем RPM
  calcRPM();

  // Получаем угол опережения
  int ignitionAdvance = getAdvanceAngle(rpm);
  ignitionAdvance = constrain(ignitionAdvance, 0, startAngle);  // Ограничиваем УОЗ

  // Рассчитываем задержку перед искрой
  delayIgnition = calcDelayIgnition(ignitionAdvance);

  // Ожидаем нужное время перед искрой
  delayMicroseconds(delayIgnition); 

  // Вызываем искру
  spark();
}

// 🔹 Управление искрой
void spark() {
  digitalWrite(ignitionPin, HIGH);
  delayMicroseconds(2000);  // Длительность искры (можно настраивать)
  digitalWrite(ignitionPin, LOW);
}

// 🔹 Рассчитываем RPM
void calcRPM() {
  unsigned long currentTime = micros();
  unsigned long pulseInterval = currentTime - lastPulseTime;

  if (pulseInterval > 1000) {  // Защита от деления на 0
    rpm = 60000000 / pulseInterval;
  }
  lastPulseTime = currentTime;
}

// 🔹 Рассчитываем задержку перед искрой с учётом угла датчика
unsigned long calcDelayIgnition(int ignitionAdvance) {
  int effectiveAdvance = startAngle - ignitionAdvance;
  if (effectiveAdvance < 0) effectiveAdvance = 0;  // Искра не может быть позже ВМТ

  if (rpm > 0) {
    return (effectiveAdvance * 1000000UL) / (rpm * 6);  // Перевод градусов в микросекунды
  }
  return 0;  // Если двигатель стоит, задержки нет
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
  
  return 38; // Максимальный угол
}
