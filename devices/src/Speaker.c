/*
 * Speaker.c
 *
 *  Created on: Apr 14, 2022
 *      Author: troth
 */

#include "driverlib.h"
#include "Speaker.h"

static Timer_A_initUpModeParam timerA2Param = {
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_1,
    115,
    TIMER_A_TAIE_INTERRUPT_ENABLE,
    TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,
    TIMER_A_DO_CLEAR,
    false
};

static struct DAC12_A_initParam dacParam = {
    //! Decides which DAC12_A sub-module to configure.
    //! \n Valid values are:
    //! - \b DAC12_A_SUBMODULE_0
    //! - \b DAC12_A_SUBMODULE_1
    DAC12_A_SUBMODULE_0,
    //! Selects the output pin that the selected DAC12_A module will output to.
    //! \n Valid values are:
    //! - \b DAC12_A_OUTPUT_1 [Default]
    //! - \b DAC12_A_OUTPUT_2
    DAC12_A_OUTPUT_1,
    //! Is the upper limit voltage that the data can be converted in to.
    //! \n Valid values are:
    //! - \b DAC12_A_VREF_INT [Default]
    //! - \b DAC12_A_VREF_AVCC
    //! - \b DAC12_A_VREF_EXT - For devices with CTSD16, use Ref module
    //!    Ref_enableReferenceVoltageOutput/Ref__disableReferenceVoltageOutput
    //!    to select VeREF(external reference signal) or VREFBG(internally
    //!    generated reference signal)
    DAC12_A_VREF_AVCC,
    //! Is the multiplier of the Vout voltage.
    //! \n Valid values are:
    //! - \b DAC12_A_VREFx1 [Default]
    //! - \b DAC12_A_VREFx2
    //! - \b DAC12_A_VREFx3
    DAC12_A_VREFx1,
    //! Is the setting of the settling speed and current of the Vref+ and the
    //! Vout buffer.
    //! \n Valid values are:
    //! - \b DAC12_A_AMP_OFF_PINOUTHIGHZ [Default] - Initialize the DAC12_A
    //!    Module with settings, but do not turn it on.
    //! - \b DAC12_A_AMP_OFF_PINOUTLOW - Initialize the DAC12_A Module with
    //!    settings, and allow it to take control of the selected output pin to
    //!    pull it low (Note: this takes control away port mapping module).
    //! - \b DAC12_A_AMP_LOWIN_LOWOUT - Select a slow settling speed and
    //!    current for Vref+ input buffer and for Vout output buffer.
    //! - \b DAC12_A_AMP_LOWIN_MEDOUT - Select a slow settling speed and
    //!    current for Vref+ input buffer and a medium settling speed and
    //!    current for Vout output buffer.
    //! - \b DAC12_A_AMP_LOWIN_HIGHOUT - Select a slow settling speed and
    //!    current for Vref+ input buffer and a high settling speed and current
    //!    for Vout output buffer.
    //! - \b DAC12_A_AMP_MEDIN_MEDOUT- Select a medium settling speed and
    //!    current for Vref+ input buffer and for Vout output buffer.
    //! - \b DAC12_A_AMP_MEDIN_HIGHOUT - Select a medium settling speed and
    //!    current for Vref+ input buffer and a high settling speed and current
    //!    for Vout output buffer.
    //! - \b DAC12_A_AMP_HIGHIN_HIGHOUT - Select a high settling speed and
    //!    current for Vref+ input buffer and for Vout output buffer.
    DAC12_A_AMP_MEDIN_MEDOUT,
    //! Selects the trigger that will start a conversion.
    //! \n Valid values are:
    //! - \b DAC12_A_TRIGGER_ENCBYPASS [Default] - Automatically converts data
    //!    as soon as it is written into the data buffer. (Note: Do not use
    //!    this selection if grouping DAC's).
    //! - \b DAC12_A_TRIGGER_ENC - Requires a call to enableConversions() to
    //!    allow a conversion, but starts a conversion as soon as data is
    //!    written to the data buffer (Note: with DAC12_A module's grouped,
    //!    data has to be set in BOTH DAC12_A data buffers to start a
    //!    conversion).
    //! - \b DAC12_A_TRIGGER_TA - Requires a call to enableConversions() to
    //!    allow a conversion, and a rising edge of Timer_A's Out1 (TA1) to
    //!    start a conversion.
    //! - \b DAC12_A_TRIGGER_TB - Requires a call to enableConversions() to
    //!    allow a conversion, and a rising edge of Timer_B's Out2 (TB2) to
    //!    start a conversion.
    DAC12_A_TRIGGER_ENCBYPASS
};

static const uint16_t sineLUT[] = { 0x800,0x864,0x8c8,0x92c,0x98f,0x9f1,0xa52,0xab1,
                                0xb0f,0xb6b,0xbc5,0xc1c,0xc71,0xcc3,0xd12,0xd5f,
                                0xda7,0xded,0xe2e,0xe6c,0xea6,0xedc,0xf0d,0xf3a,
                                0xf63,0xf87,0xfa7,0xfc2,0xfd8,0xfe9,0xff5,0xffd,
                                0xfff,0xffd,0xff5,0xfe9,0xfd8,0xfc2,0xfa7,0xf87,
                                0xf63,0xf3a,0xf0d,0xedc,0xea6,0xe6c,0xe2e,0xded,
                                0xda7,0xd5f,0xd12,0xcc3,0xc71,0xc1c,0xbc5,0xb6b,
                                0xb0f,0xab1,0xa52,0x9f1,0x98f,0x92c,0x8c8,0x864,
                                0x800,0x79b,0x737,0x6d3,0x670,0x60e,0x5ad,0x54e,
                                0x4f0,0x494,0x43a,0x3e3,0x38e,0x33c,0x2ed,0x2a0,
                                0x258,0x212,0x1d1,0x193,0x159,0x123,0xf2,0xc5,
                                0x9c,0x78,0x58,0x3d,0x27,0x16,0xa,0x2,
                                0x0,0x2,0xa,0x16,0x27,0x3d,0x58,0x78,
                                0x9c,0xc5,0xf2,0x123,0x159,0x193,0x1d1,0x212,
                                0x258,0x2a0,0x2ed,0x33c,0x38e,0x3e3,0x43a,0x494,
                                0x4f0,0x54e,0x5ad,0x60e,0x670,0x6d3,0x737,0x79b};

static uint16_t speakerCount = 0;

/*
 * Name: Speaker_Init
 * Purpose: Initializes DAC and IO pin for speaker input and enable. Configures speaker timer
 */
void Speaker_Init(void) {
    Timer_A_initUpMode(TIMER_A2_BASE, &timerA2Param);
    DAC12_A_init(DAC12_A_BASE, &dacParam);
    DAC12_A_setInputDataFormat(DAC12_A_BASE, DAC12_A_SUBMODULE_0, DAC12_A_JUSTIFICATION_RIGHT, DAC12_A_UNSIGNED_BINARY);
    DAC12_A_calibrateOutput(DAC12_A_BASE, DAC12_A_SUBMODULE_0);

    GPIO_setOutputLowOnPin(SPEAKER_PORT, SPEAKER_EN); //configure IO pin for speaker EN
    GPIO_setAsOutputPin(SPEAKER_PORT, SPEAKER_EN);
    GPIO_setAsPeripheralModuleFunctionOutputPin(SPEAKER_PORT, SPEAKER_DAC);
}

/*
 * Name: Speaker_Beep
 * Purpose: Sets speaker enable and enables timer interrupt which plays out LUT on the DAC. Interrupt will disable speaker and interrupt when done
 */
void Speaker_Beep(void) {
    GPIO_setOutputHighOnPin(SPEAKER_PORT, SPEAKER_EN); //enable speaker
    speakerCount = 0;
    Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE); //start timer A2
}

#pragma vector=TIMER2_A1_VECTOR
__interrupt void TIMERA2_HANDLER(void) {
    DAC12_A_setData(DAC12_A_BASE, DAC12_A_SUBMODULE_0, sineLUT[speakerCount & 0x7F]);
    speakerCount++;
    if (speakerCount >= 28160) {
        Timer_A_stop(TIMER_A2_BASE);
        GPIO_setOutputLowOnPin(SPEAKER_EN, SPEAKER_DAC);
    }
    TA2CTL &= ~TAIFG;
}
