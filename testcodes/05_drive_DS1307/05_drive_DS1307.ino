#include <Wire.h>

#define RTC_I2C_ADDR 0x68
#define I2C_SDA         0
#define I2C_SCL         1

#define BAUDRATE 115200

#define TEST_SIZE 20

/*
uint8_t dec2bcd(uint8_t datum) {
  uint8_t digit_10 = 0, digit_1 = 0, res = 0;

  digit_10 = datum / 10;
  digit_1  = datum % 10;

  res = (digit_10 << 4) | digit_1;

  return res;
}
*/

uint8_t dec2bcd(uint8_t datum) {
  return ((datum / 10) << 4) | (datum % 10);
}

void setup() {
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin();

  Serial.begin(BAUDRATE);
  for(size_t i = 0; i < 100 & !Serial; i++) delay(10);

  Serial.print("test start.\n\n");

  uint8_t test_data[TEST_SIZE] = {0};

  for(size_t i = 0; i < TEST_SIZE; i++) {
    test_data[i] = random(0, 256);
  }

  // write and read test routine
  Wire.beginTransmission(RTC_I2C_ADDR);
  Wire.write(0x10);  // start address
  for(size_t i = 0; i < TEST_SIZE; i++) {
    Wire.write(test_data[i]);
  }
  Wire.endTransmission();

  Wire.beginTransmission(RTC_I2C_ADDR);
  Wire.write(0x10);  // start address
  Wire.endTransmission();

  uint8_t read_buffer[TEST_SIZE] = {0};

  Wire.requestFrom(RTC_I2C_ADDR, TEST_SIZE);
  for(size_t i = 0; i < TEST_SIZE; i++) {
    read_buffer[i] = Wire.read();
  }

  for(size_t i = 0; i < TEST_SIZE; i++) {
    char buffer[48];
    uint8_t w = test_data[i];
    uint8_t r = read_buffer[i];

    sprintf(buffer, "[%2d of %2d] wrote data = 0x%0x, read data = 0x%0x, %s\n", i + 1, TEST_SIZE, w, r, (w == r) ? "OK!" : "Failed...");
    Serial.print(buffer);
  }

  Serial.print("\ntest end\n");
}

void loop() {
}
