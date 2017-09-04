/*
 * main.c
 *
 *  Created on: 2017 Apr 29 15:17:59
 *  Author: Vlad
 */

#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include <MAX6920.h>
#include <stdlib.h>

#define LIGHT_SAMPLE_TIME 1000000
#define BUTTON_DEBOUNCE 250000
#define LONG_PRESS_TIME 2000000
#define SHOW_TIME 1500000
#define CAROUSEL_TIME 300000
#define BRIGHTNESS_HYSTERESIS 5

RTC_STATUS_t status = RTC_STATUS_FAILURE;
E_EEPROM_XMC1_OPERATION_STATUS_t oper_status = E_EEPROM_XMC1_OPERATION_STATUS_SUCCESS;

typedef struct
{
	uint8_t test_dig[30];
	uint8_t valid;
} test;

XMC_RTC_TIME_t time_val =
{
	.seconds = 0U,
	.minutes = 0U,
	.hours = 0U,
	.days = 1U,
	.month = 1U,
	.year = 2010U
};
XMC_RTC_TIME_t dst1, dst2;

uint8_t dst_reg = 1;
uint8_t century = 20;
uint8_t DST_set = 0;
uint8_t power_status = 1;

uint8_t butt1_status = 0, butt1_used = 0, butt2_status = 0, butt2_used = 0;
uint8_t message_0_printed = 0, message_1_printed = 0, message_2_printed = 0;
uint8_t text_0_pos, text_1_pos = 0, text_2_pos = 0;
uint8_t old_minutes = 61, old_year = 0;
uint32_t current_time = 0, last_butt1_time = 0, last_butt2_time = 0;
uint32_t carousel_0_timer, carousel_0_timer_status;
uint32_t carousel_1_timer, carousel_1_timer_status;
uint32_t carousel_2_timer, carousel_2_timer_status;
uint32_t light_timer, light_timer_status;
int32_t last_brightness_percentage = 0;
uint8_t blink_flag = 1;
uint8_t set_flag = 0;
uint8_t comm_finished = 0;
uint8_t battery_ok = 1;
uint8_t day_of_week = 1;

uint16_t screen[10];
uint8_t dig[20] = {2, 0, 1, 5, 1, 0, 1, 0, 0, 0, 0};
uint8_t write_data[5] = {1, 1, 20, 0, 1};
uint8_t read_data[5];

uint16_t conv[10][200] =
{
	{64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 1020, 120, 1500, 1276, 1656, 1780, 2036, 124, 2044, 1788, 1004, 104, 1484, 1260, 1640, 1764, 2020, 108, 2028, 1772, 64, 64, 64, 64, 64, 64, 64, 1900, 2028, 964, 64, 1988, 1860, 2020, 1896, 104, 232, 64, 960, 64, 64, 1004, 1868, 64, 64, 1764, 64, 1000, 64, 64, 64, 64, 1484, 64, 64, 64, 64, 64, 64, 1516, 2016, 1472, 1512, 1996, 64, 1772, 1888, 96, 64, 64, 832, 64, 1376, 1504, 64, 64, 1344, 64, 1984, 480, 64, 64, 64, 1640, 64, 64, 64, 64, 64, 64},
	{2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 3004, 2104, 3484, 3260, 3640, 3764, 4020, 2108, 4028, 3772, 2988, 2088, 3468, 3244, 3624, 3748, 4004, 2092, 4012, 3756, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 3884, 4012, 2948, 2048, 3972, 3844, 4004, 3880, 2088, 2216, 2048, 2944, 2048, 2048, 2988, 3852, 2048, 2048, 3748, 2048, 2984, 2048, 2048, 2048, 2048, 3468, 2048, 2048, 2048, 2048, 2048, 2048, 3500, 4000, 3456, 3496, 3980, 2048, 3756, 3872, 2080, 2048, 2048, 2816, 2048, 3360, 3488, 2048, 2048, 3328, 2048, 3968, 2464, 2048, 2048, 2048, 3624, 2048, 2048, 2048, 2048, 2048, 2048},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 957, 57, 1437, 1213, 1593, 1717, 1973, 61, 1981, 1725, 941, 41, 1421, 1197, 1577, 1701, 1957, 45, 1965, 1709, 1, 1, 1, 1, 1, 1, 1, 1837, 1965, 901, 1, 1925, 1797, 1957, 1833, 41, 169, 1, 897, 1, 1, 941, 1805, 1, 1, 1701, 1, 937, 1, 1, 1, 1, 1421, 1, 1, 1, 1, 1, 1, 1453, 1953, 1409, 1449, 1933, 1, 1709, 1825, 33, 1, 1, 769, 1, 1313, 1441, 1, 1, 1281, 1, 1921, 417, 1, 1, 1, 1577, 1, 1, 1, 1, 1, 1},
	{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 958, 58, 1438, 1214, 1594, 1718, 1974, 62, 1982, 1726, 942, 42, 1422, 1198, 1578, 1702, 1958, 46, 1966, 1710, 2, 2, 2, 2, 2, 2, 2, 1838, 1966, 902, 2, 1926, 1798, 1958, 1834, 42, 170, 2, 898, 2, 2, 942, 1806, 2, 2, 1702, 2, 938, 2, 2, 2, 2, 1422, 2, 2, 2, 2, 2, 2, 1454, 1954, 1410, 1450, 1934, 2, 1710, 1826, 34, 2, 2, 770, 2, 1314, 1442, 2, 2, 1282, 2, 1922, 418, 2, 2, 2, 1578, 2, 2, 2, 2, 2, 2}
};

char message_0[500], message_1[100], message_2[100];

void butt1Interrupt(void);
void butt2Interrupt(void);
uint8_t checkButt1(uint8_t continuous);
uint8_t checkButt2(uint8_t continuous);
void lightAdjust(void);
void lightSample(void);
void writeData(void);
void trans(void);
void rec(void);
uint8_t decToBcd(uint8_t value);
uint8_t bcdToDec(uint8_t value);
void setRTC(void);
uint8_t readRTC(void);
void advanceText0(void);
void advanceText1(void);
void advanceText2(void);
void print(uint8_t update_0, uint8_t update_1, uint8_t update_2);
void clearDisplay(uint8_t clear_0, uint8_t clear_1, uint8_t clear_2);
void writeMonth(uint8_t m, uint8_t mess);
void delayer(uint32_t delay_time);
void updateClock(void);
uint8_t checkLvl2(XMC_RTC_TIME_t check_val);
uint8_t checkLvl1(test check_object);
test nextDig(test object, uint8_t dig_no, uint8_t f);
uint8_t dateComp(XMC_RTC_TIME_t comp);
void findDST(void);
uint8_t checkDST(void);
void advanceTime(void);
void reverseTime(void);
void refreshSetDisplay(uint8_t i);
void setClock(void);

int main(void)
{
 	DAVE_Init();           /* Initialization of DAVE APPs  */

	RTC_Stop();

	light_timer = SYSTIMER_CreateTimer(LIGHT_SAMPLE_TIME, SYSTIMER_MODE_PERIODIC, (void*)lightSample, NULL);
	carousel_0_timer = SYSTIMER_CreateTimer(CAROUSEL_TIME, SYSTIMER_MODE_PERIODIC, (void*)advanceText0, NULL);
	carousel_1_timer = SYSTIMER_CreateTimer(CAROUSEL_TIME, SYSTIMER_MODE_PERIODIC, (void*)advanceText1, NULL);
	carousel_2_timer = SYSTIMER_CreateTimer(CAROUSEL_TIME, SYSTIMER_MODE_PERIODIC, (void*)advanceText2, NULL);

	delayer(500000);

	strcpy(message_0, "    ");
	print(1, 0, 0);

	initializeDisplay(4, 10000, 60, screen);

	light_timer_status = SYSTIMER_StartTimer(light_timer);

	delayer(500000);
/*
	write_data[0] = 0;
	writeData();
*/
 	if(oper_status == E_EEPROM_XMC1_OPERATION_STATUS_SUCCESS)
 	{
 	    oper_status = E_EEPROM_XMC1_Read(1, 0U, read_data, 5);
 	}

 	if(read_data[0] == 0)
 	{
 		strcpy(message_0, "    hELLO     ");
 		print(1, 0, 0);
 		while(message_0_printed == 0)
 		{

 		}

 		setClock();
 	}
 	else
 	{
 		dst_reg = read_data[1];
 		century = read_data[2];
 		DST_set = read_data[3];
 		power_status = read_data[4];

 		battery_ok = readRTC();

 		if(battery_ok != 1)
 		{
 			strcpy(message_0, "    PLEASE rEPLACE BAttEry thEn SEt tinne     ");
 			print(1, 0, 0);
 			while(message_0_printed == 0)
 			{

 			}

 			time_val.seconds = 0;
 			time_val.minutes = 0;
 			time_val.hours = 0;
 			time_val.days = 1;
 			time_val.month = 1;
 			time_val.year = (century * 100) + 10;
 		}
 	}

	while(1)
	{
		current_time = SYSTIMER_GetTime();

		readRTC();

		if(time_val.year != old_year)
		{
			findDST();

			old_year = time_val.year;
		}

		if(time_val.minutes != old_minutes)
		{
			if((checkDST() == 1) && (DST_set == 0))
			{
				advanceTime();

				DST_set = 1;
				writeData();
			}
			if((checkDST() == 0) && (DST_set == 1))
			{
				reverseTime();

				DST_set = 0;
				writeData();
			}

			updateClock();

			old_minutes = time_val.minutes;
		}

		if(checkButt1(0) == 1)
		{
			if(power_status == 1)
			{
				setClock();
			}
		}

		if(checkButt2(0) == 1)
		{
			if(power_status == 1)
			{
				power_status = 0;
				writeData();
			}
			else
			{
				power_status = 1;
				writeData();
			}
		}

		if(power_status == 1)
		{
			displayOn();
		}
		else
		{
			displayOff();
		}
	}

	return 0;
}

void butt1Interrupt(void)
{
	if(PIN_INTERRUPT_GetPinValue(&BUTT1) == 0)
	{
		butt1_status = 1;

		butt1_used = 0;
	}
	else
	{
		butt1_status = 0;
	}
}

void butt2Interrupt(void)
{
	if(PIN_INTERRUPT_GetPinValue(&BUTT2) == 0)
	{
		butt2_status = 1;

		butt2_used = 0;
	}
	else
	{
		butt2_status = 0;
	}
}

uint8_t checkButt1(uint8_t continuous)
{
	if((butt1_status == 1) && ((current_time - last_butt1_time) > BUTTON_DEBOUNCE))
	{
		last_butt1_time = current_time;

		if(continuous == 1)
		{
			butt1_used = 1;

			return 1;
		}
		else
		{
			if(butt1_used == 0)
			{
				butt1_used = 1;

				return 1;
			}
		}
	}

	return 0;
}

uint8_t checkButt2(uint8_t continuous)
{
	if((butt2_status == 1) && ((current_time - last_butt2_time) > BUTTON_DEBOUNCE))
	{
		last_butt2_time = current_time;

		if(continuous == 1)
		{
			butt2_used = 1;

			return 1;
		}
		else
		{
			if(butt2_used == 0)
			{
				butt2_used = 1;

				return 1;
			}
		}
	}

	return 0;
}

void lightAdjust(void)
{
	XMC_VADC_RESULT_SIZE_t result;

	result = ADC_MEASUREMENT_GetGlobalResult();
	result = result >> ((uint32_t)LIGHT.iclass_config_handle->conversion_mode_standard * (uint32_t)2);

	int32_t percentage = ((int32_t)result * (0 - 7) + 25700) / 250;

	if(percentage > 100)
	{
		percentage = 100;
	}
	if(percentage < 5)
	{
		percentage = 5;
	}

	if(abs(percentage - last_brightness_percentage) > BRIGHTNESS_HYSTERESIS)
	{
		last_brightness_percentage = percentage;

		setBrightness(percentage);
	}
}

void lightSample(void)
{
	ADC_MEASUREMENT_StartConversion(&LIGHT);
}

void writeData(void)
{
	write_data[1] = dst_reg;
	write_data[2] = century;
	write_data[3] = DST_set;
	write_data[4] = power_status;
 	if(oper_status == E_EEPROM_XMC1_OPERATION_STATUS_SUCCESS)
 	{
 		oper_status = E_EEPROM_XMC1_Write(1, write_data);
 	}
}

void trans(void)
{
	comm_finished = 1;
}

void rec(void)
{
	comm_finished = 1;
}

uint8_t decToBcd(uint8_t value)
{
	return (value / 10 * 16 + value % 10);
}

uint8_t bcdToDec(uint8_t value)
{
	return ((value / 16) * 10 + value % 16);
}

void setRTC(void)
{
	uint8_t tx_data[8];

	tx_data[0] = 0x2;
	tx_data[1] = decToBcd(time_val.seconds);
	tx_data[2] = decToBcd(time_val.minutes);
	tx_data[3] = decToBcd(time_val.hours);
	tx_data[4] = decToBcd(time_val.days);
	tx_data[5] = decToBcd(day_of_week);
	tx_data[6] = decToBcd(time_val.month);
	tx_data[7] = decToBcd((time_val.year - (century * 100)));

	comm_finished = 0;
	I2C_MASTER_Transmit(&I2C_MASTER_0, true, 0xA2, tx_data, 8, true);
	while(comm_finished == 0)
	{

	}
}

uint8_t readRTC(void)
{
	uint8_t tx_data[1];
	uint8_t rx_data[7];
	uint8_t integrity_flag;
	uint8_t century_flag;

	tx_data[0] = 0x2;

	comm_finished = 0;
	I2C_MASTER_Transmit(&I2C_MASTER_0, true, 0xA2, tx_data, 1, true);
	while(comm_finished == 0)
	{

	}

	comm_finished = 0;
	I2C_MASTER_Receive(&I2C_MASTER_0, true, 0xA2, rx_data, 7, true, true);
	while(comm_finished == 0)
	{

	}

	century_flag = (rx_data[5] & 128) >> 7;
	integrity_flag = !((rx_data[0] & 128) >> 7);

	time_val.seconds = bcdToDec(rx_data[0] & 127);
	time_val.minutes = bcdToDec(rx_data[1] & 127);
	time_val.hours = bcdToDec(rx_data[2] & 63);
	time_val.days = bcdToDec(rx_data[3] & 63);
	day_of_week = bcdToDec(rx_data[4] & 7);
	time_val.month = bcdToDec(rx_data[5] & 31);
	time_val.year = (century * 100) + bcdToDec(rx_data[6]);

	if(century_flag == 1)
	{
		century++;
		writeData();

		time_val.year = (century * 100) + bcdToDec(rx_data[6]);

		setRTC();
	}

	return integrity_flag;
}

void advanceText0(void)
{
	for(uint8_t i = 0; i < 4; i++)
	{
		screen[i] = conv[i][(uint16_t)message_0[(text_0_pos + i)]];
	}

	text_0_pos++;

	if(message_0[(text_0_pos + 3)] == '\0')
	{
		text_0_pos = 0;

		message_0_printed = 1;
	}
}

void advanceText1(void)
{
	for(uint8_t i = 0; i < 2; i++)
	{
		screen[i] = conv[i][(uint16_t)message_1[(text_1_pos + i)]];
	}

	text_1_pos++;

	if(message_1[(text_1_pos + 1)] == '\0')
	{
		text_1_pos = 0;

		message_1_printed = 1;
	}
}

void advanceText2(void)
{
	for(uint8_t i = 0; i < 2; i++)
	{
		screen[(i + 2)] = conv[i + 2][(uint16_t)message_2[(text_2_pos + i)]];
	}

	text_2_pos++;

	if(message_2[(text_1_pos + 1)] == '\0')
	{
		text_2_pos = 0;

		message_2_printed = 1;
	}
}

void print(uint8_t update_0, uint8_t update_1, uint8_t update_2)
{
	uint8_t leng_0, leng_1, leng_2;

	if(update_0 == 1)
	{
		clearDisplay(1, 0, 0);

		message_0_printed = 0;

		leng_0 = strlen(message_0);

		if(leng_0 < 5)
		{
			for(uint8_t i = 0; i < leng_0; i++)
			{
				screen[i] = conv[i][(uint16_t)message_0[i]];
			}

			message_0_printed = 1;
		}
		else
		{
			text_0_pos = 1;

			for(uint8_t i = 0; i < 4; i++)
			{
				screen[i] = conv[i][(uint16_t)message_0[i]];
			}

			carousel_0_timer_status = SYSTIMER_StartTimer(carousel_0_timer);
		}
	}
	if(update_1 == 1)
	{
		clearDisplay(0, 1, 0);

		message_1_printed = 0;

		leng_1 = strlen(message_1);

		if(leng_1 < 3)
		{
			for(uint8_t i = 0; i < leng_1; i++)
			{
				screen[i] = conv[i][(uint16_t)message_1[i]];
			}

			message_1_printed = 1;
		}
		else
		{
			text_1_pos = 1;

			for(uint8_t i = 0; i < 2; i++)
			{
				screen[i] = conv[i][(uint16_t)message_1[i]];
			}

			carousel_1_timer_status = SYSTIMER_StartTimer(carousel_1_timer);
		}
	}
	if(update_2 == 1)
	{
		clearDisplay(0, 0, 1);

		message_2_printed = 0;

		leng_2 = strlen(message_2);

		if(leng_2 < 3)
		{
			for(uint8_t i = 0; i < leng_2; i++)
			{
				screen[(i + 2)] = conv[(i + 2)][(uint16_t)message_2[i]];
			}

			message_2_printed = 1;
		}
		else
		{
			text_2_pos = 1;

			for(uint8_t i = 0; i < 2; i++)
			{
				screen[(i + 2)] = conv[(i + 2)][(uint16_t)message_2[i]];
			}

			carousel_2_timer_status = SYSTIMER_StartTimer(carousel_2_timer);
		}
	}
}

void clearDisplay(uint8_t clear_0, uint8_t clear_1, uint8_t clear_2)
{
	if(clear_0 == 1)
	{
		carousel_0_timer_status = SYSTIMER_StopTimer(carousel_0_timer);
		carousel_1_timer_status = SYSTIMER_StopTimer(carousel_1_timer);
		carousel_2_timer_status = SYSTIMER_StopTimer(carousel_2_timer);

		for(uint8_t i = 0; i < 4; i++)
		{
			screen[i] = 0;
		}
	}
	if(clear_1 == 1)
	{
		carousel_0_timer_status = SYSTIMER_StopTimer(carousel_0_timer);
		carousel_1_timer_status = SYSTIMER_StopTimer(carousel_1_timer);

		for(uint8_t i = 0; i < 2; i++)
		{
			screen[i] = 0;
		}
	}
	if(clear_2 == 1)
	{
		carousel_0_timer_status = SYSTIMER_StopTimer(carousel_0_timer);
		carousel_2_timer_status = SYSTIMER_StopTimer(carousel_2_timer);

		for(uint8_t i = 0; i < 2; i++)
		{
			screen[(i + 2)] = 0;
		}
	}
}

void writeMonth(uint8_t m, uint8_t mess)
{
	if(mess == 0)
	{
		if(m == 1) strcpy(message_0, "JAn");
		if(m == 2) strcpy(message_0, "FEB");
		if(m == 3) strcpy(message_0, "nnAr");
		if(m == 4) strcpy(message_0, "APr");
		if(m == 5) strcpy(message_0, "nnAy");
		if(m == 6) strcpy(message_0, "JUn");
		if(m == 7) strcpy(message_0, "JUL");
		if(m == 8) strcpy(message_0, "AUG");
		if(m == 9) strcpy(message_0, "SEP");
		if(m == 10) strcpy(message_0, "OCt");
		if(m == 11) strcpy(message_0, "nOu");
		if(m == 12) strcpy(message_0, "dEC");
	}
	if(mess == 1)
	{
		if(m == 1) strcpy(message_1, "  JAn   ");
		if(m == 2) strcpy(message_1, "  FEB   ");
		if(m == 3) strcpy(message_1, "  nnAr   ");
		if(m == 4) strcpy(message_1, "  APr   ");
		if(m == 5) strcpy(message_1, "  nnAy   ");
		if(m == 6) strcpy(message_1, "  JUn   ");
		if(m == 7) strcpy(message_1, "  JUL   ");
		if(m == 8) strcpy(message_1, "  AUG   ");
		if(m == 9) strcpy(message_1, "  SEP   ");
		if(m == 10) strcpy(message_1, "  OCt   ");
		if(m == 11) strcpy(message_1, "  nOu   ");
		if(m == 12) strcpy(message_1, "  dEC   ");
	}
}

void delayer(uint32_t delay_time)
{
	uint32_t enter_time;
	enter_time = current_time;

	while ((current_time - enter_time) < delay_time)
	{
		current_time = SYSTIMER_GetTime();
	}
}

void updateClock(void)
{
	if(battery_ok == 0)
	{
		strcpy(message_0, "    PLEASE rEPLACE BAttEry thEn SEt tinne     ");
		print(1, 0, 0);
		while(message_0_printed == 0)
		{

		}
	}

	message_0[0] = '0' + (time_val.hours / 10);
	message_0[1] = '0' + (time_val.hours % 10) - 10;
	message_0[2] = '0' + (time_val.minutes / 10);
	message_0[3] = '0' + (time_val.minutes % 10);
	message_0[4] = '\0';

	print(1, 0, 0);
}

uint8_t checkLvl2(XMC_RTC_TIME_t check_val)
{
	XMC_RTC_TIME_t check_result;

	status = RTC_STATUS_FAILURE;
	status = RTC_SetTime(&check_val);

	delayer(10000);

	RTC_GetTime(&check_result);

	RTC_Stop();

	if(
		(check_result.year != check_val.year) ||
		(check_result.month != check_val.month) ||
		(check_result.days != check_val.days) ||
		(check_result.hours != check_val.hours) ||
		(check_result.minutes != check_val.minutes)
	)
	{

		return 0;
	}
	else
	{
		return 1;
	}
}

uint8_t checkLvl1(test check_object)
{
	XMC_RTC_TIME_t check_val;

	check_val.year = check_object.test_dig[0] * 1000 + check_object.test_dig[1] * 100 + check_object.test_dig[2] * 10 + check_object.test_dig[3];
	check_val.month = check_object.test_dig[4];
	check_val.days = check_object.test_dig[5] * 10 + check_object.test_dig[6];
	check_val.hours = check_object.test_dig[7] * 10 + check_object.test_dig[8];
	check_val.minutes = check_object.test_dig[9] * 10 + check_object.test_dig[10];

	if(
		(check_val.year != (check_object.test_dig[0] * 1000 + check_object.test_dig[1] * 100 + check_object.test_dig[2] * 10 + check_object.test_dig[3])) ||
		(check_val.month != check_object.test_dig[4]) ||
		(check_val.days != (check_object.test_dig[5] * 10 + check_object.test_dig[6])) ||
		(check_val.hours != (check_object.test_dig[7] * 10 + check_object.test_dig[8])) ||
		(check_val.minutes != (check_object.test_dig[9] * 10 + check_object.test_dig[10]))
	)
	{
		return 0;
	}
	else
	{
		return (checkLvl2(check_val));
	}
}

test nextDig(test object, uint8_t dig_no, uint8_t f)
{
	uint8_t i = object.test_dig[dig_no];

	uint8_t bec = object.test_dig[dig_no];

	do
	{
		object.test_dig[dig_no] = i;

		if(dig_no != f)
		{
			object = nextDig(object, (dig_no + 1), f);

			if(object.valid == 1)
			{
				return object;
			}
		}
		else
		{
			if(checkLvl1(object) == 1)
			{
				object.valid = 1;

				return object;
			}
		}

		i++;

		if(dig_no == 4)
		{
			if(i == 13)
			{
				i = 1;
			}
		}
		else if(dig_no == 5)
		{
			if(i == 4)
			{
				i = 0;
			}
		}
		else if(dig_no == 7)
		{
			if(i == 3)
			{
				i = 0;
			}
		}
		else if(dig_no == 9)
		{
			if(i == 6)
			{
				i = 0;
			}
		}
		else
		{
			if(i == 10)
			{
				i = 0;
			}
		}
	} while(i != bec);

	object.test_dig[dig_no] = bec;

	object.valid = 0;

	return object;
}

uint8_t dateComp(XMC_RTC_TIME_t comp)
{
	if(comp.year > time_val.year)
	{
		return 1;
	}
	if(comp.year < time_val.year)
	{
		return 0;
	}
	if(comp.year == time_val.year)
	{
		if(comp.month > time_val.month)
		{
			return 1;
		}
		if(comp.month < time_val.month)
		{
			return 0;
		}
		if(comp.month == time_val.month)
		{
			if(comp.days > time_val.days)
			{
				return 1;
			}
			if(comp.days < time_val.days)
			{
				return 0;
			}
			if(comp.days == time_val.days)
			{
				if(comp.hours > time_val.hours)
				{
					return 1;
				}
				if(comp.hours < time_val.hours)
				{
					return 0;
				}
				if(comp.hours == time_val.hours)
				{
					if(comp.minutes > time_val.minutes)
					{
						return 1;
					}
					if(comp.minutes < time_val.minutes)
					{
						return 0;
					}
					if(comp.minutes == time_val.minutes)
					{
						if(comp.seconds > time_val.seconds)
						{
							return 1;
						}
						if(comp.seconds < time_val.seconds)
						{
							return 0;
						}
						if(comp.seconds == time_val.seconds)
						{
							return 0;
						}
					}
				}
			}
		}
	}
	return 2;
}

void findDST(void)
{
	uint8_t weekday;
	uint32_t d;
	uint32_t m;
	uint32_t y;

	if(dst_reg == 1)
	{
		dst1.seconds = 0U;
		dst1.minutes = 0U;
		dst1.hours = 3U;
		dst1.days = 31U;
		dst1.month = 3U;
		dst1.year = 1970U;

		dst2.seconds = 0U;
		dst2.minutes = 0U;
		dst2.hours = 4U;
		dst2.days = 31U;
		dst2.month = 10U;
		dst2.year = 1970U;
	}
	if(dst_reg == 2)
	{
		dst1.seconds = 0U;
		dst1.minutes = 0U;
		dst1.hours = 2U;
		dst1.days = 31U;
		dst1.month = 3U;
		dst1.year = 1970U;

		dst2.seconds = 0U;
		dst2.minutes = 0U;
		dst2.hours = 2U;
		dst2.days = 31U;
		dst2.month = 11U;
		dst2.year = 1970U;
	}

	dst1.year = time_val.year;
	d = dst1.days;
	m = dst1.month;
	y = dst1.year;
	weekday = (d+=m<3?y--:y-2,23*m/9+d+4+y/4-y/100+y/400)%7;
	while(weekday != 0)
	{
		dst1.days--;

		weekday--;
	}
	if(dst_reg == 2)
	{
		dst1.days = dst1.days - 14;
	}

	dst2.year = time_val.year;
	d = dst2.days;
	m = dst2.month;
	y = dst2.year;
	weekday = (d+=m<3?y--:y-2,23*m/9+d+4+y/4-y/100+y/400)%7;
	while(weekday != 0)
	{
		dst2.days--;

		weekday--;
	}
	if(dst_reg == 2)
	{
		dst2.days = dst2.days - 21;
	}
}

uint8_t checkDST(void)
{
	if(dst_reg != 0)
	{
		if(dateComp(dst1) == 1)
		{
			return 0;
		}
		else if(dateComp(dst2) == 1)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

void advanceTime(void)
{
	time_val.hours++;

	if(time_val.hours == 24)
	{
		time_val.hours = 0;

		time_val.days++;

		if(checkLvl2(time_val) != 1)
		{
			time_val.days = 1;

			time_val.month++;

			if(time_val.month == 13)
			{
				time_val.month = 1;

				time_val.year++;
			}
		}
	}

	setRTC();
}

void reverseTime(void)
{
	if(time_val.hours > 0)
	{
		time_val.hours--;
	}
	else
	{
		time_val.hours = 23;

		if(time_val.days > 1)
		{
			time_val.days--;
		}
		else
		{
			time_val.days = 31;

			if(time_val.month > 1)
			{
				time_val.month--;

				while(checkLvl2(time_val) != 1)
				{
					time_val.days--;
				}
			}
			else
			{
				time_val.month = 12;

				time_val.year--;

				time_val.days = 31;
			}
		}
	}

	setRTC();
}

void refreshSetDisplay(uint8_t i)
{
	if(i < 4)
	{
		message_0[0] = '0' + dig[0];
		message_0[1] = '0' + dig[1];
		message_0[2] = '0' + dig[2];
		message_0[3] = '0' + dig[3];
		message_0[4] = '\0';

		message_0[i] = message_0[i] - 10 * blink_flag;

		print(1, 0, 0);
	}
	if(i == 4)
	{
		writeMonth(dig[4], 0);

		print(1, 0, 0);
	}
	if((i > 4) && (i < 7))
	{
		message_2[0] = '0' + dig[5];
		message_2[1] = '0' + dig[6];
		message_2[3] = '\0';

		message_2[(i - 1) % 4] = message_2[(i - 1) % 4] - 10 * blink_flag;

		print(0, 0, 1);
	}
	if(i > 6)
	{
		message_0[0] = '0' + dig[7];
		message_0[1] = '0' + dig[8];
		message_0[2] = '0' + dig[9];
		message_0[3] = '0' + dig[10];
		message_0[4] = '\0';

		message_0[(i - 3) % 4] = message_0[(i - 3) % 4] - 10 * blink_flag;

		print(1, 0, 0);
	}
}

void setClock(void)
{
	test start_object;
	test result_object;

	uint8_t dig_change_flag = 0;
	uint8_t req_flag;
	uint32_t last_blink_time = 0;

	RTC_Stop();

	set_flag = 0;

	dig[0] = (time_val.year / 1000) % 10;
	dig[1] = (time_val.year / 100) % 10;
	dig[2] = (time_val.year / 10) % 10;
	dig[3] = time_val.year % 10;
	dig[4] = time_val.month;
	dig[5] = (time_val.days / 10) % 10;
	dig[6] = time_val.days % 10;
	dig[7] = (time_val.hours / 10) % 10;
	dig[8] = time_val.hours % 10;
	dig[9] = (time_val.minutes / 10) % 10;
	dig[10] = time_val.minutes % 10;

	strcpy(message_0, "    PLEASE SEt dSt rEGIOn     ");
	print(1, 0, 0);

	while(1)
	{
		current_time = SYSTIMER_GetTime();

		if(checkButt2(0) == 1)
		{
			break;
		}
	}

	if(dst_reg == 1)
	{
		strcpy(message_0, "    rOnnAnIA     ");
		print(1, 0, 0);
	}
	if(dst_reg == 2)
	{
		strcpy(message_0, "    USA     ");
		print(1, 0, 0);
	}
	if(dst_reg == 0)
	{
		strcpy(message_0, "    nO dSt     ");
		print(1, 0, 0);
	}

	while(1)
	{
		current_time = SYSTIMER_GetTime();

		switch(dst_reg)
		{
			case 1:

				if(checkButt1(1) == 1)
				{
					strcpy(message_0, "    USA     ");
					print(1, 0, 0);

					dst_reg = 2;
					writeData();
				}

				break;

			case 2:

				if(checkButt1(1) == 1)
				{
					strcpy(message_0, "    nO dSt     ");
					print(1, 0, 0);

					dst_reg = 0;
					writeData();
				}

				break;

			case 0:

				if(checkButt1(1) == 1)
				{
					strcpy(message_0, "    rOnnAnIA     ");
					print(1, 0, 0);

					dst_reg = 1;
					writeData();
				}

				break;

			default:

				break;
		}

		if(checkButt2(0) == 1)
		{
			break;
		}
	}

	if(dst_reg == 0)
	{
		req_flag = 7;
	}
	else
	{
		req_flag = 0;
	}

	for(uint8_t i = req_flag; i < 11; i++)
	{
		if(i == 0)
		{
			strcpy(message_0, "yEAr");
			print(1, 0, 0);

			delayer(SHOW_TIME);
		}
		if(i == 4)
		{
			strcpy(message_0, "dAtE");
			print(1, 0, 0);

			delayer(SHOW_TIME);
		}
		if(i == 7)
		{
			if(req_flag != 7)
			{
				strcpy(message_0, "    tinne     ");
				print(1, 0, 0);
				while(message_0_printed == 0)
				{

				}
			}
		}

		refreshSetDisplay(i);

		while(1)
		{
			current_time = SYSTIMER_GetTime();

			if((current_time - last_blink_time) > 500000)
			{
				if(blink_flag == 0)
				{
					blink_flag = 1;
				}
				else
				{
					blink_flag = 0;
				}

				refreshSetDisplay(i);

				last_blink_time = current_time;
			}

			if(checkButt1(1) == 1)
			{
				dig[i]++;

				if(i == 0)
				{
					if(dig[0] == 9)
					{
						dig[0] = 0;
					}
				}

				if(i == 4)
				{
					if(dig[4] == 13)
					{
						dig[4] = 1;
					}
				}
				else if(i == 5)
				{
					if(dig[5] == 4)
					{
						dig[5] = 0;
					}
				}
				else if(i == 7)
				{
					if(dig[7] == 3)
					{
						dig[7] = 0;
					}
				}
				else if(i == 9)
				{
					if(dig[9] == 6)
					{
						dig[9] = 0;
					}
				}
				else
				{
					if(dig[i] == 10)
					{
						dig[i] = 0;
					}
				}

				if(dst_reg != 0)
				{
					if((dig[4] == dst1.month) && ((dig[5] * 10 + dig[6]) == dst1.days))
					{
						if(i == 8)
						{
							if((dig[7] * 10 + dig[8]) == dst1.hours)
							{
								dig[8]++;

								dig_change_flag = 1;
							}
						}
					}
				}

				for(int j = 0; j < 11; j++)
				{
					start_object.test_dig[j] = dig[j];
				}
				start_object.valid = 0;

				dig_change_flag = 1;

				if(i < 3)
				{
					result_object = nextDig(start_object, i, 2);
					dig[i] = result_object.test_dig[i];
				}
				else if(i < 7)
				{
					result_object = nextDig(start_object, i, 6);
					dig[i] = result_object.test_dig[i];
				}
				else if(i < 9)
				{
					result_object = nextDig(start_object, i, 8);
					dig[i] = result_object.test_dig[i];
				}
				else
				{
					result_object = nextDig(start_object, i, 10);
					dig[i] = result_object.test_dig[i];
				}

				refreshSetDisplay(i);
			}

			if(checkButt2(0) == 1)
			{
				if(i == 6)
				{
					time_val.year = dig[0] * 1000 + dig[1] * 100 + dig[2] * 10 + dig[3];
					time_val.month = dig[4];
					time_val.days = dig[5] * 10 + dig[6];

					findDST();

					old_year = time_val.year;
				}

				if(dst_reg != 0)
				{
					if((i == 6) || (i == 7))
					{
						if((dig[4] == dst1.month) && ((dig[5] * 10 + dig[6]) == dst1.days))
						{
							if((dig[7] * 10 + dig[8]) == dst1.hours)
							{
								result_object.test_dig[8]++;

								dig_change_flag = 1;
							}
						}
					}
				}

				if(dig_change_flag == 1)
				{
					dig_change_flag = 0;

					for(uint8_t j = i; j < 11; j++)
					{
						dig[j] = result_object.test_dig[j];
					}
				}

				refreshSetDisplay(i);

				if(i == 4)
				{
					writeMonth(dig[4], 1);

					print(0, 1, 0);
				}

				break;
			}
		}
	}

	time_val.hours = dig[7] * 10 + dig[8];
	time_val.minutes = dig[9] * 10 + dig[10];
	time_val.seconds = 0;

	century = time_val.year / 100;

	setRTC();

	writeData();

	message_0[1] = '0' + dig[8] - 10;
	message_0[3] = '0' + dig[10];
	print(1, 0, 0);

	if(checkDST() == 1)
	{
		DST_set = 1;
	}
	else
	{
		DST_set = 0;
	}
	writeData();

	battery_ok = 1;

	set_flag = 1;
}
