#include "driverlib.h"

int main(void) {

    WDT_A_hold(WDT_A_BASE);

    //use Timer_A to blink LED, simplest test program
    //initialize IO output
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    while(1) {
        __delay_us(1000);
        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }

    return (0);
}
