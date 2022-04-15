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

static int16_t stepCount = 0;
static int16_t step2Count = 0;
static int16_t step3Count = 0;
static uint16_t step1Dir = 0;
static uint16_t step2Dir = 0;
static uint16_t step3Dir = 0;

bool manualAlign = false;
static bool dataReady = false;
static bool stepAligning = false;
static uint16_t currStep = STEP1_PWM;

extern struct bmi160_dev bmi160config;


/*
 * Name: AlignSteppers
 * Purpose: Aligns camera mount to be parallel to ground and aligns north
 */
/*
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

        //uint8_t fifo_wtm_int_en : 1;
    struct bmi160_sensor_data accel;
    struct bmi160_sensor_data gyro;

    //Initialize interrupts on INT1 and INT2
    volatile int8_t result = bmi160_set_int_config(&bmi_accel_int, &bmi160config);
    GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN3 | GPIO_PIN4); //Enable interrupts on GPIO pins for INT1 and INT2

    while(!flatAlign) {
        while(!dataReady); //wait for accelerometer data
        //read accelerometer data

        LCD_Clear(ST77XX_BLACK);

        result = bmi160_get_sensor_data(BMI160_ACCEL_ONLY, &accel, &gyro, &bmi160config);
        accel.x = ((accel.x & 0x00FF) << 8) | ((accel.x & 0xFF00) >> 8);
        accel.y = ((accel.y & 0x00FF) << 8) | ((accel.y & 0xFF00) >> 8);
        accel.z = ((accel.z & 0x00FF) << 8) | ((accel.z & 0xFF00) >> 8);

        //write sensor data to lcd
        LCD_WriteInt(1, 1, accel.x, 0xFFFF);
        LCD_WriteInt(1, 21, accel.y, 0xFFFF);
        LCD_WriteInt(1, 41, accel.z, 0xFFFF);

        //delay 1 seconds?
        __delay_cycles(16777216);

        dataReady = false;
    }

}
*/

/*
 * Name: AlignSteppers
 * Purpose: Aligns camera mount to be parallel to ground and aligns north
 */
void AlignSteppers(void) {
    //Initialize relevant variables
    stepCount = 0;
    step2Count = 0;
    step3Count = 0;
    step1Dir = 0;
    step2Dir = 0;
    step3Dir = 0;
    dataReady = false;
    stepAligning = false;
    bool flatAlign = false;

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


    struct bmi160_sensor_data accel;
    struct bmi160_sensor_data gyro;

    //Initialize interrupts on INT1 and INT2
    volatile int8_t result = bmi160_set_int_config(&bmi_accel_int, &bmi160config);
    GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN3 | GPIO_PIN4); //Enable interrupts on GPIO pins for INT1 and INT2

    bool step1Align = false;
    bool step2Align = false;

    LCD_Clear(0x0000);
    LCD_Text(1, 1, "ALIGNING MOTORS", 0xFFFF);

    while(!flatAlign) {
        while(!dataReady); //wait for accelerometer data
        //read accelerometer data

        //LCD_Clear(ST77XX_BLACK);
        //disable interrupt on accelerometer pin

        GPIO_disableInterrupt(GPIO_PORT_P2, GPIO_PIN3 | GPIO_PIN4);

        result = bmi160_get_sensor_data(BMI160_ACCEL_ONLY, &accel, &gyro, &bmi160config);
        accel.x = ((accel.x & 0x00FF) << 8) | ((accel.x & 0xFF00) >> 8);
        accel.y = ((accel.y & 0x00FF) << 8) | ((accel.y & 0xFF00) >> 8);
        accel.z = ((accel.z & 0x00FF) << 8) | ((accel.z & 0xFF00) >> 8);

        //write sensor data to lcd
        //LCD_WriteInt(1, 1, accel.x, 0xFFFF);
        //LCD_WriteInt(1, 21, accel.y, 0xFFFF);
        //LCD_WriteInt(1, 41, accel.z, 0xFFFF);

        if (!step1Align) { //initilize step1 IO
            if (accel.z < -1000) { //placeholder value
                GPIO_setOutputHighOnPin(STEP_PORT, STEP2_DIR);
            }
            else if (accel.z > 1000) {
                GPIO_setOutputLowOnPin(STEP_PORT, STEP2_DIR);
            }
            else {
                step1Align = true;
                LCD_Text(1, 21, "STEP 1 ALIGNED.", 0xFFFF);
                continue;
            }

            currStep = STEP2_PWM;

            //initialize stepCount based on how close Z is to 0
            stepCount = (abs(accel.z) >> 4) + 10;

            //work on aligning step 1
            //start timer
            stepAligning = true;
            Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
            Timer_A_enableInterrupt(TIMER_A0_BASE);
            //enable timer interrupts
            while(stepAligning); //run PWM for a certain number of counts

            Timer_A_stop(TIMER_A0_BASE);
            Timer_A_disableInterrupt(TIMER_A0_BASE); //stop timer
        }

        else if (!step2Align) {
            //work on aligning step 2
            if (accel.y < -1000) { //placeholder value
                GPIO_setOutputLowOnPin(STEP_PORT, STEP1_DIR);
            }
            else if (accel.y > 1000) {
                GPIO_setOutputHighOnPin(STEP_PORT, STEP1_DIR);
            }
            else {
                step2Align = true;
                LCD_Text(1, 41, "STEP 2 ALIGNED.", 0xFFFF);
                flatAlign = true;
                continue;
            }

            currStep = STEP1_PWM;

            //initialize stepCount based on how close Z is to 0
            stepCount = (abs(accel.y) >> 4) + 10;

            //work on aligning step 1
            //start timer
            stepAligning = true;
            Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
            Timer_A_enableInterrupt(TIMER_A0_BASE);
            //enable timer interrupts
            while(stepAligning); //run PWM for a certain number of counts

            Timer_A_stop(TIMER_A0_BASE);
            Timer_A_disableInterrupt(TIMER_A0_BASE); //stop timer

        }

        __delay_cycles(200000);
        dataReady = false;
        GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN3 | GPIO_PIN4);//reenable accelerometer interrupt?
    }

    LCD_Text(1, 61, "FLAT ALIGNED.", 0xFFFF);
    GPIO_disableInterrupt(GPIO_PORT_P2, GPIO_PIN3 | GPIO_PIN4);

}

/*
 * Name: AxisAlign
 * Purpose: Aligns with the Earth's Axis
 */
void AxisAlign(void) {
    LCD_Clear(0x0000);
    LCD_Text(1, 1, "ALIGNING AXIS", 0xFFFF);

    stepCount = 0;
    bool axisAlign = false;
    dataReady = false;

    struct bmi160_sensor_data accel;
    struct bmi160_sensor_data gyro;

    GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN3 | GPIO_PIN4); //Enable interrupts on GPIO pins for INT1 and INT2
    currStep = STEP2_PWM;
    while(!axisAlign) {
        while(!dataReady);

        GPIO_disableInterrupt(GPIO_PORT_P2, GPIO_PIN3 | GPIO_PIN4);

        bmi160_get_sensor_data(BMI160_ACCEL_ONLY, &accel, &gyro, &bmi160config);
        accel.x = ((accel.x & 0x00FF) << 8) | ((accel.x & 0xFF00) >> 8);
        accel.y = ((accel.y & 0x00FF) << 8) | ((accel.y & 0xFF00) >> 8);
        accel.z = ((accel.z & 0x00FF) << 8) | ((accel.z & 0xFF00) >> 8);

        if (accel.z < 7500) {
            GPIO_setOutputHighOnPin(STEP_PORT, STEP2_DIR);
        }
        else if (accel.z > 9500) {
            GPIO_setOutputLowOnPin(STEP_PORT, STEP2_DIR);
        }

        //if (accel.x <= 15722 && accel.x >= 15722 && accel.z <= 9500 && accel.z >= 7500) {
        else if (accel.x <= 15722 && accel.x >= 13722 && accel.z <= 9500 && accel.z >= 7500) {
            axisAlign = true; //check condition
            continue;
        }

        //start PWM
        stepAligning = true;

        stepCount = 100;

        Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
        Timer_A_enableInterrupt(TIMER_A0_BASE);

        while(stepAligning);

        Timer_A_stop(TIMER_A0_BASE);
        Timer_A_disableInterrupt(TIMER_A0_BASE); //stop timer

        //Stop PWM
        __delay_cycles(200000);
        dataReady = false;
        GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN3 | GPIO_PIN4); //reenable accelerometer interrupt

    }

    LCD_Text(1, 21, "Axis Aligned", 0xFFFF);
    GPIO_setOutputLowOnPin(STEP_PORT, STEP2_DIR);
}

/*
 * Name: AlignNorth
 * Purpose: Aligns camera mount to face towards earth's magnetic north.
 */
void AlignNorth(void) {
    LCD_Clear(0x0000);
    LCD_Text(1, 1, "ALIGNING NORTH", 0xFFFF);

    stepCount = 0;
    bool northAlign = false;
    dataReady = false;

    GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN3 | GPIO_PIN4);
    currStep = STEP3_PWM;

    uint8_t rawData[8];
    struct bmi160_sensor_data mag;

    while(!northAlign) {
        //while(!dataReady);

        GPIO_disableInterrupt(GPIO_PORT_P2, GPIO_PIN3 | GPIO_PIN4); //disable sensor interrupt

        LCD_Clear(0x0000);

        bmi160_read_aux_data_auto_mode(rawData, &bmi160config); //read magnetometer data

        mag.x = ((int16_t)rawData[1] << 8 ) | rawData[0];
        mag.y = ((int16_t)rawData[3] << 8) | rawData[2];
        mag.z = ((int16_t)rawData[5] << 8) | rawData[4];

        LCD_WriteInt(1, 1, mag.x, 0xFFFF);
        LCD_WriteInt(1, 21, mag.y, 0xFFFF);
        LCD_WriteInt(1, 31, mag.z, 0xFFFF);

        __delay_cycles(16777216);

        dataReady = false;
    }



}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER_A0_HANDLER(void) {
    //check which interrupt occurred
   if (TA0CTL & TAIFG) { //Timer overflow interrupt, set pin low
       if (stepAligning) {
           GPIO_setOutputLowOnPin(STEP_PORT, currStep);
           //decrement step counter
           stepCount--;
           //if counter == 0, set stepAligning to false
           if (stepCount <= 0) {
               stepAligning = false;
           }
       }
       TA0CTL &= ~TAIFG;
   }
   else if (TA0CCTL1 & CCIFG) { //PWM interrupt, set pin high
       if (stepAligning) {
           GPIO_setOutputHighOnPin(STEP_PORT, currStep);
       }
       TA0CCTL1 &= ~CCIFG;
   }
}

static Timer_A_initUpModeParam timerA0Param = {
    //! Selects Clock source.
    //! \n Valid values are:
    //! - \b TIMER_A_CLOCKSOURCE_EXTERNAL_TXCLK [Default]
    //! - \b TIMER_A_CLOCKSOURCE_ACLK
    //! - \b TIMER_A_CLOCKSOURCE_SMCLK
    //! - \b TIMER_A_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
    TIMER_A_CLOCKSOURCE_SMCLK,
    //! Is the desired divider for the clock source
    //! \n Valid values are:
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_1 [Default]
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_2
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_3
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_4
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_5
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_6
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_7
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_8
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_10
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_12
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_14
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_16
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_20
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_24
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_28
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_32
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_40
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_48
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_56
    //! - \b TIMER_A_CLOCKSOURCE_DIVIDER_64
    TIMER_A_CLOCKSOURCE_DIVIDER_1,
    //! Is the specified Timer_A period. This is the value that gets written
    //! into the CCR0. Limited to 16 bits[uint16_t]
    0x7FFF, //maybe change later?
    //! Is to enable or disable Timer_A interrupt
    //! \n Valid values are:
    //! - \b TIMER_A_TAIE_INTERRUPT_ENABLE
    //! - \b TIMER_A_TAIE_INTERRUPT_DISABLE [Default]
    TIMER_A_TAIE_INTERRUPT_ENABLE,
    //! Is to enable or disable Timer_A CCR0 captureComapre interrupt.
    //! \n Valid values are:
    //! - \b TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE
    //! - \b TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE [Default]
    TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,
    //! Decides if Timer_A clock divider, count direction, count need to be
    //! reset.
    //! \n Valid values are:
    //! - \b TIMER_A_DO_CLEAR
    //! - \b TIMER_A_SKIP_CLEAR [Default]
    TIMER_A_DO_CLEAR,
    //! Whether to start the timer immediately
    false
};

static Timer_A_initCompareModeParam timerA0Compare = {
    //! Selects the Capture register being used. Refer to datasheet to ensure
    //! the device has the capture compare register being used.
    //! \n Valid values are:
    //! - \b TIMER_A_CAPTURECOMPARE_REGISTER_0
    //! - \b TIMER_A_CAPTURECOMPARE_REGISTER_1
    //! - \b TIMER_A_CAPTURECOMPARE_REGISTER_2
    //! - \b TIMER_A_CAPTURECOMPARE_REGISTER_3
    //! - \b TIMER_A_CAPTURECOMPARE_REGISTER_4
    //! - \b TIMER_A_CAPTURECOMPARE_REGISTER_5
    //! - \b TIMER_A_CAPTURECOMPARE_REGISTER_6
    TIMER_A_CAPTURECOMPARE_REGISTER_1,
    //! Is to enable or disable timer captureComapre interrupt.
    //! \n Valid values are:
    //! - \b TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE [Default]
    //! - \b TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE
    TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE,
    //! Specifies the output mode.
    //! \n Valid values are:
    //! - \b TIMER_A_OUTPUTMODE_OUTBITVALUE [Default]
    //! - \b TIMER_A_OUTPUTMODE_SET
    //! - \b TIMER_A_OUTPUTMODE_TOGGLE_RESET
    //! - \b TIMER_A_OUTPUTMODE_SET_RESET
    //! - \b TIMER_A_OUTPUTMODE_TOGGLE
    //! - \b TIMER_A_OUTPUTMODE_RESET
    //! - \b TIMER_A_OUTPUTMODE_TOGGLE_SET
    //! - \b TIMER_A_OUTPUTMODE_RESET_SET
    TIMER_A_OUTPUTMODE_OUTBITVALUE,
    //! Is the count to be compared with in compare mode
    0x3FFF
};
/*
 * Name: Stepper_Init()
 * Purpose: Initializes stepper motor pins and Timer A0 Module used to PWM the stepper motors
 */
void Stepper_Init(void) {
    //initialize IO
    GPIO_setOutputLowOnPin(STEP_PORT, STEP1_PWM | STEP2_PWM | STEP3_PWM | STEP1_DIR | STEP2_DIR | STEP3_DIR);
    GPIO_setAsOutputPin(STEP_PORT, STEP1_PWM | STEP2_PWM | STEP3_PWM | STEP1_DIR | STEP2_DIR | STEP3_DIR);

    Timer_A_initUpMode(TIMER_A0_BASE, &timerA0Param); //initialize up mode and set CCR
    Timer_A_initCompareMode(TIMER_A0_BASE, &timerA0Compare);

}

#pragma vector=PORT2_VECTOR
__interrupt void P2_HANDLER(void) {
    //check which pin triggered interrupt
    if (P2IFG & BIT3 || P2IFG & BIT4) {
            dataReady = true;
            P2IFG &= ~BIT3;
            P2IFG &= ~BIT4;
    }
    //set data ready flag
}

extern bool button_f;
extern bool select_f;
extern RE_DIR select_dir;

void MoveSteppersManual(void) {
    //initialize IO
    //GPIO_setOutputLowOnPin(STEP_PORT, STEP1_PWM | STEP2_PWM | STEP3_PWM | STEP1_DIR | STEP2_DIR | STEP3_DIR);
    //GPIO_setAsOutputPin(STEP_PORT, STEP1_PWM | STEP2_PWM | STEP3_PWM | STEP1_DIR | STEP2_DIR | STEP3_DIR);

    uint16_t currMotor = 1;
    uint16_t currPWMPin = STEP1_PWM;
    uint16_t currDirPin = STEP1_DIR;

    manualAlign = true;
    uint16_t numPulse = 10;

    GPIO_enableInterrupt(RE_PORT, RE1_PB);

    //wait for update from rotary encoders
    while(manualAlign) {
        while(manualAlign && !button_f && !select_f);
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
                    stepCount--;
                }
                else {
                    GPIO_setOutputHighOnPin(STEP_PORT, currDirPin);
                    stepCount++;
                }

                //if (stepCount > 500) { //fixme: temp, hang forever if issue occurs
                //    while(1);
                //}

                if (currPWMPin == STEP3_PWM) {
                    numPulse = 50;
                }
                else {
                    numPulse = 10;
                }

                for (uint16_t i = 0; i < numPulse; i++) {
                    GPIO_setOutputHighOnPin(STEP_PORT, currPWMPin);
                    __delay_cycles(20000);
                    GPIO_setOutputLowOnPin(STEP_PORT, currPWMPin);
                    select_f = false;
                    __delay_cycles(20000);
                }

        }
    }

    GPIO_disableInterrupt(RE_PORT, RE1_PB);

}

/*
 * Name: Stepper_Track
 * Purpose: Rotates Step 1 clockwise to counteract earth's counterclockwise rotation
 */
void Stepper_Track(void) {
    manualAlign = true;

    currStep = STEP1_PWM;
    //set dir on step 1
    GPIO_setOutputHighOnPin(STEP_PORT, STEP1_DIR);
    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0, 0xFFFF);
    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1, 0x7FFF);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    Timer_A_enableInterrupt(TIMER_A0_BASE);
    stepCount = 30000;
    stepAligning = true;
    GPIO_enableInterrupt(RE_PORT, RE1_PB);
    //start Timer A
    while(stepAligning && manualAlign);
    Timer_A_stop(TIMER_A0_BASE);
    manualAlign = false;
    stepAligning = false;
    stepCount = 0;
    Timer_A_disableInterrupt(TIMER_A0_BASE);
    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0, timerA0Param.timerPeriod);
    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1, timerA0Compare.compareValue);
    GPIO_disableInterrupt(RE_PORT, RE1_PB);
}



