#include "driverlib.h"
#include "msp430.h"
#include "GPS.h"
#include "LCDLib.h"
#include "UI.h"
#include "bmx160_msp430.h"
#include "Stepper.h"
#include "Speaker.h"

int main(void) {

    WDT_A_hold(WDT_A_BASE);

    UCS_initClockSignal(UCS_FLLREF, UCS_REFOCLK_SELECT, UCS_CLOCK_DIVIDER_1);
    UCS_initFLLSettle(16777, 512);

    UCS_initClockSignal(UCS_SMCLK, UCS_DCOCLK_SELECT, UCS_CLOCK_DIVIDER_1);
    UCS_initClockSignal(UCS_MCLK, UCS_DCOCLK_SELECT, UCS_CLOCK_DIVIDER_1);
    UCS_initClockSignal(UCS_ACLK, UCS_VLOCLK_SELECT, UCS_CLOCK_DIVIDER_1); //set ACLK for 10kHz

    UI_Init();
    BMI160_Init();
    LCD_Clear(bgColor);
    //LCD_Text(1, 1, "April 12, 2022.", ST77XX_BLUE);
    //LCD_Text(1, 30, "Hi", ST77XX_BLUE);
    Stepper_Init();
    Speaker_Init();

    __enable_interrupt();
    uint8_t selection = 0xFF;

    while(1) {
        selection = 0xFF;
        //Show welcome message, press button to continue
        while(selection != 1) {
            LCD_Clear(bgColor);
            LCD_Text(1, 1, "Welcome to", textColor);
            LCD_Text(1, 15, "StarTracker", textColor);
            selection = UI_DisplayMenu("Align Flat", "Change Color", "Beep", "Manual Adjust", 4);

            switch(selection) {
            case 1:
                AlignSteppers();
                AxisAlign();
                break;
            case 2:
                UI_SetColor();
                break;
            case 3:
                Speaker_Beep();
                __delay_cycles(9000000);
                Speaker_Beep();
                break;
            case 4:
                MoveSteppersManual();
                break;
            }
        }

        Speaker_Beep();
        __delay_cycles(9000000);
        Speaker_Beep();

        selection = 0xFF;
        while (selection != 1) {
            LCD_Clear(bgColor);
            LCD_Text(1, 1, "StarTracker Ready", textColor);
            selection = UI_DisplayMenu("Track", "Realign",  "Manual Adjust", 0, 3);
            switch (selection) {
            case 1:
                Stepper_Track();
                break;
            case 2:
                AlignSteppers();
                AxisAlign();
                break;
            case 3:
                MoveSteppersManual();
                break;
            }
        }


    }

    selection = 0xFF;

    char test[4][10] = {"Option 1.", "Option 2.", "Option 3.", "Option 4."};
    LCD_Text(1, 1, test[0], textColor);
    //test[0] = ;
    //test[1] = "Option 2.";
    //test[2] = "Option 3.";
    //test[3] = "Option 4.";

    MoveSteppersManual();

    LCD_Clear(bgColor);
    LCD_Text(1, 1, "Manual Aligned", textColor);
    while(1);

    selection = UI_DisplayMenu(test[0], test[1], test[2], test[3], 4);

    LCD_Clear(bgColor);

    switch(selection) {
    case 1:
        LCD_Text(1, 1, "Op 1 Selected", textColor);
        break;
    case 2:
        LCD_Text(1, 1, "Op 2 Selected", textColor);
        break;
    case 3:
        LCD_Text(1, 1, "Op 3 Selected", textColor);
        break;
    case 4:
        LCD_Text(1, 1, "Op 4 Selected", textColor);
    }

    while(1) {
        Speaker_Beep();
        __delay_cycles(32000000 / 4);
    }



    //MoveSteppersManual();

    //AlignSteppers();
    //AxisAlign();
    //AlignNorth();

    //GPS_Init();
    //GPS_enable();

    //Write code to read from GPS and display results on LCD

    //LCD_SetPoint(80, 80, ST77XX_BLUE);
    //LCD_SetPoint(80, 10, ST77XX_BLUE);

    //volatile uint32_t hialice = UCS_getMCLK();
    //hialice = UCS_getSMCLK();

    while(1);

    //initialize IO output
    //GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    //GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    //while(1) {
    //    __delay_cycles(100000);
    //    GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    //}

    return (0);
}
