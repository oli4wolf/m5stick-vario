#include <M5StickCPlus.h>
#include "utility/Speaker.h"

RTC_TimeTypeDef TimeStruct;

#include "SparkFun_MS5637_Arduino_Library.h"
MS5637 PressureSensor;

#include "climb.h"

SPEAKER Speaker;

int last_info = millis();

void setup()
{
  M5.begin();
  Wire.begin(32, 33);
  M5.Lcd.setTextColor(YELLOW);                      // Set the font color to yellow.  设置字体颜色为黄色
  M5.Lcd.setTextSize(2);                            // Set the font size to 2.  设置字体大小为2
  M5.Lcd.println("M5StickC\nVario\n\now@owolf.ch"); // Print a string on the screen.  在屏幕上打印字符串
  delay(3000);
  M5.Lcd.fillScreen(BLACK); // Make the screen full of black (equivalent to clear() to clear the screen).  使屏幕充满黑色(等效clear()清屏)

  Serial.begin(115200);

  M5.Rtc.begin();

  Speaker.begin();

  // Initialize Barometer
  if (PressureSensor.begin() == false)
  {
    Serial.println("MS5637 sensor did not respond. Please check wiring.");
    while (1)
      ;
  }

  init_Timer(PressureSensor);

  vTaskDelay(3000);
}

void loop()
{
  M5.Lcd.fillScreen(BLACK);

  M5.Rtc.GetTime(&TimeStruct);
  M5.Lcd.setCursor(0, 15);
  M5.Lcd.printf("Time: %02d : %02d : %02d/n", TimeStruct.Hours, TimeStruct.Minutes, TimeStruct.Seconds);

  float temperature = PressureSensor.getTemperature();
  float pressure = PressureSensor.getPressure();

  M5.Lcd.setCursor(0, 100);
  M5.Lcd.print("Temperature=");
  M5.Lcd.print(temperature, 1);
  M5.Lcd.print("(C)");
  M5.Lcd.println();
  M5.Lcd.print(" Pressure=");
  M5.Lcd.print(pressure, 3);
  M5.Lcd.print("(hPa or mbar)");

  M5.Lcd.setTextSize(4);

  while (1)
  {
    int16_t climb_cms = climb_update(0.5);
    // Climb
    // Precision of the MS5637 is about 13cm.

    /** Tone logic **/
    if (abs(climb_cms) > 30)
    {
      double sink_ms = (climb_cms * 2.0) / 100.0;
      for (int i = 0; i < 12; i++)
      {
        if (climb[i] > sink_ms)
        {
          if (sink_ms < 0.0)
          {
            M5.Lcd.fillScreen(RED);
            M5.Lcd.setTextColor(BLACK);
            M5.Lcd.setCursor(0, 0);
          }
          else
          {
            M5.Lcd.fillScreen(GREEN);
            M5.Lcd.setTextColor(BLACK);
            M5.Lcd.setCursor(0, 0);
          }
#ifdef __debug__
          Serial.println(sink_ms);
          Serial.println(frequency[i]);
          Serial.println(duration[i]);
          Serial.println(duty[i]);
          Serial.println(duty[i] * (duration[i]));
#endif

          M5.Lcd.print("ALT: ");
          M5.Lcd.println(average_altitude);
          M5.Lcd.print("m/s: ");
          M5.Lcd.println((climb_cms * 2.0) / 100.0);

          // PlayTone(frequency[i], duty[i] * (duration[i] / 1000.0)); // Playtone is in seconds vTaskDelay in milliseconds
          // Speaker.tone(frequency[i], (duration[i]));
          if (i != 5)
          {
            Serial.print("Frequency with i:");
            Serial.println(i);
            //Speaker.tone(frequency[i]);
            //Speaker.beep();
          }
          else
          {
            Serial.println("Mute logic i==5");
            //Speaker.mute();
          }
          // delay(duration[i]);
          // Speaker.update();
          //  vTaskDelay(duty[i] * (duration[i]));
          //  vTaskDelay(duration[i] * (duty[i]/100.0));
          last_info = millis();
          break;
        }
      }
    }
    else
    {
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.print("ALT: ");
      M5.Lcd.println(average_altitude / 100.0);
      M5.Lcd.print("\nm/s: ");
      M5.Lcd.println((climb_cms * 2.0) / 100.0);
      vTaskDelay(500);
    }
  }
}
