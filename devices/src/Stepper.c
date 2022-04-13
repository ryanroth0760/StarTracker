/*
 * Stepper.c
 *
 *  Created on: Apr 13, 2022
 *      Author: troth
 */

#include "Stepper.h"
#include "driverlib.h"
#include "bmi160.h"
#include "UI.h"
#include "LCDLib.h"

static int16_t step1Count = 0;
static int16_t step2Count = 0;
static int16_t step3Count = 0;
static uint16_t step1Dir = 0;
static uint16_t step2Dir = 0;
static uint16_t step3Dir = 0;

static bool dataReady = false;
static bool flatAlign = false;

extern struct bmi160_dev bmi160config;


/*
 * Name: AlignSteppers
 * Purpose: Aligns camera mount to be parallel to ground and aligns north
 */
void AlignSteppers(void) {
    //Initialize relevant variables
    step1Count = 0;
    step2Count = 0;
    step3Count = 0;
    step1Dir = 0;
    step2Dir = 0;
    step3Dir = 0;
    dataReady = false;

    struct bmi160_int_pin_settg bmi_int_pin = {
                                        1,
                                        0,
                                        1,
                                        1,
                                        0,
                                        4
    };

    struct bmi160_int_settg bmi_accel_int = {
                                      BMI160_INT_CHANNEL_1,
                                      BMI160_ACC_GYRO_DATA_RDY_INT,
                                      bmi_int_pin,
                                      0, //union defines configuration for other interrupts
                                      0,
                                      0
    };


        /*! Structure configuring Interrupt pins */
        //struct bmi160_int_pin_settg int_pin_settg;

        /*! Union configures required interrupt */
        //union bmi160_int_type_cfg int_type_cfg;

        /*! FIFO FULL INT 1-enable, 0-disable */
        //uint8_t fifo_full_int_en : 1;

        /*! FIFO WTM INT 1-enable, 0-disable */
        //uint8_t fifo_wtm_int_en : 1;
    struct bmi160_sensor_data accel;
    struct bmi160_sensor_data gyro;



    //Initialize interrupts on INT1 and INT2
    bmi160_set_int_config(&bmi_accel_int, &bmi160config);
    GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN3 | GPIO_PIN4); //Enable interrupts on GPIO pins for INT1 and INT2

    while(!flatAlign) {
        //while(!dataReady); //wait for accelerometer data
        //read accelerometer data

        LCD_Clear(ST77XX_BLACK);

        bmi160_get_sensor_data(BMI160_ACCEL_ONLY, &accel, &gyro, &bmi160config);

        //write sensor data to lcd
        LCD_WriteInt(1, 1, accel.x, 0xFFFF);
        LCD_WriteInt(1, 21, accel.y, 0xFFFF);
        LCD_WriteInt(1, 41, accel.z, 0xFFFF);

        //delay 1 seconds?
        __delay_cycles(16777216);

        dataReady = false;
    }

}

#pragma vector=PORT2_VECTOR
__interrupt void P2_HANDLER(void) {
    //check which pin triggered interrupt
    if (P2IV & P2IV_P2IFG3) {
            dataReady = true;
    }
    //set data ready flag
}

extern bool button_f;
extern bool select_f;
extern RE_DIR select_dir;

void MoveSteppersManual(void) {
    //initialize IO
    GPIO_setOutputLowOnPin(STEP_PORT, STEP1_PWM | STEP2_PWM | STEP3_PWM | STEP1_DIR | STEP2_DIR | STEP3_DIR);
    GPIO_setAsOutputPin(STEP_PORT, STEP1_PWM | STEP2_PWM | STEP3_PWM | STEP1_DIR | STEP2_DIR | STEP3_DIR);

    uint16_t currMotor = 1;
    uint16_t currPWMPin = STEP1_PWM;
    uint16_t currDirPin = STEP1_DIR;

    //wait for update from rotary encoders
    while(1) {
        while(!button_f && !select_f);
            if (button_f) {
                currPWMPin <<= 1; //cycle motor
                currDirPin <<= 1;
                if (currPWMPin >= 0x10) {
                    currPWMPin = STEP1_PWM;
                    currDirPin = STEP1_DIR;
                }
                button_f = false;
            }
            else if (select_f) {
                if (select_dir == LEFT) {
                    GPIO_setOutputLowOnPin(STEP_PORT, currDirPin);
                    step1Count--;
                }
                else {
                    GPIO_setOutputHighOnPin(STEP_PORT, currDirPin);
                    step1Count++;
                }

                if (step1Count > 500) { //fixme: temp, hang forever if issue occurs
                    while(1);
                }

                for (uint16_t i = 0; i < 10; i++) {
                GPIO_setOutputHighOnPin(STEP_PORT, currPWMPin);
                __delay_cycles(20000);
                GPIO_setOutputLowOnPin(STEP_PORT, currPWMPin);
                select_f = false;
                __delay_cycles(20000);
                }

        }
    }
}



