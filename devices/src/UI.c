/*
 * UI.c
 *
 *      Author: troth
 */

#include "driverlib.h"
#include "UI.h"


//typedef enum UI_REGION = {
//
//} UI_REGION;

bool button_f = false; //set to true when rotary encoder button is pushed
bool select_f = false; //set to true when rotary encoder is turned
RE_DIR select_dir;

#define UI_REGION1_XSTART   10
#define UI_REGION1_XEND     10
#define UI_REGION1_YSTART   10
#define UI_REGION1_YEND     10
#define UI_REGION2_XSTART   10
#define UI_REGION2_XEND     10
#define UI_REGION2_YSTART   10
#define UI_REGION2_YEND     10
#define UI_REGION3_XSTART   10
#define UI_REGION3_XEND     10
#define UI_REGION3_YSTART   10
#define UI_REGION3_YEND     10
#define UI_REGION4_XSTART   10
#define UI_REGION4_XEND     10
#define UI_REGION4_YSTART   10
#define UI_REGION4_YEND     10

#define UI_SELECT_XOFFSET   10    //X offset from Region XEND
#define UI_SELECT_YOFFSET   5    //Y offset from Region YStart
#define UI_SELECT_SIZE      6   //dimensions of the select square in pixels

/*
 * Purpose: Call LCD's initialization function and initialize P3.2-3.7 for use with rotary encoders.
 *          Falling edge interrupts on pins 3 and 6
 */
void UI_Init(void) {
    LCD_Init();

    GPIO_setAsInputPin(RE_PORT, RE1_A | RE1_B | RE1_PB | RE2_A | RE2_B | RE2_PB);
    GPIO_selectInterruptEdge(RE_PORT, RE1_B | RE2_B | RE2_PB, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_enableInterrupt(RE_PORT, RE1_B | RE2_B | RE2_PB);

    //FIXME: probably some more initialization down here, figure out later
}

/*
 * Name: UI_DisplayMenu
 * Purpose: Displays a Menu with parameters passed in. Returns region selected as a number. numOptions should be between 1 and 4
 */
uint8_t UI_DisplayMenu(const char** options, uint8_t numOptions) {
    return 1; //FIXME: finish later
}

/****** INTERRUPTS ******/

#pragma vector=PORT3_VECTOR
__interrupt void P3_HANDLER(void) {
    if (P3IFG & 0x8) { //check port 3 interrupt flags
        if (P3IN & GPIO_PIN2) { //RE1 B falling edge, adjust brightness
            LCD_incBacklight(); //right turn, increase brightness
        }
        else {
            LCD_decBacklight(); //left turn, decrease brightness
        }
        P3IFG &= ~0x8;
    }
    //FIXME: temp code for motor rotary encoder testing
    if (P3IFG & 0x40) {
        if (P3IN & GPIO_PIN5) {
            select_dir = RIGHT;
        }
        else {
            select_dir = LEFT;
        }
        select_f = true; //RE2 B falling edge, adjust selection
        P3IFG &= ~0x40;
    }
    if (P3IFG & 0x80) {
        button_f = true; //RE2 PB pushed, make selection
        P3IFG &= ~0x80;
    }
}

/****** INTERRUPTS ******/

