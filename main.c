#include "driverlib.h"
#include "msp430.h"
#include "GPS.h"
#include "LCDLib.h"

int main(void) {

    WDT_A_hold(WDT_A_BASE);

    UCS_initClockSignal(UCS_FLLREF, UCS_REFOCLK_SELECT, UCS_CLOCK_DIVIDER_1);
    UCS_initFLLSettle(16777, 512);

    UCS_initClockSignal(UCS_SMCLK, UCS_DCOCLK_SELECT, UCS_CLOCK_DIVIDER_1);
    UCS_initClockSignal(UCS_MCLK, UCS_DCOCLK_SELECT, UCS_CLOCK_DIVIDER_1);
    UCS_initClockSignal(UCS_ACLK, UCS_VLOCLK_SELECT, UCS_CLOCK_DIVIDER_1); //set ACLK for 10kHz

    LCD_Init();
    LCD_Clear(ST77XX_RED);
    LCD_Text(1, 1, "April 8, 2022.", ST77XX_BLUE);
    LCD_Text(1, 20, "5:20pm",ST77XX_BLUE);
    LCD_Text(1, 30, "Hello, Blake", ST77XX_BLUE);

    GPS_Init();
    GPS_enable();

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
