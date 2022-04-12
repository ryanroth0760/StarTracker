/*
 * bmx160_msp430.c
 *
 *      Author: troth
 */

#include "bmx160_msp430.h"
#include "bmi160.h"
#include "driverlib.h"

/****** PRIVATE FUNCTION PROTOTYPES ******/

struct bmi160_dev bmi160config;

int8_t bmi_i2c_write(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
int8_t bmi_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
void bmi_delay_ms(uint32_t period, void *intf_ptr);

void I2C_Init(void);

void delay_Init(void);

/****** PRIVATE FUNCTION PROTOTYPES ******/

/****** PUBLIC FUNCTION DEFINITIONS ******/

/*
 * Name: init_bmi160(void)
 * Purpose: initializes I2C, configures bmi160 dev struct, and then calls bmi160 API functions to configure the device
 */
void BMI160_Init(void) {
    I2C_Init();
    delay_Init(); //initialize timer b for use in the delay function

    bmi160config.write = bmi_i2c_write;
    bmi160config.read = bmi_i2c_read;
    bmi160config.delay_ms = bmi_delay_ms;
    bmi160config.id = BMI160_I2C_ADDR;
    bmi160config.intf = BMI160_I2C_INTF;

}

/****** PUBLIC FUNCTION DEFINITIONS ******/

/****** PRIVATE FUNCTION DEFINITIONS ******/

int8_t bmi_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

    /*
     * The parameter intf_ptr can be used as a variable to store the I2C address of the device
     */

    /*
     * Data on the bus should be like
     * |------------+---------------------|
     * | I2C action | Data                |
     * |------------+---------------------|
     * | Start      | -                   |
     * | Write      | (reg_addr)          |
     * | Stop       | -                   |
     * | Start      | -                   |
     * | Read       | (reg_data[0])       |
     * | Read       | (....)              |
     * | Read       | (reg_data[len - 1]) |
     * | Stop       | -                   |
     * |------------+---------------------|
     */

    return rslt;
}

int8_t bmi_i2c_write(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

    /*
     * The parameter intf_ptr can be used as a variable to store the I2C address of the device
     */

    /*
     * Data on the bus should be like
     * |------------+---------------------|
     * | I2C action | Data                |
     * |------------+---------------------|
     * | Start      | -                   |
     * | Write      | (reg_addr)          |
     * | Write      | (reg_data[0])       |
     * | Write      | (....)              |
     * | Write      | (reg_data[len - 1]) |
     * | Stop       | -                   |
     * |------------+---------------------|
     */

    return rslt;
}

void bmi_delay_ms(uint32_t period, void *intf_ptr) {
    //Timer_B_setCompareValue(TIMER_B0_BASE, TIMER_B_CAPTURECOMPARE_REGISTER_0, ((uint16_t) period) * 10 - 1);//start counter on timer B, set CCR0 to period*10 - 1
    TB0R = ((uint16_t) period) * 10 - 1;
    Timer_B_startCounter(TIMER_B0_BASE, TIMER_B_CONTINUOUS_MODE);
    while(!(TB0CTL & TBIFG)); //wait until overflow flag is set
    TB0CTL &= ~TBIFG; //clear overflow flag
    Timer_B_stop(TIMER_B0_BASE); //stop timer
}

void I2C_Init(void) {
    USCI_B_I2C_initMasterParam = { //struct to initialize I2C module
                                   USCI_B_I2C_CLOCKSOURCE_SMCLK,
                                   16777216,
                                   USCI_B_I2C_SET_DATA_RATE_400KBPS
    };

    USCI_B_I2C_initMaster(USCI_B0_BASE, &USCI_B_I2C_initMasterParam);

    //set up relevant pins
#ifdef __MSP430F5529__
    P3SEL |= BIT0 | BIT1; //select alternate functions on pins 3.0 and 3.1
    GPIO_setAsInputPin(GPIO_PORT_P1, GPIO_PIN3); //configure pins for interrupts, 2.3 is INT1, 2.4 is INT2
    GPIO_setAsInputPin(GPIO_PORT_P1, GPIO_PIN4);
    GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN3 | GPIO_PIN4, GPIO_LOW_TO_HIGH_TRANSITION);//configure pins for interrupts

#elif defined __MSP430F6438__
    const uint8_t portmapping[] = {
                                   PM_NONE,//use remap controller to select P2.1 as SDA and P2.2 as SCL
                                   PM_UCB0SDA,
                                   PM_UCB0SCL,
                                   PM_NONE,
                                   PM_NONE,
                                   PM_NONE,
                                   PM_NONE,
                                   PM_NONE
    };
    PMAP_initPortsParam initPmapParams;
    initPmapParams.portMapping = portmapping;
    initPmapParams.PxMAPy = (uint8_t*) &P2MAP01;
    initPmapParam.numberOfPorts = 1;
    initPmapParam.portMapReconfigure = PMAP_ENABLE_RECONFIGURATION;
    PMAP_initPorts(PMAP_CTRL_BASE, &initPmapParam);

    GPIO_setAsInputPin(GPIO_PORT_P2, GPIO_PIN3); //configure pins for interrupts, 2.3 is INT1, 2.4 is INT2
    GPIO_setAsInputPin(GPIO_PORT_P2, GPIO_PIN4);
    GPIO_selectInterruptEdge(GPIO_PORT_P2, GPIO_PIN3 | GPIO_PIN4, GPIO_LOW_TO_HIGH_TRANSITION);
#endif

    USCI_B_I2C_enable(USCI_B0_BASE);
}

void delay_Init(void) {
    Timer_B_initContinuousModeParam TBInit_Param = {
                                            TIMER_B_CLOCKSOURCE_ACLK,
                                            TIMER_B_CLOCKSOURCE_DIVIDER_1,
                                            TIMER_B_TBIE_INTERRUPT_DISABLE,
                                            TIMER_B_DO_CLEAR,
                                            false //do not start timer immediately
    };
    Timer_B_initContinuousMode(TIMER_B0_BASE, &TBInit_Param);
}

/****** PRIVATE FUNCTION DEFINITIONS ******/




