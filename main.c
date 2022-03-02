#include "driverlib.h"

int main(void) {
    WDT_A_hold(WDT_A_BASE); //disable watchdog timer

    //use delay loop to blink LED as first test program

    //then use rotary encoder to control PWM pin as second test program. Test on LED

    return (0);
}
