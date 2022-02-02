/**
 * @file climp.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-01-04
 *
 * @copyright Copyright (c) 2022
 *
 * Espressif example for Timer
 * https://github.com/espressif/esp-idf/blob/master/examples/system/esp_timer/main/esp_timer_example_main.c
 *
 * Climb update formulae from
 * https://github.com/3s1d/tweety/tree/master/src climb.c
 */
#include <M5StickC.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "sdkconfig.h"

#include "climb.h"

#include "SparkFun_MS5637_Arduino_Library.h"
MS5637 barometricSensor;

int32_t climb_buffer[CLIMB_SAMPLES];
volatile uint8_t climb_buf_idx;
volatile int32_t d1, d2;

/* climb rate in cm/s */
int16_t climb_cms;

float last_pressure_reading = 0.0;
int average_altitude = 0;

// Let's calculate a window depending how fast the trigger is.
int8_t counter = 0;

static void periodic_timer_callback(void *arg)
{
	float pressure = barometricSensor.getPressure();

	// Validation code
	if (abs(last_pressure_reading - pressure) > pressure / 3)
	{
		Serial.print(millis());
		Serial.print(" ERROR: Pressure: ");
		Serial.print(pressure);
		Serial.print(", temperature: ");
		Serial.println(barometricSensor.getTemperature());
		return;
	}
	else
	{
		last_pressure_reading = pressure;
	}

	/* add new value to buffer */
	volatile int32_t alt_cm = ((1.0f - pow(pressure / 1013.25f, 0.190295f)) * 145366.45f) * 30.48f;
#ifdef __debug__
	Serial.print(millis());
	Serial.print(": Pressure: ");
	Serial.print(pressure);
	Serial.print(", alt_cm: ");
	Serial.println(alt_cm);
#endif

	{
		climb_buffer[climb_buf_idx] = alt_cm;
		if (++climb_buf_idx >= CLIMB_SAMPLES)
		{
			climb_buf_idx = 0;
		}
	}
}

void init_Timer(MS5637 &sensor)
{
	barometricSensor = sensor;

	last_pressure_reading = barometricSensor.getPressure();

	// Needed to change it from the Espressif example .c to .cpp initialization.
	esp_timer_create_args_t periodic_timer_args;
	periodic_timer_args.callback = &periodic_timer_callback;
	periodic_timer_args.name = "periodic";

	esp_timer_handle_t periodic_timer;
	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));

	/* Start the timers */
	ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 20000)); // 20ms
}

/* we are using linear regression here */
/* Code used from the tweety project.
 * there are two main variant as option -> Kalman Filter or Linear Regression.
 * Both are valid to be used, i have to dig deeper the advantage of the different method.
 * First i want to make it work.
 */
int16_t climb_update(double trigger_interval)
{
	/* compute average altitude */
	int32_t avg_alt = 0;

	for (uint8_t i = 0; i < CLIMB_SAMPLES; i++)
	{
		portDISABLE_INTERRUPTS();
		volatile int_fast32_t buf = climb_buffer[i];
		portENABLE_INTERRUPTS();
		avg_alt += buf;
	}

	avg_alt /= CLIMB_SAMPLES;
	average_altitude = avg_alt;

#ifdef __debug__
	Serial.print("Altitude: ");
	Serial.println(average_altitude);
	// debug_put((uint8_t *) &avg_alt, 3);
#endif

	/* compute LR numerator */
	int32_t LR_num = 0;
	uint8_t idx = climb_buf_idx;

	for (uint8_t i = 0; i < CLIMB_SAMPLES; i++)
	{
		/* first increasing, gives us a little time buffer */
		if (++idx >= CLIMB_SAMPLES)
			idx = 0;
		portDISABLE_INTERRUPTS();
		volatile int_fast32_t buf = climb_buffer[idx];
		portENABLE_INTERRUPTS();
		LR_num += ((int32_t)i - LR_x_cross) * (buf - avg_alt);
	}

	/* final climb value */
	volatile int_fast16_t climb = (LR_num * CLIMB_SAMPLES_PER_SEC) / LR_den;

	/* make publicly available */
	portDISABLE_INTERRUPTS();
	climb_cms = climb; // Todo: Why protect Climb?
	portENABLE_INTERRUPTS();
	Serial.print(millis());
	Serial.print(" Climb in cms: ");
	Serial.println(climb_cms);
	return climb_cms;
}