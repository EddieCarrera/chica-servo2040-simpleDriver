#pragma once

#include <stdio.h>
#include <cstring>
#include "pico/stdlib.h"
#include "servo2040.hpp"
#include "analogmux.hpp"
#include "analog.hpp"
#include "button.hpp"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Commands */
#define SET_CMD	0xD3 // 0x53 & 0x80
#define GET_CMD	0xC7 // 0x47 & 0x80

/* A0/A1/A2 Mapping */
#define A0_GPIO_PIN			26
#define A1_GPIO_PIN			27
#define A2_GPIO_PIN			28
#define A0_GPIO_MASK		(1<<A0_GPIO_PIN)
#define A1_GPIO_MASK		(1<<A1_GPIO_PIN)
#define A3_GPIO_MASK		(1<<A2_GPIO_PIN)
#define GPIO_OUTPUT_MASK	0xFFFFFFFF
#define GPIO_INPUT_MASK		0x00
#define GPIO_HIGH_MASK		0xFFFFFFFF
#define GPIO_LOW_MASK		0x00

/* Miscellaneous */
#define MAX_COUNT_VALUE		127

/*******************************************************************************
 * Constants
 ******************************************************************************/
/* Timing */
constexpr uint GETC_TIMEOUT_US	= 100; // 10bits/115200bps = 86.8us acquire time

/* LED */
constexpr float BRIGHTNESS		= 0.3f;		// Normalized

/* Ratios */
constexpr float b1024_3_3V_RATIO	= 310.3f;
constexpr float b1024_5V_RATIO		= 204.8f;
constexpr float CURR_LSb			= 0.0814f;

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
typedef enum {
	SERVO1, SERVO2, SERVO3, SERVO4, SERVO5, SERVO6, 
	SERVO7, SERVO8, SERVO9, SERVO10, SERVO11, SERVO12, 
	SERVO13, SERVO14, SERVO15, SERVO16, SERVO17, SERVO18,
	TS1, TS2, TS3, TS4, TS5, TS6, 
	CURR, VOLT, RELAY, A1, A2, cmdPin_num
} cmdPins;

typedef enum {
	set,
	get
} hexapodCmds;

/*******************************************************************************
 * Structures
 ******************************************************************************/
typedef struct {
	hexapodCmds cmd;
	uint startIdx;
	uint count;
	uint valueBuff[MAX_COUNT_VALUE];
} cmdPkt;

/*******************************************************************************
 * Lookup Tables
 ******************************************************************************/
constexpr uint RP_hardwarePins_table[] = 
{
	SERVO1,		SERVO2,		SERVO3,			
	SERVO4,		SERVO5,		SERVO6,			
	SERVO7,		SERVO8,		SERVO9, 		
	SERVO10,	SERVO11,	SERVO12,		
	SERVO13,	SERVO14,	SERVO15, 		
	SERVO16,	SERVO17,	SERVO18,		
	servo::servo2040::SENSOR_1_ADDR,		// TS_L1
	servo::servo2040::SENSOR_2_ADDR,		// TS_L2
	servo::servo2040::SENSOR_3_ADDR,		// TS_L3
	servo::servo2040::SENSOR_4_ADDR,		// TS_R1
	servo::servo2040::SENSOR_5_ADDR,		// TS_R2
	servo::servo2040::SENSOR_6_ADDR,		// TS_R3
	servo::servo2040::CURRENT_SENSE_ADDR,	// CURR
	servo::servo2040::VOLTAGE_SENSE_ADDR,	// VOLT
	A0_GPIO_PIN,							// RELAY
	A1_GPIO_PIN,							// A1
	A2_GPIO_PIN								// A2
};

/*******************************************************************************
 * Function Forward Declarations
 ******************************************************************************/

/*******************************************************************************
 * Core Functions
 ******************************************************************************/
void parse_and_command_task(
void
);

/*******************************************************************************
 * VCP/Parsing Support Functions
 ******************************************************************************/
uint cmdPin_to_hardwarePin(
cmdPins cmdPin
);

void vcp_transmit(
uint *txbuff,
uint size
);

/*******************************************************************************
 * LED Support Functions
 ******************************************************************************/
void pendingVCP_ledSequence(
void
);

/*******************************************************************************
 * Sensing Support Functions
 ******************************************************************************/
float read_current(
void
);

float read_voltage(
void
);

float read_analogPin(
uint sensorAddress
);