/*
 * MAX6920.h
 *
 *  Created on: Nov 06, 2016
 *      Author: Vlad
 */

#ifndef MAX6920_H_
#define MAX6920_H_

void initializeDisplay(uint8_t display_size, uint32_t filament_freq, uint32_t scan_freq, uint16_t *screen);
void displayOn(void);
void displayOff(void);
void setBrightness(uint8_t new_brightness);

#endif /* MAX6920_H_ */
