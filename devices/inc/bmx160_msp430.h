/*
 * bmx160_msp430.h
 *
 *      Author: troth
 */

#ifndef DEVICES_INC_BMX160_MSP430_H_
#define DEVICES_INC_BMX160_MSP430_H_

/*
 * Name: init_bmi160(void)
 * Purpose: initializes I2C, configures bmi160 dev struct, and then calls bmi160 API functions to configure the device
 */
void BMI160_Init(void);

#endif /* DEVICES_INC_BMX160_MSP430_H_ */
