/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "servo2040.hpp"

using namespace servo;

/* Input Defines */
#define SPECIAL_BYTE1	0x1b	
#define SPECIAL_BYTE2	0x5b
#define KEY_UP			0x41
#define KEY_RIGHT		0x43
#define KEY_DOWN		0x42
#define KEY_LEFT		0x44

/* PWM Defines */
#define MIN_PULSE_VALUE	500
#define MAX_PULSE_VALUE 2500

/* Create an array of servo pointers */
const int START_PIN = servo2040::SERVO_1; 	// Can be changed to only calibrate a 
const int END_PIN = servo2040::SERVO_18;	// a group of servos
const int NUM_SERVOS = (END_PIN - START_PIN) + 1;
ServoCluster servos = ServoCluster(pio0, 0, START_PIN, NUM_SERVOS);

/* PWM value storage */
int neg45_PWMvalues[NUM_SERVOS];
int pos45_PWMvalues[NUM_SERVOS];
int average_PWMvalues[NUM_SERVOS];

/* Helper variables */
char inputByte1;
char inputByte2;
char inputByte3;
uint currPWM;
bool calibState = 0; // 0:-45, 1:+45

int main() {
	stdio_init_all();
	
	/* Initialize the servo cluster */
	servos.init();
	
	while (!stdio_usb_connected()); // Wait for VCP/CDC connection;
	
	sleep_ms(2000);
	
	printf(	"\r\n Script by EddieArchuleta, special thanks to MYP for starting the hexapod community!\r\n\r\n"
			" Welcome to the rp2040 calibration script!\r\n\r\n"
		
			" This script will record and return the PWM values at \r\n"
			" -45 and +45 degrees for each servo. It will also return \r\n"
			" the average PWM of the values at the end of the script for \r\n"
			" calibration purposes.\r\n\r\n");
	
	printf(	" This script assumes all 18 servo channels are powered and attached.\r\n\r\n");

	printf(	" WARNING:\r\n"
			" If you want to run servos with a higher voltage than 5V, you'll\r\n"
			" need to cut the 'Separate USB and Ext. Power' trace on the back of\r\n"
			" the board to prevent the RP2040 being damaged by the increased voltage.\r\n\r\n");
	
	printf(" Press any key to continue...\r\n\r\n");
	getchar();

	printf(" Centering all servos...\r\n\r\n");
	/* Enable all servos (centers all servos) */
	for (auto currServo = START_PIN; currServo < NUM_SERVOS; currServo++) {
		printf(" Centering servo %d...\r\n", currServo + 1);
		servos.enable(currServo);
		sleep_ms(250); 	// Give each servo time to center to avoid
	}					// drawing too much current at once
	printf("\r\n");
	
	printf(	" Each servo will now be calibrated. The controls are as follows:\r\n"
			" Up Arrow:\t Fast Clockwise\r\n"
			" Right Arrow:\t Slow Clockwise\r\n\r\n"
			" Down Arrow:\t Fast Counter-Clockwise\r\n"
			" Left Arrow:\t Slow Counter-Clockwise\r\n\r\n"
			" Type the space bar to save the PWM as the calibrated value and continue.\r\n\r\n");	
	
	for (auto currServo = START_PIN; currServo < NUM_SERVOS;)
	{	
		if (!calibState) {
			printf(" **********************************************************************\r\n");
			printf("\r\n The -45 degree PWM for servo %d will now be calibrated\r\n", currServo + 1);
			printf(" Move the servo to -45 degrees...\r\n\r\n");
		}
		else {
			printf("\r\n The +45 degree PWM for servo %d will now be calibrated\r\n", currServo + 1);
			printf(" Move the servo to +45 degrees...\r\n\r\n");
		}
		
		while (inputByte1 != ' ')
		{
			inputByte1 = 0;
			inputByte2 = 0;
			inputByte3 = 0;
			currPWM = servos.pulse(currServo);
			printf(" PWM: %d\r\n", currPWM);		
			
			/* special KEY_ handling 
			   3 bytes returned: 0x1b, 0x5b, 0x__ */
			inputByte1 = getchar();
			if (inputByte1 == SPECIAL_BYTE1) {
				inputByte2 = getchar(); // Throw away SPECIAL_BYTE2
				inputByte3 = getchar(); // Arrow ID
				switch (inputByte3)
				{
				case KEY_UP:
					{
						currPWM -= 20;
						break;
					}
				case KEY_RIGHT:
					{
						currPWM -= 10;
						break;
					}
				case KEY_DOWN:
					{
						currPWM += 20;
						break;
					}
				case KEY_LEFT:
					{
						currPWM += 10;
						break;
					}
				default:
					printf("\r\n Oops! An invalid key was pressed, please try again.\r\n\r\n");
					break;
				} // switch (serialInput)
			}
			/* normal character handling */
			else if (inputByte1 != ' ') {
					printf("\r\n Oops! An invalid key was pressed, please try again.\r\n\r\n");
			}
			
			/* Input Hygiene */
			if (currPWM < MIN_PULSE_VALUE){
				currPWM = MIN_PULSE_VALUE;
				printf(" D'oh! The minimum PWM in the script is %d\r\n", MIN_PULSE_VALUE);
			}
			else if (currPWM > MAX_PULSE_VALUE){
				currPWM = MAX_PULSE_VALUE;
				printf(" D'oh! The minimum PWM in the script is %d\r\n", MAX_PULSE_VALUE);
			}
			
			servos.pulse(currServo, currPWM);
		} // while (serialInput != ' ')
		
		/* Exiting keyboard input loop */
		inputByte1 = 0;
		inputByte2 = 0;
		inputByte3 = 0;
		
		/* Binary state machine */
		if (!calibState) {
			calibState = 1;
			printf("\r\n The -45 degree PWM value for servo %d is saved as %d\r\n\r\n", 
					currServo + 1, currPWM);
			neg45_PWMvalues[currServo] = currPWM;
		}
		else {
			calibState = 0;
			printf("\r\n The +45 degree PWM value for servo %d is saved as %d\r\n\r\n", 
				   currServo + 1, currPWM);
			printf(" **********************************************************************\r\n");
			pos45_PWMvalues[currServo] = currPWM;
			currServo++; // -45/+45 for currServo calibrated at this point, move to next servo
		}
	} // for (auto currServo = START_PIN; currServo < END_PIN; currServo++)
	
	/* Take the average PWM value for each servo */
	for (auto currServo = START_PIN; currServo < NUM_SERVOS; currServo++)
	{
		average_PWMvalues[currServo] = 
			round((float(neg45_PWMvalues[currServo] + pos45_PWMvalues[currServo])/2));
	}
	
	/* Print results */
	printf(" **********************************************************************\r\n");
	printf(	" Calibration is now complete, don't forget to copy or take a screenshot!\r\n"
			" See results below: \r\n\r\n");
	for (auto currServo = START_PIN; currServo < NUM_SERVOS; currServo++)
	{
		printf(" Servo %d:\t -45 deg value: %d\t +45 deg value: %d\t Average Calibrated Value: %d\r\n",
				currServo + 1, neg45_PWMvalues[currServo], pos45_PWMvalues[currServo], average_PWMvalues[currServo]);
		sleep_ms(100); // Delay to give cool scrolling effect
	}
	printf(" \r\n**********************************************************************\r\n\r\n");

	printf(" Centering all servos with calibrated center value...\r\n\r\n");
	/* Center all servos with calibrated value */
	for (auto currServo = START_PIN; currServo < NUM_SERVOS; currServo++) {
		printf(" Centering servo %d...\r\n", currServo + 1);
		servos.pulse(currServo, average_PWMvalues[currServo]);
		sleep_ms(250); 	// Give each servo time to center to avoid
	}					// drawing too much current at once
	
	printf("\r\n");
	printf(" All servos are now centered with the calibrated average value found.\r\n"
		   " Please visually inspect that all servos are exactly centered at 90 degrees.\r\n"
		   " If not, calibration process should be repeated to find correct center PWM value\r\n");
	printf("\r\n");
	printf("\r\n");
	printf("\r\n");
	return 1;
}
	