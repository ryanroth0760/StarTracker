/*
 * Speaker.h
 *
 *  Created on: Apr 14, 2022
 *      Author: troth
 */

#ifndef DEVICES_INC_SPEAKER_H_
#define DEVICES_INC_SPEAKER_H_

#define SPEAKER_PORT    GPIO_PORT_P6
#define SPEAKER_EN      GPIO_PIN5
#define SPEAKER_DAC     GPIO_PIN6

/*
 * Name: Speaker_Init
 * Purpose: Initializes DAC and IO pin for speaker input and enable
 */
void Speaker_Init(void);

/*
 * Name: Speaker_Beep
 * Purpose: Sets speaker enable and enables timer interrupt which plays out LUT on the DAC. Interrupt will disable speaker and interrupt when done
 */
void Speaker_Beep(void);


#endif /* DEVICES_INC_SPEAKER_H_ */
