/*
 * Stepper.h
 *
 *  Created on: Apr 13, 2022
 *      Author: troth
 */

#ifndef DEVICES_INC_STEPPER_H_
#define DEVICES_INC_STEPPER_H_

#define STEP_PORT   GPIO_PORT_P1
#define STEP1_PWM   GPIO_PIN1
#define STEP2_PWM   GPIO_PIN2
#define STEP3_PWM   GPIO_PIN3
#define STEP1_DIR   GPIO_PIN4
#define STEP2_DIR   GPIO_PIN5
#define STEP3_DIR   GPIO_PIN6

/*
 * Name: Stepper_Init()
 * Purpose: Initializes stepper motor pins and Timer A0 Module used to PWM the stepper motors
 */
void Stepper_Init(void);

/*
 * Name: AlignSteppers
 * Purpose: Aligns camera mount to be parallel to ground and aligns north
 */
void AlignSteppers(void);

/*
 * Name: Stepper_Track
 * Purpose: Rotates Step 1 clockwise to counteract earth's counterclockwise rotation
 */
void Stepper_Track(void);

void AxisAlign(void);

/*
 * Name: AlignNorth
 * Purpose: Aligns camera mount to face towards earth's magnetic north.
 */
void AlignNorth(void);

/*
 * Move steppers with rotary encoders
 */
void MoveSteppersManual(void);

#endif /* DEVICES_INC_STEPPER_H_ */
