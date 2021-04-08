#include <avr/eeprom.h>
#include <GyverButton.h>
#include <iarduino_RTC.h>
#include <iarduino_OLED_txt.h>

#define TEXT_TIME 60  // Время автовыключения экрана текста в секундах
#define CLOCK_TIME 10  // Время автовыключения экрана часов в секундах
#define STRINGS 6
#define STR_LEN 20
#define CHR_LEN 6

int eeprom_i;
GButton btn(2);  // Указать пин для подключения кнопки
iarduino_OLED_txt myOLED(0x3C);  // А здесь адрес дисплея

// iarduino_RTC watch(RTC_DS1302, 3, 4, 5);  // А тут создаем обьект часов (Указываем свою модель - RTC_DS1302, RTC_DS1307 или RTC_DS3231)
// iarduino_RTC watch(RTC_DS1307);
iarduino_RTC watch(RTC_DS3231);

extern uint8_t SmallFontRus[];
extern uint8_t MediumFont[];

void setup() {
  myOLED.begin();
  myOLED.setFont(SmallFontRus);
  myOLED.setCoding(TXT_UTF8);
  myOLED.print("loading...", OLED_C, 1);
  myOLED.print("Запуск памяти...", OLED_C, 3);
  eeprom_write_byte(0, '\n');
  eeprom_write_byte(1022, '\0');
  myOLED.print("Запуск Serial...", OLED_C, 4);
  Serial.begin(1200);
  myOLED.print("Проверка памяти...", OLED_C, 5);
  int t_eeprom_i = 0;
  while (eeprom_read_byte(t_eeprom_i) != '\0') {
    t_eeprom_i++;
    Serial.print((char)eeprom_read_byte(t_eeprom_i));
    delay(5);
  }
  myOLED.print("Завершение загрузки...", OLED_C, 6);
  delay(1000);
  myOLED.clrScr();
}

void loop() {
  // Проверяем Serial и прописываем данные в EEPROM
  if (Serial.available()) {
    if (Serial.peek() == '\n') {
      Serial.print("\nЗаписано байт: ");
      Serial.println(eeprom_i);
      Serial.print("\nКонтрольный вывод: ");
      eeprom_write_byte(eeprom_i + 1, '\0');
      eeprom_i = 0;
      while (eeprom_read_byte(eeprom_i) != '\0') {
        eeprom_i++;
        Serial.print((char)eeprom_read_byte(eeprom_i));
      }
      Serial.println();
      eeprom_i = 0;
      Serial.read();
    }
    else {
      eeprom_i++;
      if (eeprom_i > 1020) {
        while (Serial.peek() != '\n') {
          Serial.read();
        }
        Serial.println("[error]\tПереполнение памяти макстимум - 1020 символов");
      }
      else {
        eeprom_write_byte(eeprom_i, Serial.read());
      }
      // Serial.write(Serial.read());
    }
  }
  // Обрабатываем события дисплея
  btn.tick();
  if (btn.isSingle()) {
    time();
  }
  if (btn.isDouble()) {
    text();
  }
}

void time() {
  // Serial.println(watch.gettime("d-m-Y, H:i:s, D"));
  uint32_t timer = millis();
  uint32_t i;
  int h, m, s;
  while (!btn.isSingle() and (millis() - timer < CLOCK_TIME * 1000)) {
    myOLED.setFont(MediumFont);
    btn.tick();
    i = millis();
    if (i % 1000 == 0) {
      watch.gettime();
      delay(1);
      i /= 1000; h = i / 60 / 60 % 24; m = i / 60 % 60; s = i % 60;
      myOLED.setCursor(16, 4);
      if (watch.Hours < 10) {
        myOLED.print(0);
      }
      myOLED.print(watch.Hours);
      myOLED.print(":");
      if (watch.minutes < 10) {
        myOLED.print(0);
      }
      myOLED.print(watch.minutes);
      myOLED.print(":");
      if (watch.seconds < 10) {
        myOLED.print(0);
      }
      myOLED.print(watch.seconds);
      myOLED.setFont(SmallFontRus);
      myOLED.print(watch.gettime("d-m-Y"), OLED_C, 7);
    }
  }
  myOLED.clrScr();
}

void text() {
  uint32_t timer = millis();
  render_page(0);
  int page = 0;
  while (!btn.isDouble() and (millis() - timer < TEXT_TIME * 1000)) {
    btn.tick();
    if (btn.isSingle()) {
      myOLED.clrScr();
      page++;
      // page %= pages;
      render_page(page);
    }
    if (btn.isDouble()) {
      break;
    }
  }
  myOLED.clrScr();
}

void render_page(int page) {
  int len = 0;
  int eeprom_i = 0;
  while (eeprom_read_byte(eeprom_i) != '\0') {
    eeprom_i++;
    //Serial.print(String((char)eeprom_read_byte(eeprom_i)));
  }
  eeprom_i--;
  int pages = (int)ceil((float)eeprom_i / (float)(STR_LEN * STRINGS));
  page = page % pages;
  myOLED.setFont(SmallFontRus);
  myOLED.setCoding(TXT_UTF8);
  int index;
  String temp;
  for (int i = 0; i < STRINGS; i++) {
    myOLED.setCursor(0, i);
    temp = "";
    int j = 0;
    int _shift = (page * STRINGS * STR_LEN);
    while (temp.length() < STR_LEN) {
      index = j + i * STR_LEN + 1;
      if ((char)eeprom_read_byte(index + _shift) == '\0') break;
      //Serial.print(index + _shift);
      temp += (char)eeprom_read_byte(index + _shift);
      // myOLED.print((char)eeprom_read_byte(index), j * CHR_LEN, i);
      j++;
    }
    myOLED.print(temp, 0, i);
    if ((char)eeprom_read_byte(index + _shift) == '\0') break;
  }
  temp = "page " + String(page + 1) + "/" + String(pages);
  myOLED.print(temp, OLED_L, STRINGS + 1);
  temp = "in mem " + String(eeprom_i) + "b";
  myOLED.print(temp, OLED_R, STRINGS + 1);
}
