#include <M5StickCPlus.h>
#include "utility/Speaker.h"

RTC_TimeTypeDef TimeStruct;

#include "SparkFun_MS5637_Arduino_Library.h"
MS5637 PressureSensor;

#include "climb.h"

int last_info = millis();

#define BUZZER_PIN 2

#define SOUND_PWM_CHANNEL 0
#define SOUND_RESOLUTION 8                     // 8 bit resolution
#define SOUND_ON (1 << (SOUND_RESOLUTION - 1)) // 50% duty cycle
#define SOUND_OFF 0                            // 0% duty cycle

void tone(int pin, int frequency, int duration)
{
  ledcSetup(SOUND_PWM_CHANNEL, frequency, SOUND_RESOLUTION); // Set up PWM channel
  ledcAttachPin(pin, SOUND_PWM_CHANNEL);                     // Attach channel to pin
  ledcWrite(SOUND_PWM_CHANNEL, SOUND_ON);
  delay(duration);
  ledcWrite(SOUND_PWM_CHANNEL, SOUND_OFF);
}

void display(int avg_alt, int climb_cms, uint32_t bg_color, uint16_t txt_color)
{
  M5.Lcd.setTextSize(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("ALT: ");
  M5.Lcd.println(average_altitude / 100.0);
  M5.Lcd.setTextSize(4);
  M5.Lcd.println("\nm/s: ");
  M5.Lcd.println((climb_cms * 2.0) / 100.0);
}

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
  M5.Lcd.setTextSize(4);

  while (1)
  {
    int16_t climb_cms = climb_update(0.5);
    // Climb
    // Precision of the MS5637 is about 13cm.

    if (abs(climb_cms) > 30)
    {
      double sink_ms = (climb_cms * 2.0) / 100.0;
      for (int i = 0; i < 12; i++)
      {
        if (climb[i] > sink_ms)
        {
          if (sink_ms < 0.0)
          {
            display(average_altitude, climb_cms, RED, BLACK);
          }
          else
          {
            display(average_altitude, climb_cms, GREEN, BLACK);
          }
#ifdef __debug__
          Serial.println(sink_ms);
          Serial.println(frequency[i]);
          Serial.println(duration[i]);
          Serial.println(duty[i]);
          Serial.println(duty[i] * (duration[i]));
#endif

          tone(26, frequency[i], duration[i]);
          vTaskDelay(duty[i] * (duration[i]));
          last_info = millis();
          break;
        }
      }
    }
    else
    {
      display(average_altitude, climb_cms, BLACK, WHITE);

      vTaskDelay(500);
    }
  }
}


