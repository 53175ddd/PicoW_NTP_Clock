/* RTC デモ用 */

#include <Wire.h>

#define HC595_DATA  10
#define HC595_CLOCK 11
#define HC595_LATCH 12

#define I2C_SDA 0
#define I2C_SCL 1

#define RTC_I2C_ADDR 0x68
#define RTC_SQW         2

typedef struct {
  uint8_t week;
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
  drive595(0b01000000 >> (time.week % 7));
  drive595(font[time.sec & 0b00001111]);
  drive595(font[time.sec >> 4]);
  drive595(font[time.min & 0b00001111]);
  drive595(font[time.min >> 4]);
  drive595(font[time.hour & 0b00001111]);
  drive595(font[time.hour >> 4]);
  latch595();
}

local_time_t read_rtc(void) {
  local_time_t res;

  Wire.beginTransmission(RTC_I2C_ADDR);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(RTC_I2C_ADDR, 4);

  res.sec  = Wire.read();
  res.min  = Wire.read();
  res.hour = Wire.read();
  res.week = Wire.read();

  return res;
}

uint8_t dec2bcd(uint8_t datum) {
  return ((datum / 10) << 4) | (datum % 10);
}

void setup() {
  Serial.begin(115200);
  for(int8_t i = 0; i < 100 & !Serial; i++) delay(10);

  pinMode(RTC_SQW, INPUT);
  pinMode(HC595_DATA , OUTPUT);  digitalWrite(HC595_DATA , LOW);
  pinMode(HC595_CLOCK, OUTPUT);  digitalWrite(HC595_CLOCK, LOW);
  pinMode(HC595_LATCH, OUTPUT);  digitalWrite(HC595_LATCH, LOW);

  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin();
  
  Wire.beginTransmission(RTC_I2C_ADDR);
  Wire.write(0x00);
  Wire.write(dec2bcd(56));
  Wire.write(dec2bcd(34));
  Wire.write(dec2bcd(12));
  Wire.write(1);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0b00010000);
  Wire.endTransmission();
}

void loop() {
  static uint8_t sqw_status = 0;

  sqw_status = (sqw_status << 1) + digitalRead(RTC_SQW);

  if((sqw_status & 0b00000011) == 0b01) {
    local_time_t now = read_rtc();
    display_time(now);
  }

  delay(10);
}
