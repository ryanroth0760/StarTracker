/*
 * GPS.c
 *
 *  Created on: Apr 1, 2022
 *      Author: troth
 */

#include "GPS.h"
#include "driverlib.h"
#include "usci_a_uart.h"

/******** PRIVATE FUNCTION PROTOTYPES ********/

/*
 * Name: GPS_getDataRaw
 * Purpose: Receives valid GPRMC sentence from the GPS and returns a pointer for it.
 *          Pointer will point to one of two ping-pong buffers in global space
 */
char* GPS_getDataRaw(void);

/*
 * Name: GPS_interruptHandler
 * Purpose: Reads data into one of two ping pong buffers.
 *          Switch buffers and set a flag for processing when end of sentence condition is reached
 */
//__interrupt void GPS_RX_interruptHandler(void);

/******** PRIVATE FUNCTION PROTOTYPES ********/

/******** PRIVATE GLOBAL VARIABLES ********/

static char GPS_buffer1[128];
static char GPS_buffer2[128];
static char* GPS_sampling;
static char* GPS_processing;

/******** PRIVATE GLOBAL VARIABLES ********/

/******** PUBLIC FUNCTION DEFINITIONS ********/

void GPS_Init(void) {
    USCI_A_UART_initParam GPS_Init_param = {
        USCI_A_UART_CLOCKSOURCE_SMCLK, //clock source
        109, //clock prescaler
        3, //first mod reg, get from users guide
        0xB5, //second mod reg
        USCI_A_UART_NO_PARITY, //parity
        USCI_A_UART_LSB_FIRST,
        USCI_A_UART_ONE_STOP_BIT, //FIXME: initialize to LSB and test for correctness later
        USCI_A_UART_MODE, //UART mode
        USCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION //low frequency or oversampling baud generation
    };

    USCI_A_UART_init(GPS_BASE, &GPS_Init_param); //initializes UART module for 9600 baud, no parity, LSB first to communicate with GPS. Does not turn module on

    //REMAP GPS pins to use UART
#ifdef __MSP430F6438
    P8SEL |= BIT2 | BIT3; //SET PORT 8 PIN 2 AND 3 AS UART PINS

#elif defined __MSP430F5529__
    P3SEL |= BIT3 | BIT4; //set port 3 pin 3 and 4 as UART pins

#endif

    //Pin 4.3 for msp430f6
    //Initialize GPIO pin to serve as UART disable.
    GPIO_setOutputLowOnPin(GPS_PORT_IO, GPS_EN); //set port to initialize 0, off

    GPIO_setAsOutputPin(GPS_PORT_IO, GPS_EN);
    //GPIO_setAsInputPin(GPS_PORT_IO, GPS_FIX);
    //GPIO_setAsInputPin(GPS_PORT_IO, GPS_PPS);

    //FIXME: configure interrupts for UART port RX? but dont enable

}

/*
 * Name: GPS_getData
 * Purpose: Returns parsed GPS data for use by the calling program
 */
void GPS_getData(gps_data_t* data) {
    char* dataRaw;
    dataRaw = GPS_getDataRaw();

    char currSection[10]; //stores data for the current section enclosed by delimiters
    unsigned int sectionIndex = 0;
    data->valid = true; //GPS_getDataRaw already ensures that the data returned is valid

    unsigned int index = 7; //timestamp begins at GPS data index 7
    sectionIndex = 0;

    //Parse time from raw string
    while (dataRaw[index] != ',') {
        currSection[sectionIndex] = dataRaw[index];
        index++;
        sectionIndex++;
    }
    index++; //increment past delimiter

    for (int i = 0; i < sectionIndex; i++) {
        data->timeStamp[i] = currSection[i];
    }
    data->timeStamp[6] = 0;
    sectionIndex = 0;

    //parse validity
    index += 2;

    sectionIndex = 0;
    //parse latitude
    while (dataRaw[index] != ',') {
        currSection[sectionIndex] = dataRaw[index];
        index++;
        sectionIndex++;
    }
    index++;

    //parse degrees from minutes
    for (int i = 0; i < sectionIndex; i++) {
        if (i < 2) {
            data->latitudeDegrees[i] = currSection[i];
        }
        else {
            data->latitudeMinutes[i-2] = currSection[i];
        }
    }
    data->latitudeDegrees[2] = 0;
    data->latitudeMinutes[5] = 0;

    //Parse north/south
    data->latNorth = dataRaw[index] == 'N';
    index += 2;

    sectionIndex = 0;
    //Parse longitude
    while (dataRaw[index] != ',') {
       currSection[sectionIndex] = dataRaw[index];
       index++;
       sectionIndex++;
    }
    index++;

    //parse degrees from minutes
    for (int i = 0; i < sectionIndex; i++) {
        if (i < 3) {
            data->longitudeDegrees[i] = currSection[i];
        }
        else {
            data->longitudeMinutes[i-3] = currSection[i];
        }
    }
    data->longitudeDegrees[3] = 0;
    data->longitudeMinutes[5] = 0;

    //parse east/west
    data->longEast = dataRaw[index] == 'E';
}

/*
 * Name: GPS_enable();
 * Purpose: Turn on GPS by setting EN pin to high and turning on UART module
 */
void GPS_enable(void) {
    GPIO_setOutputHighOnPin(GPS_PORT_IO, GPS_EN);
    USCI_A_UART_enable(GPS_BASE);
}

/*
 * Name: GPS_disable();
 * Purpose: Turns off GPS by setting EN pin to low and turning off UART module
 */
void GPS_disable(void) {
    GPIO_setOutputLowOnPin(GPS_PORT_IO, GPS_EN);
    USCI_A_UART_disable(GPS_BASE);
}

/******** PUBLIC FUNCTION DEFINITIONS ********/


/******** PRIVATE FUNCTION DEFINITIONS ********/

static volatile bool sentenceReceived = false;

/*
 * Name: GPS_getDataRaw
 * Purpose: Receives valid GPRMC sentence from the GPS and returns a pointer for it.
 *          Pointer will point to one of two ping-pong buffers in global space
 */
char* GPS_getDataRaw(void) {
    GPS_sampling = &GPS_buffer1[0];
    GPS_processing = &GPS_buffer2[0];
    bool validSentence = false;
    unsigned int index = 0;

    USCI_A_UART_enableInterrupt(GPS_BASE, USCI_A_UART_RECEIVE_INTERRUPT); //FIXME: enable interrupts on GPS RX Port

    //wait until valid sentence received
    while(!validSentence) {
        while(!sentenceReceived); //poll until a full sentence is received
        //check status first 6 characters for status code, then check status code
        validSentence = GPS_processing[1] == 'G' && GPS_processing[2] == 'P' && GPS_processing[3] == 'R' && GPS_processing[4] == 'M' &&
                        GPS_processing[5] == 'C';
        //parse until status flag
        index = 7;
        while(GPS_processing[index] != ',') {
            index++;
        }
        index++;
        validSentence = validSentence && (GPS_processing[index] == 'A');
        sentenceReceived = false;
    }
    USCI_A_UART_disableInterrupt(GPS_BASE, USCI_A_UART_RECEIVE_INTERRUPT); //FIXME: disable interrupts on GPS RX Port
    return GPS_processing;
}

static unsigned int GPS_index = 0;
static bool sentenceActive = false;
/*
 * Name: GPS_interruptHandler
 * Purpose: Reads data into one of two ping pong buffers.
 *          Switch buffers and set a flag for processing when end of sentence condition is reached
 */
#ifdef __MSP430F6438
#pragma vector=USCI_A1_VECTOR
#elif defined __MSP430F5529
#pragma vector=USCI_A0_VECTOR
#endif
__interrupt void GPS_RX_interruptHandler(void) {
    char* temp;
    char received = UCA1RXBUF; //read received character from UART buffer
    if (received == '$') {
        sentenceActive = true;
    }
    if (sentenceActive) {
        GPS_sampling[GPS_index] = received;//write character to sample buffer
        if (received == 0x0a) {//if character is line feed, sentence has ended.
            temp = GPS_processing; //Switch buffers and set sentence received flag.
            GPS_processing = GPS_sampling;
            GPS_sampling = temp;
            sentenceReceived = true;
            GPS_index = 0; //Reset index to 0
            sentenceActive = false;
        }
    }

}

/******** PRIVATE FUNCTION DEFINITIONS ********/


