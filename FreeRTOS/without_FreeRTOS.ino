#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
  pinMode(A0, INPUT);
  lcd.setCursor(0, 0);
  lcd.print("ADC reading:");
}

void loop() {
  Serial.println(analogRead(A0));
  lcd.setCursor(10, 1);
  lcd.print(analogRead(A0));
  delay(800);
  
}