/*
 * LCDLib.h
 *
 *  Created on: Mar 2, 2017
 *      Author: Danny
 */

#ifndef LCDLIB_H_
#define LCDLIB_H_

#include <stdbool.h>
#include <stdint.h>
#include "driverlib.h"
#include "msp430.h"
/************************************ Defines *******************************************/

/* LCD SPI PIN CONFIG */
#ifdef __MSP430F6438__
#define LCD_PORT_IO     GPIO_PORT_P9
#define LCD_TFT_CS_PORT GPIO_PORT_P8
#define LCD_PORT_SPI    GPIO_PORT_P8
#define LCD_SIMO        GPIO_PIN5
#define LCD_SOMI        GPIO_PIN6
#define LCD_SCLK         GPIO_PIN4

#define LCD_RESET       GPIO_PIN2
#define LCD_DC          GPIO_PIN1
#define LCD_CARD_CS     GPIO_PIN0
#define LCD_TFT_CS      GPIO_PIN7

#define LCD_LITE        GPIO_PIN3
#define LCD_PWM_TIMER   TIMER_A1_BASE
#define LCD_PWM_PORT    GPIO_PORT_P3
#define LCD_PWM_REG     TIMER_A_CAPTURECOMPARE_REGISTER_0
#define LCD_OUTPUT_MODE TIMER_A_OUTPUTMODE_TOGGLE

#elif defined __MSP430F5529__
#define LCD_PORT_IO     GPIO_PORT_P6 //FIXME: temp port figure one out later
#define LCD_PORT_SPI    GPIO_PORT_P4
#define LCD_TFT_CS_PORT GPIO_PORT_P6
#define LCD_SIMO        GPIO_PIN1
#define LCD_SOMI        GPIO_PIN2
#define LCD_SCLK         GPIO_PIN3

#define LCD_RESET       GPIO_PIN2
#define LCD_DC          GPIO_PIN1
#define LCD_CARD_CS     GPIO_PIN0
#define LCD_TFT_CS      GPIO_PIN4
#define LCD_LITE        GPIO_PIN2
#define LCD_PWM_TIMER   TIMER_A0_BASE
#define LCD_PWM_PORT    GPIO_PORT_P1
#define LCD_PWM_REG     TIMER_A_CAPTURECOMPARE_REGISTER_1
#define LCD_OUTPUT_MODE TIMER_A_OUTPUTMODE_RESET_SET

#endif

#define LCD_PWM_INCDEC  1000 //value to increment or decrement by when rotary encoder is turned. experiment to determine a good setting

/* REGISTERS AND COMMANDS FROM ADAFRUIT LCD DRIVER LIBRARY https://github.com/adafruit/Adafruit-ST7735-Library/blob/master/Adafruit_ST77xx.h */

#define ST_CMD_DELAY 0x80 // special signifier for command lists

#define ST77XX_NOP 0x00
#define ST77XX_SWRESET 0x01
#define ST77XX_RDDID 0x04
#define ST77XX_RDDST 0x09

#define ST77XX_SLPIN 0x10
#define ST77XX_SLPOUT 0x11
#define ST77XX_PTLON 0x12
#define ST77XX_NORON 0x13

#define ST77XX_INVOFF 0x20
#define ST77XX_INVON 0x21
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON 0x29
#define ST77XX_CASET 0x2A
#define ST77XX_RASET 0x2B
#define ST77XX_RAMWR 0x2C
#define ST77XX_RAMRD 0x2E

#define ST77XX_PTLAR 0x30
#define ST77XX_TEOFF 0x34
#define ST77XX_TEON 0x35
#define ST77XX_MADCTL 0x36
#define ST77XX_COLMOD 0x3A

#define ST77XX_MADCTL_MY 0x80
#define ST77XX_MADCTL_MX 0x40
#define ST77XX_MADCTL_MV 0x20
#define ST77XX_MADCTL_ML 0x10
#define ST77XX_MADCTL_RGB 0x00

#define ST77XX_RDID1 0xDA
#define ST77XX_RDID2 0xDB
#define ST77XX_RDID3 0xDC
#define ST77XX_RDID4 0xDD

// Some ready-made 16-bit ('565') color settings:
#define ST77XX_BLACK 0x0000
#define ST77XX_WHITE 0xFFFF
#define ST77XX_RED 0xF800
#define ST77XX_GREEN 0x07E0
#define ST77XX_BLUE 0x001F
#define ST77XX_CYAN 0x07FF
#define ST77XX_MAGENTA 0xF81F
#define ST77XX_YELLOW 0xFFE0
#define ST77XX_ORANGE 0xFC00

// Some register settings
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH 0x04

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// Some ready-made 16-bit ('565') color settings:
#define ST7735_BLACK ST77XX_BLACK
#define ST7735_WHITE ST77XX_WHITE
#define ST7735_RED ST77XX_RED
#define ST7735_GREEN ST77XX_GREEN
#define ST7735_BLUE ST77XX_BLUE
#define ST7735_CYAN ST77XX_CYAN
#define ST7735_MAGENTA ST77XX_MAGENTA
#define ST7735_YELLOW ST77XX_YELLOW
#define ST7735_ORANGE ST77XX_ORANGE

/* END ADAFRUIT LIBRARY DEFINES */

/* Screen size */
#define MAX_SCREEN_X     160
#define MAX_SCREEN_Y     128
#define MIN_SCREEN_X     0
#define MIN_SCREEN_Y     0
#define SCREEN_SIZE      20480

#define col_start        2
#define row_start        1

/* Register details */
#define SPI_START   (0x70)     /* Start byte for SPI transfer        */
#define SPI_RD      (0x01)     /* WR bit 1 within start              */
#define SPI_WR      (0x00)     /* WR bit 0 within start              */
#define SPI_DATA    (0x02)     /* RS bit 1 within start byte         */
#define SPI_INDEX   (0x00)     /* RS bit 0 within start byte         */

/* CS LCD*/
#define SPI_CS_LOW      GPIO_setOutputLowOnPin(LCD_TFT_CS_PORT, LCD_TFT_CS)
#define SPI_CS_HIGH     GPIO_setOutputHighOnPin(LCD_TFT_CS_PORT, LCD_TFT_CS)

/* LCD Command Select Pin */
#define SPI_DC_LOW      GPIO_setOutputLowOnPin(LCD_PORT_IO, LCD_DC)
#define SPI_DC_HIGH     GPIO_setOutputHighOnPin(LCD_PORT_IO, LCD_DC)

/* CS Touchpanel */
//#define SPI_CS_TP_LOW P10OUT &= ~BIT5
//#define SPI_CS_TP_HIGH P10OUT |= BIT5

/* XPT2046 registers definition for X and Y coordinate retrieval */
#define CHX         0x90
#define CHY         0xD0

/* LCD colors */
#define LCD_WHITE          0xFFFF
#define LCD_BLACK          0x0000
#define LCD_BLUE           0x0197
#define LCD_RED            0xF800
#define LCD_MAGENTA        0xF81F
#define LCD_GREEN          0x07E0
#define LCD_CYAN           0x7FFF
#define LCD_YELLOW         0xFFE0
#define LCD_GRAY           0x2104
#define LCD_PURPLE         0xF11F
#define LCD_ORANGE         0xFD20
#define LCD_PINK           0xfdba
#define LCD_OLIVE          0xdfe4

/* ILI 9325 registers definition */
#define READ_ID_CODE                        0x00
#define DRIVER_OUTPUT_CONTROL               0x01
#define DRIVING_WAVE_CONTROL                0x02
#define ENTRY_MODE                          0x03
#define RESIZING_CONTROL                    0x04
#define DISPLAY_CONTROL_1                   0x07
#define DISPLAY_CONTROL_2                   0x08
#define DISPLAY_CONTROL_3                   0x09
#define DISPLAY_CONTROL_4                   0x0A
#define RGB_DISPLAY_INTERFACE_CONTROL_1     0x0C
#define FRAME_MARKER_POSITION               0x0D
#define RGB_DISPLAY_INTERFACE_CONTROL_2     0x0F
#define POWER_CONTROL_1                     0x10
#define POWER_CONTROL_2                     0x11
#define POWER_CONTROL_3                     0x12
#define POWER_CONTROL_4                     0x13
#define GRAM_HORIZONTAL_ADDRESS_SET         0x20
#define GRAM_VERTICAL_ADDRESS_SET           0x21
#define DATA_IN_GRAM                        0x22
#define POWER_CONTROL_7                     0x29
#define FRAME_RATE_AND_COLOR_CONTROL        0x2B

#define GAMMA_CONTROL_1                     0x30
#define GAMMA_CONTROL_2                     0x31
#define GAMMA_CONTROL_3                     0x32
#define GAMMA_CONTROL_4                     0x35
#define GAMMA_CONTROL_5                     0x36
#define GAMMA_CONTROL_6                     0x37
#define GAMMA_CONTROL_7                     0x38
#define GAMMA_CONTROL_8                     0x39
#define GAMMA_CONTROL_9                     0x3C
#define GAMMA_CONTROL_10                    0x3D

#define HOR_ADDR_START_POS                  0x50
#define HOR_ADDR_END_POS                    0x51
#define VERT_ADDR_START_POS                 0x52
#define VERT_ADDR_END_POS                   0x53
#define GATE_SCAN_CONTROL_0X60              0x60
#define GATE_SCAN_CONTROL_0X61              0x61
#define GATE_SCAN_CONTROL_0X6A              0x6A
#define PART_IMAGE_1_DISPLAY_POS            0x80
#define PART_IMG_1_START_END_ADDR_0x81      0x81
#define PART_IMG_1_START_END_ADDR_0x82      0x81
#define PART_IMAGE_2_DISPLAY_POS            0x83
#define PART_IMG_2_START_END_ADDR_0x84      0x84
#define PART_IMG_2_START_END_ADDR_0x85      0x85
#define PANEL_ITERFACE_CONTROL_1            0x90
#define PANEL_ITERFACE_CONTROL_2            0x92
#define PANEL_ITERFACE_CONTROL_4            0x95

#define GRAM                                0x22
#define HORIZONTAL_GRAM_SET                 0x20
#define VERTICAL_GRAM_SET                   0x21

/************************************ Defines *******************************************/

/********************************** Structures ******************************************/
typedef struct Point {
    uint16_t x;
    uint16_t y;
}Point;
/********************************** Structures ******************************************/

/************************************ Public Functions  *******************************************/

/*
 * Increments duty cycle value in timer by defined inc dec value
 */
inline void LCD_incBacklight(void);

/*
 * Decrements duty cycle value in timer by defined inc dec value.
 */
inline void LCD_decBacklight(void);

/*******************************************************************************
 * Function Name  : LCD_DrawRectangle
 * Description    : Draw a rectangle as the specified color
 * Input          : xStart, xEnd, yStart, yEnd, Color
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void LCD_DrawRectangle(int16_t xStart, int16_t xEnd, int16_t yStart, int16_t yEnd, uint16_t Color);

/******************************************************************************
* Function Name  : PutChar
* Description    : Lcd screen displays a character
* Input          : - Xpos: Horizontal coordinate
*                  - Ypos: Vertical coordinate
*                  - ASCI: Displayed character
*                  - charColor: Character color
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
inline void PutChar( uint16_t Xpos, uint16_t Ypos, uint8_t ASCI, uint16_t charColor);

/******************************************************************************
* Function Name  : LCD_Text
* Description    : Displays the string
* Input          : - Xpos: Horizontal coordinate
*                  - Ypos: Vertical coordinate
*                  - str: Displayed string
*                  - charColor: Character color
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
void LCD_Text(uint16_t Xpos, uint16_t Ypos, uint8_t *str,uint16_t Color);

/******************************************************************************
* Function Name  : LCD_WriteInt
* Description    : Displays the string
* Input          : - Xpos: Horizontal coordinate
*                  - Ypos: Vertical coordinate
*                  - val: Displayed string
*                  - Color: Character color
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
void LCD_WriteInt(uint16_t Xpos, uint16_t Ypos, uint32_t val, uint16_t Color);

/*******************************************************************************
* Function Name  : LCD_Write_Data_Only
* Description    : Data writing to the LCD controller
* Input          : - data: data to be written
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
inline void LCD_Write_Data_Only(uint16_t data);

/*******************************************************************************
* Function Name  : LCD_Clear
* Description    : Fill the screen as the specified color
* Input          : - Color: Screen Color
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
void LCD_Clear(uint16_t Color);

/******************************************************************************
* Function Name  : LCD_SetPoint
* Description    : Drawn at a specified point coordinates
* Input          : - Xpos: Row Coordinate
*                  - Ypos: Line Coordinate
* Output         : None
* Return         : None
* Attention      : 18N Bytes Written
*******************************************************************************/
void LCD_SetPoint(uint16_t Xpos, uint16_t Ypos, uint16_t color);

/*******************************************************************************
* Function Name  : LCD_WriteData
* Description    : LCD write register data
* Input          : - data: register data
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
inline void LCD_WriteData(uint16_t data);

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Reads the selected LCD Register.
* Input          : None
* Output         : None
* Return         : LCD Register Value.
* Attention      : None
*******************************************************************************/
inline uint16_t LCD_ReadReg(uint16_t LCD_reg);

/*******************************************************************************
* Function Name  : LCD_WriteIndex
* Description    : LCD write register address
* Input          : - index: register address
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
inline void LCD_WriteIndex(uint16_t index);

/*******************************************************************************
 * Function Name  : SPISendRecvTPByte
 * Description    : Send one byte then receive one byte of response from Touchpanel
 * Input          : uint8_t: byte
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline uint8_t SPISendRecvTPByte (uint8_t byte);

/*******************************************************************************
* Function Name  : SPISendRecvByte
* Description    : Send one byte then recv one byte of response
* Input          : uint8_t: byte
* Output         : None
* Return         : Recieved value 
* Attention      : None
*******************************************************************************/
inline uint8_t SPISendRecvByte(uint8_t byte);

/*******************************************************************************
* Function Name  : LCD_Write_Data_Start
* Description    : Start of data writing to the LCD controller
* Input          : None
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
inline void LCD_Write_Data_Start(void);

/*******************************************************************************
* Function Name  : LCD_ReadData
* Description    : LCD read data
* Input          : None
* Output         : None
* Return         : return data
* Attention  : None
*******************************************************************************/
inline uint16_t LCD_ReadData();

inline uint16_t LCD_ReadPixelColor(uint16_t x, uint16_t y);

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Writes to the selected LCD register.
* Input          : - LCD_Reg: address of the selected register.
*                  - LCD_RegValue: value to write to the selected register.
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
inline void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue);

/*******************************************************************************
* Function Name  : LCD_SetCursor
* Description    : Sets the cursor position.
* Input          : - Xpos: specifies the X position.
*                  - Ypos: specifies the Y position.
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
inline void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos );

/*******************************************************************************
* Function Name  : LCD_Init
* Description    : Configures LCD Control lines, sets whole screen black
* Input          : bool usingTP: determines whether or not to enable TP interrupt
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
void LCD_Init(void);

/*******************************************************************************
 * Function Name  : TP_ReadXY
 * Description    : Obtain X and Y touch coordinates
 * Input          : None
 * Output         : None
 * Return         : Point structure
 * Attention      : None
 *******************************************************************************/
Point TP_ReadXY();

/*******************************************************************************
 * Function Name  : TP_ReadX
 * Description    : Obtain X touch coordinate
 * Input          : None
 * Output         : None
 * Return         : X Coordinate
 * Attention      : None
 *******************************************************************************/
uint16_t TP_ReadX();

/*******************************************************************************
 * Function Name  : TP_ReadY
 * Description    : Obtain Y touch coordinate
 * Input          : None
 * Output         : None
 * Return         : Y Coordinate
 * Attention      : None
 *******************************************************************************/
uint16_t TP_ReadY();

/************************************ Public Functions  *******************************************/




#endif /* LCDLIB_H_ */
