/*
 * UI.h
 *
 *      Author: troth
 */

#ifndef DEVICES_INC_UI_H_
#define DEVICES_INC_UI_H_

#include "LCDLib.h"
#include "driverlib.h"
#include "msp430.h"

/****** DEFINITIONS ******/
#define RE_PORT     GPIO_PORT_P3
#define RE1_A       GPIO_PIN2
#define RE1_B       GPIO_PIN3
#define RE1_PB      GPIO_PIN4
#define RE2_A       GPIO_PIN5
#define RE2_B       GPIO_PIN6
#define RE2_PB      GPIO_PIN7

typedef enum _RE_DIR {
                             LEFT = 0,
                             RIGHT = 1
} RE_DIR;

/****** DEFINITIONS ******/

/****** PUBLIC FUNCTION PROTOTYPES ******/

/*
 * Name: UI_DisplayMenu
 * Purpose: Displays a Menu with parameters passed in. Returns region selected as a number. numOptions should be between 1 and 4
 */
uint8_t UI_DisplayMenu(const char** options, uint8_t numOptions);

/*
 * Purpose: Call LCD's initialization function and initialize P3.2-3.7 for use with rotary encoders.
 *          Falling edge interrupts on pins 3 and 6
 */
void UI_Init(void);

/****** PUBLIC FUNCTION PROTOTYPES ******/

#endif /* DEVICES_INC_UI_H_ */
