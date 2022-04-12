/*
 * GPS.h
 *
 *  Created on: Apr 1, 2022
 *      Author: troth
 */

#ifndef GPS_H_
#define GPS_H_

#include "driverlib.h"

/******** CONSTANT DEFINITIONS ********/

#ifdef __MSP430F6438
#define GPS_PORT_IO     GPIO_PORT_P4
#define GPS_EN          GPIO_PIN3
#define GPS_FIX         GPIO_PIN2
#define GPS_PPS         GPIO_PIN1
#define GPS_PORT_TXRX   GPIO_PORT_P8
#define GPS_OUT         GPIO_PIN3
#define GPS_IN          GPIO_PIN2

#define GPS_BASE        USCI_A1_BASE

#elif defined __MSP430F5529__
#define GPS_PORT_IO     GPIO_PORT_P6
#define GPS_EN          GPIO_PIN3
#define GPS_PORT_TXRX   GPIO_PORT_P3
#define GPS_OUT         GPIO_PIN4
#define GPS_IN          GPIO_PIN3

#define GPS_BASE        USCI_A0_BASE

#endif



/******** CONSTANT DEFINITIONS ********/

/******** GPS DATA STRUCTURE ********/

typedef struct _GPS_DATA { //each string has an extra character for null terminator
    char timeStamp[7]; //holds the timestamp for the fix
    bool valid; //set to false if status flag is V, true if status flag is A
    char latitudeDegrees[3]; //longitude string
    char latitudeMinutes[6];
    bool latNorth;
    char longitudeDegrees[4];
    char longitudeMinutes[6];
    bool longEast;
    char date[7];
} gps_data_t;

/******** GPS DATA STRUCTURE ********/

/******** PUBLIC FUNCTION PROTOTYPES ********/

/*
 * Name: GPS_Init();
 * Purpose: Initialize UART 9600 baud on port A to fit NMEA standard for GPS
 *          Initialize GPS pins for proper reading
 */
void GPS_Init(void);

/*
 * Name: GPS_disable();
 * Purpose: Turns off GPS by setting EN pin to low and turning off UART module
 */
void GPS_disable(void);

/*
 * Name: GPS_enable();
 * Purpose: Turn on GPS by setting EN pin to high and turning on UART module
 */
void GPS_enable(void);

/*
 * Name: GPS_getData
 * Purpose: Returns parsed GPS data for use by the calling program
 */
void GPS_getData(gps_data_t* data);

/******** PUBLIC FUNCTION PROTOTYPES ********/



#endif /* GPS_H_ */
