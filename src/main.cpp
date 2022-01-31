#include <M5Core2.h>

RTC_TimeTypeDef TimeStruct;

#include "SparkFun_MS5637_Arduino_Library.h"
MS5637 pressureSensor;

#include "speak.cpp"
#include "tone.h"

#include "climb.h"

File myFile;

int last_info = millis();

// Initialise SD Card and file
void initSD()
{
  if (!SD.begin(21))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }

  if (!SD.exists("/logger"))
  {
    SD.mkdir("/logger");
    Serial.println("Folder logger created.");
  }
  else
  {
    Serial.println("Folder logger already exist.");
  }

  myFile = SD.open("/logger/example.txt", FILE_WRITE);
  if (!myFile)
  {
    Serial.println("Could not create File.");
  }
  myFile.close();
  myFile = SD.open("/logger/example.txt", FILE_APPEND);
  myFile.println("New Start.");
  myFile.flush();
}

void setup()
{
  M5.begin(true, true, true, true); // Init M5Core2(Initialization of external I2C is also included).  初始化M5Core2(初始化外部I2C也包含在内)
  // Wire.begin(21, 22); //Detect internal I2C, if this sentence is not added, it will detect external I2C.  检测内部I2C,若不加此句为检测外部I2C
  M5.Lcd.setTextColor(YELLOW);          // Set the font color to yellow.  设置字体颜色为黄色
  M5.Lcd.setTextSize(2);                // Set the font size to 2.  设置字体大小为2
  M5.Lcd.println("M5Core2 I2C Tester"); // Print a string on the screen.  在屏幕上打印字符串
  delay(3000);
  M5.Lcd.fillScreen(BLACK); // Make the screen full of black (equivalent to clear() to clear the screen).  使屏幕充满黑色(等效clear()清屏)

  Serial.begin(115200);

  M5.Rtc.begin();

  // Initialize Barometer
  if (pressureSensor.begin() == false)
  {
    Serial.println("MS5637 sensor did not respond. Please check wiring.");
    while (1)
      ;
  }

  initSD(); // Initialize SD for logging.

  SpeakInit(); // Initialize Speaker on the M5core2.

  init_Timer(pressureSensor);

  vTaskDelay(3000);
}

int textColor = YELLOW;

void loop()
{

  M5.Lcd.fillScreen(BLACK);

  M5.Rtc.GetTime(&TimeStruct);
  M5.Lcd.setCursor(0, 15);
  M5.Lcd.printf("Time: %02d : %02d : %02d/n", TimeStruct.Hours, TimeStruct.Minutes, TimeStruct.Seconds);

  float temperature = pressureSensor.getTemperature();
  float pressure = pressureSensor.getPressure();

  M5.Lcd.setCursor(0, 100);
  M5.Lcd.print("Temperature=");
  M5.Lcd.print(temperature, 1);
  M5.Lcd.print("(C)");
  M5.Lcd.println();
  M5.Lcd.print(" Pressure=");
  M5.Lcd.print(pressure, 3);
  M5.Lcd.print("(hPa or mbar)");

  Serial.println(pressure);

  myFile.println(pressure);
  myFile.flush();

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
            M5.Lcd.setCursor(20, 60);
          }
          else
          {
            M5.Lcd.fillScreen(GREEN);
            M5.Lcd.setTextColor(BLACK);
            M5.Lcd.setCursor(20, 60);
          }
#ifdef __debug__
          Serial.println(sink_ms);
          Serial.println(frequency[i]);
          Serial.println(duration[i] / 1000.0);
          Serial.println(duty[i]);
          Serial.println(duty[i] * (duration[i]));
#endif

          M5.Lcd.print("ALT: ");
          M5.Lcd.println(average_altitude);
          M5.Lcd.print("m/s: ");
          M5.Lcd.println((climb_cms * 2.0) / 100.0);

          PlayTone(frequency[i], duty[i] * (duration[i] / 1000.0)); // Playtone is in seconds vTaskDelay in milliseconds
          vTaskDelay(duty[i] * (duration[i]));
          // vTaskDelay(duration[i] * (duty[i]/100.0));
          last_info = millis();
          break;
        }
      }
    }
    else
    {
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(0, 60);
      M5.Lcd.print("ALT: ");
      M5.Lcd.println(average_altitude / 100.0);
      M5.Lcd.print("m/s: ");
      M5.Lcd.println((climb_cms * 2.0) / 100.0);
      vTaskDelay(500);
    }
  }
}
