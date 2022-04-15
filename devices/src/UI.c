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
bool switch_mode_f = false; //set to true when button 1 is pushed while manually aligning motors
bool select_f = false; //set to true when rotary encoder is turned
RE_DIR select_dir;

#define UI_REGION1_XSTART   1
#define UI_REGION1_XEND     127
#define UI_REGION1_YSTART   41
#define UI_REGION1_YEND     61
#define UI_REGION2_XSTART   UI_REGION1_XSTART
#define UI_REGION2_XEND     UI_REGION1_XEND
#define UI_REGION2_YSTART   UI_REGION1_YSTART + 21
#define UI_REGION2_YEND     UI_REGION1_YEND + 21
#define UI_REGION3_XSTART   UI_REGION2_XSTART
#define UI_REGION3_XEND     UI_REGION2_XEND
#define UI_REGION3_YSTART   UI_REGION2_YSTART + 21
#define UI_REGION3_YEND     UI_REGION2_YEND + 21
#define UI_REGION4_XSTART   UI_REGION3_XSTART
#define UI_REGION4_XEND     UI_REGION3_XEND
#define UI_REGION4_YSTART   UI_REGION3_YSTART + 21
#define UI_REGION4_YEND     UI_REGION3_YEND + 21

#define UI_SELECT_XOFFSET   10    //X offset from Region XEND
#define UI_SELECT_YOFFSET   5    //Y offset from Region YStart
#define UI_SELECT_SIZE      6   //dimensions of the select square in pixels

extern bool manualAlign;

/*
 * Purpose: Call LCD's initialization function and initialize P3.2-3.7 for use with rotary encoders.
 *          Falling edge interrupts on pins 3 and 6
 */
void UI_Init(void) {
    LCD_Init();
    bgColor = ST77XX_RED;
    textColor = ST77XX_BLUE;

    GPIO_setAsInputPin(RE_PORT, RE1_A | RE1_B | RE1_PB | RE2_A | RE2_B | RE2_PB);
    GPIO_selectInterruptEdge(RE_PORT, RE1_B | RE2_B | RE2_PB | RE1_PB, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_enableInterrupt(RE_PORT, RE1_B | RE2_B | RE2_PB);

    //FIXME: probably some more initialization down here, figure out later
}

/*
 * Name: UI_DisplayMenu
 * Purpose: Displays a Menu with parameters passed in. Returns region selected as a number. numOptions should be between 1 and 4
 */
uint8_t UI_DisplayMenu(char* option1, char* option2, char* option3, char* option4, uint8_t numOptions) {
    int8_t currRegion = 1;
    int8_t prevRegion = 1;
    LCD_DrawRectangle(1, 127, 41, 120, bgColor);
    LCD_DrawRectangle(UI_REGION1_XEND - UI_SELECT_XOFFSET, UI_REGION1_XEND - UI_SELECT_XOFFSET + UI_SELECT_SIZE,
                                                  UI_REGION1_YSTART + (21 * (currRegion-1)) + UI_SELECT_YOFFSET, UI_REGION1_YSTART + (21 * (currRegion-1)) + UI_SELECT_YOFFSET + UI_SELECT_SIZE,
                                                  textColor);
    for (uint16_t i = 0; i < numOptions; i++) {
        if (i == 0) {
            LCD_Text(1, 41 + (21 * i), option1, textColor);
        }
        else if (i == 1) {
            LCD_Text(1, 41 + (21 * i), option2, textColor); //print options
        }
        else if (i == 2) {
            LCD_Text(1, 41 + (21 * i), option3, textColor);
        }
        else if (i == 3) {
            LCD_Text(1, 41 + (21 * i), option4, textColor);
        }
    }

    button_f = false;
    select_f = false;

    //wait for input
    while(!button_f) {
        while(!select_f && !button_f); //wait for selection (or button)
        if (select_f) {
            prevRegion = currRegion;
            if (prevRegion > 0) {//clear previous region's selection if applicable
                LCD_DrawRectangle(UI_REGION1_XEND - UI_SELECT_XOFFSET, UI_REGION1_XEND - UI_SELECT_XOFFSET + UI_SELECT_SIZE,
                                  UI_REGION1_YSTART + (21 * (prevRegion-1)) + UI_SELECT_YOFFSET, UI_REGION1_YSTART + (21 * (prevRegion-1)) + UI_SELECT_YOFFSET + UI_SELECT_SIZE,
                                  bgColor);
            }

            if (select_dir == LEFT) {
                currRegion--;
                if (currRegion < 1) {
                    currRegion = numOptions;
                }
            }
            else { //select_dir == RIGHT
                currRegion++;
                if (currRegion > numOptions) {
                    currRegion = 1;
                }
            }
            LCD_DrawRectangle(UI_REGION1_XEND - UI_SELECT_XOFFSET, UI_REGION1_XEND - UI_SELECT_XOFFSET + UI_SELECT_SIZE,
                                              UI_REGION1_YSTART + (21 * (currRegion-1)) + UI_SELECT_YOFFSET, UI_REGION1_YSTART + (21 * (currRegion-1)) + UI_SELECT_YOFFSET + UI_SELECT_SIZE,
                                              textColor); //draw select marker for currRegion

            select_f = false;
        }
    }

    button_f = false;

    return currRegion;
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
    if (P3IFG & 0x10) {
        manualAlign = false; //RE1 PB pushed while aligning
        P3IFG &= ~0x10;
    }
}

/*
 * UI_SetColor
 * Purpose: Sets colors for LCD menu between 4 presets
 */
void UI_SetColor(void) {
    uint8_t selection = UI_DisplayMenu("Orange & Blue", "Black & White", "Red & White", "Red & Black", 4);
    switch(selection) {
    case 1:
        bgColor = ST77XX_RED;
        textColor = ST77XX_BLUE;
        break;
    case 2:
        bgColor = 0x0000;
        textColor = 0xFFFF;
        break;
    case 3:
        bgColor = 0x001F;
        textColor = 0xFFFF;
        break;
    case 4:
        bgColor = 0x0000;
        textColor = 0x001F;
        break;
    }
}

/****** INTERRUPTS ******/

