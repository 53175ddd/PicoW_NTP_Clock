/* 全点灯と全消灯を繰り返します */

#define HC595_DATA  10
#define HC595_CLOCK 11
#define HC595_LATCH 12

void setup() {
  pinMode(HC595_DATA , OUTPUT);
  pinMode(HC595_CLOCK, OUTPUT);
  pinMode(HC595_LATCH, OUTPUT);
}

void loop() {
  // 全点灯
  digitalWrite(HC595_DATA , HIGH);
  for(size_t i = 0; i < 8; i++) {
    digitalWrite(HC595_CLOCK, HIGH);
    digitalWrite(HC595_CLOCK,  LOW);
  }
  digitalWrite(HC595_LATCH, HIGH);
  digitalWrite(HC595_LATCH,  LOW);

  delay(1000);

  // 全消灯
  digitalWrite(HC595_DATA ,  LOW);
  for(size_t i = 0; i < 8; i++) {
    digitalWrite(HC595_CLOCK, HIGH);
    digitalWrite(HC595_CLOCK,  LOW);
  }
  digitalWrite(HC595_LATCH, HIGH);
  digitalWrite(HC595_LATCH,  LOW);

  delay(1000);
}
