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

int8_t bmi_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
int8_t bmi_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
void bmi_delay_ms(uint32_t period);

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
    bmi160config.id = 0; //FIXME: I2C address find on datasheet
    bmi160config.intf = 0;

    bmi160_init(&bmi160config);

    /* Select the Output data rate, range of accelerometer sensor */
    bmi160config.accel_cfg.odr = BMI160_ACCEL_ODR_1600HZ;
    bmi160config.accel_cfg.range = BMI160_ACCEL_RANGE_16G;
    bmi160config.accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;

    /* Select the power mode of accelerometer sensor */
    bmi160config.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

    /* Select the Output data rate, range of Gyroscope sensor */
    bmi160config.gyro_cfg.odr = BMI160_GYRO_ODR_3200HZ;
    bmi160config.gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS;
    bmi160config.gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;

    /* Select the power mode of Gyroscope sensor */
    bmi160config.gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;

        /* Set the sensor configuration */
    bmi160_set_sens_conf(&bmi160config);

}

/****** PUBLIC FUNCTION DEFINITIONS ******/

/****** PRIVATE FUNCTION DEFINITIONS ******/




int8_t bmi_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_MODE); //send start condition in transmit mode
    //USCI_B_I2C_masterSendStart(USCI_B0_BASE);
    //while(USCI_B_I2C_masterIsStartSent(USCI_B0_BASE) == USCI_B_I2C_SENDING_START);
    //write register address
    USCI_B_I2C_masterSendMultiByteStart(USCI_B0_BASE, reg_addr);

    while (!(HWREG8(USCI_B0_BASE + OFS_UCBxIFG) & UCTXIFG)); //poll transmit flag before sending restart condition

    //USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_RECEIVE_MODE);
    USCI_B_I2C_masterReceiveMultiByteStart(USCI_B0_BASE); //send restart condition

    for (uint32_t i = 0; i < len; i++) {
        while (!(HWREG8(USCI_B0_BASE + OFS_UCBxIFG) & UCRXIFG)); //wait until byte is received
        if (i == len-1) {
            reg_data[i] = USCI_B_I2C_masterReceiveMultiByteFinish(USCI_B0_BASE); //stop condition
        }
        else {
            reg_data[i] = USCI_B_I2C_masterReceiveMultiByteNext(USCI_B0_BASE);
        }

    }

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

    return 0;
}

int8_t bmi_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    //int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

    USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_MODE); //set mode to transmit

    USCI_B_I2C_masterSendMultiByteStart(USCI_B0_BASE, reg_addr); //send start condition

    while (!(HWREG8(USCI_B0_BASE + OFS_UCBxIFG) & UCTXIFG));

    if (len > 1) {
        for (uint32_t i = 0; i < len; i++) {
            if (i == len-1) {
                USCI_B_I2C_masterSendMultiByteFinish(USCI_B0_BASE, reg_data);
            }
            else {
                USCI_B_I2C_masterSendMultiByteNext (USCI_B0_BASE, reg_data); //send byte
            }
        }
    }
    else {
        USCI_B_I2C_masterSendMultiByteStop(USCI_B0_BASE); //send stop condition
    }

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

    return 0;
}

void bmi_delay_ms(uint32_t period) {
    //Timer_B_setCompareValue(TIMER_B0_BASE, TIMER_B_CAPTURECOMPARE_REGISTER_0, ((uint16_t) period) * 10 - 1);//start counter on timer B, set CCR0 to period*10 - 1
    TB0R = ((uint16_t) period) * 10 - 1;
    Timer_B_startCounter(TIMER_B0_BASE, TIMER_B_CONTINUOUS_MODE);
    while(!(TB0CTL & TBIFG)); //wait until overflow flag is set
    TB0CTL &= ~TBIFG; //clear overflow flag
    Timer_B_stop(TIMER_B0_BASE); //stop timer
}

void I2C_Init(void) {
    USCI_B_I2C_initMasterParam initI2C = { //struct to initialize I2C module
                                   USCI_B_I2C_CLOCKSOURCE_SMCLK,
                                   16777216,
                                   USCI_B_I2C_SET_DATA_RATE_400KBPS
    };

    USCI_B_I2C_initMaster(USCI_B0_BASE, &initI2C);

    USCI_B_I2C_setSlaveAddress(USCI_B0_BASE, BMI160_I2C_ADDR);
    USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_MODE);

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




