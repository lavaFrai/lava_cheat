#include <avr/eeprom.h>
#include <GyverButton.h>
#include <iarduino_RTC.h>
#include <iarduino_OLED_txt.h>


/*-----------------------------------------*/
/*Здесь необходимо указать дату и время,актуальные на момент загрузки*/
/*Без ведущих нулей, в числовом виде*/

#define DAY 10  // Число
#define MOUNTH 4  // Месяц
#define YEAR 21  // Год (только последние две цифры)

#define MINUTE 0  // Минуты
#define HOURS 18 // Часы (В 24 часовом формате)

/*------------------------------------------*/


iarduino_OLED_txt myOLED(0x3C);  // Здесь адрес дисплея
// iarduino_RTC watch(RTC_DS1302, 3, 4, 5);  // А тут создаем обьект часов (Указываем свою модель - RTC_DS1302, RTC_DS1307 или RTC_DS3231)
// iarduino_RTC watch(RTC_DS1307);
iarduino_RTC watch(RTC_DS3231);

extern uint8_t SmallFontRus[];

void setup() {
  Serial.begin(1200);
  myOLED.begin();
  myOLED.setFont(SmallFontRus);
  myOLED.setCoding(TXT_UTF8);
  myOLED.print("Дисплей работает", OLED_C, 1);
  eeprom_write_byte(0, '\n');
  eeprom_write_byte(1022, '\0');
  myOLED.print("Память подготовлена", OLED_C, 2);
  watch.settime(0, MINUTE, HOURS);
  watch.settime(-1, -1, -1, DAY, MOUNTH, YEAR);
  myOLED.print("Время установлено", OLED_C, 3);
}

void loop() {
  myOLED.print(watch.gettime("H:i:s"), OLED_C, 6);
  myOLED.print(watch.gettime("d-m-Y"), OLED_C, 7);
  Serial.println("Serial - OK!");
}
