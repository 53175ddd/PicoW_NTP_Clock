/* 時刻表示デモ用 */

#define HC595_DATA  10
#define HC595_CLOCK 11
#define HC595_LATCH 12

typedef struct {
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
} local_time_t;

const uint8_t font[10] = {0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110, 0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11110110};

/* 7 セグメント LED 制御用の関数 */
void drive595(uint8_t datum) {
  for(size_t i = 0; i < 8; i++) {
    digitalWrite(HC595_DATA, (datum >> i) & 1);
    digitalWrite(HC595_CLOCK, HIGH);
    digitalWrite(HC595_CLOCK,  LOW);
  }

  digitalWrite(HC595_DATA, LOW);
}

void latch595(void) {
  digitalWrite(HC595_LATCH, HIGH);
  digitalWrite(HC595_LATCH,  LOW);
}

/* 時刻表示用の関数 */
void display_time(const local_time_t time) {
  drive595(font[time.sec%10]);
  drive595(font[time.sec/10]);
  drive595(font[time.min%10]);
  drive595(font[time.min/10]);
  drive595(font[time.hour%10]);
  drive595(font[time.hour/10]);
  latch595();
}

local_time_t count_up(local_time_t res) {
  uint32_t tmp = res.hour * 3600 + res.min * 60 + res.sec + 1;

  char buffer[32];
  sprintf(buffer, "\n\nold %02d-%02d-%02d", res.hour, res.min, res.sec);
  Serial.print(buffer);

  res.hour = (tmp / 3600) % 60;
  res.min  = (tmp % 3600) / 60;
  res.sec  = tmp % 60;

  sprintf(buffer, "\nnew %02d-%02d-%02d", res.hour, res.min, res.sec);
  Serial.print(buffer);

  return res;
}

void setup() {
  Serial.begin(115200);
  for(int8_t i = 0; i < 100 & !Serial; i++) delay(10);

  pinMode(HC595_DATA , OUTPUT);  digitalWrite(HC595_DATA , LOW);
  pinMode(HC595_CLOCK, OUTPUT);  digitalWrite(HC595_CLOCK, LOW);
  pinMode(HC595_LATCH, OUTPUT);  digitalWrite(HC595_LATCH, LOW);
}

void loop() {
  static local_time_t now = {12, 34, 56};

  display_time(now);
  now = count_up(now);

  delay(1000);
}
