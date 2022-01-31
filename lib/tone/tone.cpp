/**
 * @file tone.cpp
 * @author your name (you@domain.com)
 * @brief Class for tone array generation and Play
 * @version 0.1
 * @date 2022-01-02
 *
 * @copyright Copyright (c) 2022
 * https://github.com/adafruit/Adafruit_ZeroI2S/blob/master/examples/tone_generator/tone_generator.ino
 *
 */

#include <M5Core2.h>
#include <math.h>
#include <driver/i2s.h>

#define Speak_I2S_NUMBER I2S_NUM_0

#define SAMPLE_RATE 44100.0
#define RESOLUTION 512
#define AMPLITUDE 4024

extern const int sine_wave[512];

void GenerateSine(int amplitude, int *buffer, int buffer_length)
{
    /**
     * https://de.wikipedia.org/wiki/Schwingung#Harmonische_Schwingungen
     * y(t) = amplitude * sin(2*pi*f*t+nullphasenwinkel)
     * 2pi*f ist die Kreisfrequenz w.
     * nullphasenwinkel brauchenwir nicht.
     * Somit geschreiben amplitude * sin(w*t)
     */
#ifdef __debug__
    Serial.print("Generate Sine Wave");
    Serial.println(amplitude);
    Serial.println(buffer_length);
#endif
    for (int i = 0; i < buffer_length; ++i)
    {
        Serial.print(int(double(amplitude) * sin(2.0 * PI * (1.0 / buffer_length) * i)));
        Serial.print(", ");
        buffer[i] = int(float(amplitude) * sin(2.0 * PI * (1.0 / buffer_length) * i));
    }
}

void PlayTone(int frequency, float length_seconds)
{
#ifdef __debug__
    Serial.println("Calculate the sine wave.");
#endif
    //-> anzahl punkte der kurve berechnet für 1sekunde / abtastrate 1 sekunde.
    // -> 440*512 / 44100 = 5. Jeder 5te Punkt ist zu nehmen.
    float delta = (frequency * RESOLUTION) / float(SAMPLE_RATE);
    short sample[RESOLUTION]; // byte makes noise int does not :-()
    for (int i = 0; i < RESOLUTION; i++)
    {
        int pos = int(i * delta) % 512; // Modulo to loop back to the begining of the generated wave.
        sample[i] = sine_wave[pos];
        // Serial.print(sample[i]);
        // Serial.print("\n");
    }

    // Anzahl schlaufen zu durchlaufen
    // -> (SAMPLE_Rate / RESOLUTION) * Zeit (1s = 21)
    int iterations = (SAMPLE_RATE / RESOLUTION) * length_seconds;

    Serial.println("Play the sine wave.");
    // Serial.println(sizeof(sample));
    for (int j = 0; j < iterations; ++j)
    {
        size_t bytes_written = 0;
        i2s_write(Speak_I2S_NUMBER, &sample, sizeof(sample), &bytes_written, portMAX_DELAY);
    }
}