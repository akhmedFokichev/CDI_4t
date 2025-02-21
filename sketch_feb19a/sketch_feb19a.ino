const int hallSensor = 2;  // Датчик Холла
const int ignitionPin = 9;  // Катушка зажигания

volatile unsigned long lastPulseTime = 0;
volatile unsigned long rpm = 0;
volatile unsigned long delayIgnition = 0;

// 🔹 Указываем угол установки датчика (можно менять!)
const int startAngle = 40;  // Например, датчик стоит на 40° до ВМТ

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
    Serial.print(" | УОЗ: ");
    Serial.print(getAdvanceAngle(rpm));
    Serial.print(" | Задержка: ");
    Serial.println(delayIgnition);
  }
}

// 🔹 Функция прерывания (приходит сигнал от датчика Холла)
void fireIgnition() {
  unsigned long currentTime = micros();
  unsigned long pulseInterval = currentTime - lastPulseTime;

  if (pulseInterval > 0) {
    rpm = 60000000 / pulseInterval;  // Вычисляем RPM
  }

  lastPulseTime = currentTime;

  // Получаем угол опережения в зависимости от RPM
  int ignitionAdvance = getAdvanceAngle(rpm);

  // 🔹 Рассчитываем задержку перед искрой с учётом угла датчика
  int effectiveAdvance = startAngle - ignitionAdvance;  
  if (effectiveAdvance < 0) effectiveAdvance = 0; // Искра не может быть позже ВМТ

  delayIgnition = (effectiveAdvance * 1000000) / (rpm * 6); // Перевод градусов в микросекунды

  delayMicroseconds(delayIgnition); // Ждём нужное время перед искрой

  // Подаём искру
  digitalWrite(ignitionPin, HIGH);
  delayMicroseconds(5000); // Время зарядки катушки
  digitalWrite(ignitionPin, LOW);
}

// 🔹 Функция вычисления угла опережения зажигания (подбираем под мотор)
int getAdvanceAngle(unsigned long rpm) {
  if (rpm < 1000) return 10;
  if (rpm < 2000) return 15;
  if (rpm < 3000) return 20;
  if (rpm < 4000) return 25;
  return 30; // Максимальный угол
}
