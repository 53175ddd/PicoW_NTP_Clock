#include <WiFi.h>
#include <TimeLib.h>
#include <Wire.h>

#define HC595_DATA  10
#define HC595_CLOCK 11
#define HC595_LATCH 12

#define I2C_SDA 0
#define I2C_SCL 1

#define RTC_I2C_ADDR 0x68
#define RTC_SQW         2

#define CHECKSUM 0b10101010

#define DEBUG_EN        false
#define FORCE_RTC_RESET false

typedef struct {
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint8_t week;
} rtc_calender_t;

const uint8_t font[10] = {0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110, 0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11110110};

rtc_calender_t reset_time_info = {0};

// adapt to your network
const char* WLAN_SSID = "RuckusIoT";
const char* WLAN_PSK  = "ruckusiot";

const char* NTP_SERVER = "ntp.nict.jp";
const int8_t GMT = 9;

rtc_calender_t get_time(const char* ntp, const int8_t gmt) {
  NTP.begin(ntp);
  NTP.waitSet();

  time_t now = time(nullptr) + (gmt * 3600);

  struct tm time_info;
  gmtime_r(&now, &time_info);

  rtc_calender_t local_time;
  local_time.hour = time_info.tm_hour;
  local_time.min  = time_info.tm_min;
  local_time.sec  = time_info.tm_sec;
  local_time.week = time_info.tm_wday;

  if(DEBUG_EN) {
    char buffer[32];
    sprintf(buffer, "%d-%d-%d %d\n", local_time.hour, local_time.min, local_time.sec, local_time.week);
    Serial.print(buffer);
  }

  return local_time;
}

void init_rtc(rtc_calender_t now) {
  Wire.beginTransmission(RTC_I2C_ADDR);
  Wire.write(0);
  Wire.write(dec2bcd(now.sec));
  Wire.write(dec2bcd(now.min));
  Wire.write(dec2bcd(now.hour));
  Wire.write(now.week);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0b10010000);
  Wire.write(CHECKSUM);
  Wire.endTransmission();
}

void update_rtc(rtc_calender_t now) {
  Wire.beginTransmission(RTC_I2C_ADDR);
  Wire.write(0x00);
  Wire.write(dec2bcd(now.sec));
  Wire.write(dec2bcd(now.min));
  Wire.write(dec2bcd(now.hour));
  Wire.write(now.week);
  Wire.endTransmission();
}

rtc_calender_t read_rtc(void) {
  rtc_calender_t res;

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

/* 時刻表示用の関数 */
void display_time(const rtc_calender_t time) {
  drive595(0b01000000 >> (time.week % 7));
  drive595(font[time.sec & 0b00001111]);
  drive595(font[time.sec >> 4]);
  drive595(font[time.min & 0b00001111]);
  drive595(font[time.min >> 4]);
  drive595(font[time.hour & 0b00001111]);
  drive595(font[time.hour >> 4]);
  latch595();

  if(DEBUG_EN) {
    char buffer[32];
    sprintf(buffer, "%h-%h-%h\n", time.hour, time.min, time.sec);
    Serial.print(buffer);
  }
}

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

uint8_t dec2bcd(uint8_t datum) {
  return ((datum / 10) << 4) | (datum % 10);
}

void connecting_animation(void) {
  static uint8_t count = 0;

  switch(count) {
    case 0:
      if(DEBUG_EN) {
        Serial.print(".");
      }
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000010);
      break;

    case 1:
    case 9:
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000010);
      drive595(0b00000000);
      break;

    case 2:
    case 8:
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000010);
      drive595(0b00000000);
      drive595(0b00000000);
      break;

    case 3:
    case 7:
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000010);
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000000);
      break;

    case 4:
    case 6:
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000010);
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000000);
      break;

    case 5:
      if(DEBUG_EN) {
        Serial.print(".");
      }
      drive595(0b00000000);
      drive595(0b00000010);
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000000);
      drive595(0b00000000);
      break;
  }
  latch595();

  count = (count + 1) % 10;
}

void setup() {
  pinMode(RTC_SQW, INPUT);
  pinMode(HC595_DATA , OUTPUT); digitalWrite(HC595_DATA , LOW);
  pinMode(HC595_CLOCK, OUTPUT); digitalWrite(HC595_CLOCK, LOW);
  pinMode(HC595_LATCH, OUTPUT); digitalWrite(HC595_LATCH, LOW);

  drive595(0b01111111);
  drive595(0b01000001);  // !
  drive595(0b11111100);  // O
  drive595(0b00011100);  // L
  drive595(0b00011100);  // L
  drive595(0b10011110);  // E
  drive595(0b01101110);  // H
  latch595();

  reset_time_info.hour =  3;
  reset_time_info.min  =  0;
  reset_time_info.sec  =  0;

  if(DEBUG_EN) {
    Serial.begin(115200); 
    for(size_t i = 0; i < 500 && !Serial; i++) delay(10);
  }

  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin();

  Wire.beginTransmission(RTC_I2C_ADDR);
  Wire.write(0x08);
  Wire.endTransmission();
  Wire.requestFrom(RTC_I2C_ADDR, 1);
  uint8_t checksum = Wire.read();

  if(DEBUG_EN) {
    Serial.print("\nCHECKSUM = ");
    Serial.print(CHECKSUM, HEX);
    Serial.print("\nchecksum = ");
    Serial.print(checksum, HEX);
    Serial.print("\n\n");
  }

  if((checksum != CHECKSUM) || FORCE_RTC_RESET) {
    if(DEBUG_EN) {
      Serial.print("Trying to connect to "); 
      Serial.println(WLAN_SSID); 
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(WLAN_SSID, WLAN_PSK);
    Serial.print("WiFi.begin done.\n");
    while (WiFi.status() != WL_CONNECTED) {
      connecting_animation();
      delay(100);
    } 

    if(DEBUG_EN) {
      Serial.print("\n\nWiFi connected!\n"); 
      Serial.print("IP address: "); 
      Serial.println(WiFi.localIP());
    }

    // 時刻取得関数
    if(DEBUG_EN) {
      Serial.print("Trying to connect to NTP Server\n");
    }
    init_rtc(get_time(NTP_SERVER, GMT));

    WiFi.disconnect();
  }
} 

void loop() {
  static uint8_t sqw_status = 0;

  sqw_status = (sqw_status << 1) + digitalRead(RTC_SQW);

  if((sqw_status & 0b00000011) == 0b01) {
    rtc_calender_t now = read_rtc();
    display_time(now);

    if((now.hour == reset_time_info.hour) && (now.min == reset_time_info.min) && (now.sec == reset_time_info.sec)) {
      if(DEBUG_EN) {
        Serial.print("Trying to connect to "); 
        Serial.println(WLAN_SSID); 
      }

      WiFi.mode(WIFI_STA);
      WiFi.begin(WLAN_SSID, WLAN_PSK);
      if(DEBUG_EN) {
        Serial.print("WiFi.begin done.\n");
      }

      while (WiFi.status() != WL_CONNECTED) {
        connecting_animation();
        delay(100);
      } 

      if(DEBUG_EN) {
        Serial.print("\n\nWiFi connected!\n"); 
        Serial.print("IP address: "); 
        Serial.println(WiFi.localIP());
      }

      // 時刻取得関数
      if(DEBUG_EN) {
        Serial.print("Trying to connect to NTP Server\n");
      }

      rtc_calender_t now = get_time(NTP_SERVER, GMT);
      init_rtc(now);
     
      WiFi.disconnect();
    }
  }

  delay(5);
}
