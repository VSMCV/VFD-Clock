/*
 * MAX6920.c
 *
 *  Created on: Nov 06, 2016
 *      Author: Vlad
 */

#include <DAVE.h>

#define BRIGHTNESS_RAMP_TIME 30000

uint32_t digit_refresh_timer, digit_refresh_timer_status;
uint32_t brightness_timer, brightness_timer_status;

uint8_t number_of_digits;
uint8_t next_refresh = 0;
uint8_t brightness = 0;
uint8_t current_brightness = 0;
uint16_t *data;
uint32_t digit_refresh_time;

void smoothBrightness()
{
	if(brightness > current_brightness)
	{
		current_brightness++;
	}
	if(brightness < current_brightness)
	{
		current_brightness--;
	}

	PWM_SetDutyCycle(&BLANKING, ((100 - current_brightness) * 100));
}

void refreshDigit(void)
{
	for(uint8_t i = 0; i < 12; i++)
	{
		DIGITAL_IO_SetOutputLow(&CLK);

		if(((data[next_refresh] >> i) & 1) == 1)
		{
			DIGITAL_IO_SetOutputHigh(&DATA);
		}
		else
		{
			DIGITAL_IO_SetOutputLow(&DATA);
		}

		DIGITAL_IO_SetOutputHigh(&CLK);
	}

	DIGITAL_IO_SetOutputLow(&CLK);

	DIGITAL_IO_SetOutputHigh(&LOAD);

	DIGITAL_IO_SetOutputLow(&LOAD);

	next_refresh++;

	if(next_refresh == number_of_digits)
	{
		next_refresh = 0;
	}
}

void initializeDisplay(uint8_t display_size, uint32_t filament_freq, uint32_t scan_freq, uint16_t *screen)
{
	number_of_digits = display_size;

	digit_refresh_time = 1000000 / (scan_freq * number_of_digits);

	PWM_SetFreq(&FIL, filament_freq);

	digit_refresh_timer = SYSTIMER_CreateTimer(digit_refresh_time, SYSTIMER_MODE_PERIODIC, (void*)refreshDigit, NULL);
	brightness_timer = SYSTIMER_CreateTimer(BRIGHTNESS_RAMP_TIME, SYSTIMER_MODE_PERIODIC, (void*)smoothBrightness, NULL);

	data = screen;

	digit_refresh_timer_status = SYSTIMER_StartTimer(digit_refresh_timer);
	brightness_timer_status = SYSTIMER_StartTimer(brightness_timer);
}

void displayOn(void)
{
	PWM_SetDutyCycle(&FIL, 3000);
}

void displayOff(void)
{
	PWM_SetDutyCycle(&FIL, 10000);
}

void setBrightness(uint8_t new_brightness)
{
	brightness = new_brightness;
}
