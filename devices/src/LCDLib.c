/*
 * LCDLib.c
 *
 *  Created on: Mar 2, 2017
 *      Author: Danny
 */

#include "LCDLib.h"
//#include "msp.h"
#include "driverlib.h"
#include "msp430.h"
#include "AsciiLib.h"

/* SPI CONFIGs */
#define DEFAULT_SMCLK   16777216
#define LCD_CLK         8388608
//#define TP_CLK          1200000

static USCI_B_SPI_initMasterParam LCD_SPI_CONFIG = {
    USCI_B_SPI_CLOCKSOURCE_SMCLK,
    DEFAULT_SMCLK, //16.7MHz source clock
    LCD_CLK, //8.3MHz desired clock
    USCI_B_SPI_MSB_FIRST,
    USCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT,
    USCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH
};

static Timer_A_outputPWMParam LCD_PWM_CONFIG = {
                                                TIMER_A_CLOCKSOURCE_SMCLK,
                                                TIMER_A_CLOCKSOURCE_DIVIDER_1,
                                                65535,
                                                LCD_PWM_REG,
                                                TIMER_A_OUTPUTMODE_TOGGLE,
                                                30000 //start backlight around middle
};


//static const eUSCI_SPI_MasterConfig TP_SPI_CONFIG = {
//    EUSCI_SPI_CLOCKSOURCE_SMCLK,
//    DEFAULT_SMCLK, //12MHz source clock
//    TP_CLK, //1.2MHz desired clock
//    EUSCI_SPI_MSB_FIRST,
//    EUSCI_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT,
//    EUSCI_SPI_CLOCKPOLARITY_INACTIVITY_HIGH,
//    EUSCI_SPI_3PIN
//};

/* COMMAND SETTINGS FOR LCD INITIALIZATION, DEFINED BY VENDOR IN CODE LIBRARY */

static const uint8_t Rcmd1[] = {                       // 7735R init, part 1 (red or green tab)
                                                       15,                             // 15 commands in list:
                                                       ST77XX_SWRESET,   ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
                                                         150,                          //     150 ms delay
                                                       ST77XX_SLPOUT,    ST_CMD_DELAY, //  2: Out of sleep mode, 0 args, w/delay
                                                         255,                          //     500 ms delay
                                                       ST7735_FRMCTR1, 3,              //  3: Framerate ctrl - normal mode, 3 arg:
                                                         0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
                                                       ST7735_FRMCTR2, 3,              //  4: Framerate ctrl - idle mode, 3 args:
                                                         0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
                                                       ST7735_FRMCTR3, 6,              //  5: Framerate - partial mode, 6 args:
                                                         0x01, 0x2C, 0x2D,             //     Dot inversion mode
                                                         0x01, 0x2C, 0x2D,             //     Line inversion mode
                                                       ST7735_INVCTR,  1,              //  6: Display inversion ctrl, 1 arg:
                                                         0x07,                         //     No inversion
                                                       ST7735_PWCTR1,  3,              //  7: Power control, 3 args, no delay:
                                                         0xA2,
                                                         0x02,                         //     -4.6V
                                                         0x84,                         //     AUTO mode
                                                       ST7735_PWCTR2,  1,              //  8: Power control, 1 arg, no delay:
                                                         0xC5,                         //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
                                                       ST7735_PWCTR3,  2,              //  9: Power control, 2 args, no delay:
                                                         0x0A,                         //     Opamp current small
                                                         0x00,                         //     Boost frequency
                                                       ST7735_PWCTR4,  2,              // 10: Power control, 2 args, no delay:
                                                         0x8A,                         //     BCLK/2,
                                                         0x2A,                         //     opamp current small & medium low
                                                       ST7735_PWCTR5,  2,              // 11: Power control, 2 args, no delay:
                                                         0x8A, 0xEE,
                                                       ST7735_VMCTR1,  1,              // 12: Power control, 1 arg, no delay:
                                                         0x0E,
                                                       ST77XX_INVOFF,  0,              // 13: Don't invert display, no args
                                                       ST77XX_MADCTL,  1,              // 14: Mem access ctl (directions), 1 arg:
                                                         0xC8,                         //     row/col addr, bottom-top refresh
                                                       ST77XX_COLMOD,  1,              // 15: set color mode, 1 arg, no delay:
                                                         0x05 };                       //     16-bit color

static const uint8_t Rcmd2green[] = {                  // 7735R init, part 2 (green tab only)
                                                       2,                              //  2 commands in list:
                                                       ST77XX_CASET,   4,              //  1: Column addr set, 4 args, no delay:
                                                         0x00, 0x02,                   //     XSTART = 0
                                                         0x00, 0x7F+0x02,              //     XEND = 127
                                                       ST77XX_RASET,   4,              //  2: Row addr set, 4 args, no delay:
                                                         0x00, 0x01,                   //     XSTART = 0
                                                         0x00, 0x9F+0x01 };            //     XEND = 159

static const uint8_t Rcmd3[] = {                       // 7735R init, part 3 (red or green tab)
                                                       4,                              //  4 commands in list:
                                                       ST7735_GMCTRP1, 16      ,       //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
                                                         0x02, 0x1c, 0x07, 0x12,       //     (Not entirely necessary, but provides
                                                         0x37, 0x32, 0x29, 0x2d,       //      accurate colors)
                                                         0x29, 0x25, 0x2B, 0x39,
                                                         0x00, 0x01, 0x03, 0x10,
                                                       ST7735_GMCTRN1, 16      ,       //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
                                                         0x03, 0x1d, 0x07, 0x06,       //     (Not entirely necessary, but provides
                                                         0x2E, 0x2C, 0x29, 0x2D,       //      accurate colors)
                                                         0x2E, 0x2E, 0x37, 0x3F,
                                                         0x00, 0x00, 0x02, 0x10,
                                                       ST77XX_NORON,     ST_CMD_DELAY, //  3: Normal display on, no args, w/delay
                                                         10,                           //     10 ms delay
                                                       ST77XX_DISPON,    ST_CMD_DELAY, //  4: Main screen turn on, no args w/delay
                                                         100 };                        //     100 ms delay

/************************************  Private Functions  *******************************************/

/*
 * Delay x ms
 */
static void Delay(unsigned long interval)
{
    while(interval > 0)
    {
        //__delay_cycles(48000);
        __delay_cycles(16777);
        interval--;
    }
}

static void sendCommand(uint8_t commandByte, const uint8_t *dataBytes, uint8_t numDataBytes) {
  //SPI_BEGIN_TRANSACTION();

  SPI_CS_LOW;

  SPI_DC_LOW;          // Command mode
  SPISendRecvByte(commandByte); // Send the command byte
  SPI_DC_HIGH;
  for (int i = 0; i < numDataBytes; i++) {
     SPISendRecvByte(*dataBytes); // Send the data bytes
     dataBytes++;
  }

  SPI_CS_HIGH;
  //SPI_END_TRANSACTION();
}

static void LCD_writeCommand(uint8_t command) {
    SPI_DC_LOW;
    SPISendRecvByte(command);
    SPI_DC_HIGH;
}

static void LCD_initDisplay(const uint8_t* commands) {

   uint8_t numCommands, cmd, numArgs;
   uint16_t ms;

   numCommands = *commands; // Number of commands to follow
   commands++;
   while (numCommands--) {              // For each command...
     cmd = *commands;       // Read command
     commands++;
     numArgs = *commands;   // Number of args to follow
     commands++;
     ms = numArgs & ST_CMD_DELAY;       // If hibit set, delay follows args
     numArgs &= ~ST_CMD_DELAY;          // Mask out delay bit
     sendCommand(cmd, commands, numArgs); //FIXME: define sendCommand
     commands += numArgs;

     if (ms) {
       ms = *commands; // Read post-command delay time (ms)
       commands++;
        if (ms == 255)
            ms = 500; // If 255, delay for 500 ms
            Delay(ms);
     }
   }
}

void LCD_setAddrWindow(uint16_t x, uint16_t y, uint16_t w,
                  uint16_t h) {
    x += 1;
    y += 1;
    uint32_t xaddr = ((uint32_t)x << 16) | (x + w - 1);
    uint32_t yaddr = ((uint32_t)y << 16) | (y + h - 1);

    LCD_writeCommand(ST77XX_CASET); // Column addr set
    LCD_Write_Data_Only((uint16_t) ((xaddr >> 16) & 0xFFFF)); //write upper 16 bits
    LCD_Write_Data_Only((uint16_t) (xaddr & 0xFFFF)); //write lower 16 bits

    LCD_writeCommand(ST77XX_RASET); // Row addr set
    LCD_Write_Data_Only((uint16_t) ((yaddr >> 16) & 0xFFFF)); //write upper 16 bits
    LCD_Write_Data_Only((uint16_t) (yaddr & 0xFFFF)); //write lower 16 bits

    LCD_writeCommand(ST77XX_RAMWR); // write to RAM

}


/*******************************************************************************
 * Function Name  : LCD_initSPI
 * Description    : Configures LCD Control lines
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
static void LCD_initSPI() //FIXME: Write new code to initialize SPI
{
    /* P10.1 - CLK
     * P10.2 - MOSI
     * P10.3 - MISO
     * P10.4 - LCD CS 
     * P10.5 - TP CS 
     */

    USCI_B_SPI_initMaster(USCI_B1_BASE, &LCD_SPI_CONFIG);

    //
    //REMAP pins to use SPI
    #ifdef __MSP430F6438
        P8SEL |= BIT4 | BIT5 | BIT6;

    #elif defined __MSP430F5529__
        P4SEL |= BIT1 | BIT2 | BIT3;

    #endif

    //set LCD CS, LCD RESET, CARD CS, and LCD DC as outputs

    //initialize chip selects, resets and DC to 1
    GPIO_setOutputHighOnPin(LCD_PORT_IO, LCD_RESET);
    GPIO_setOutputHighOnPin(LCD_PORT_IO, LCD_CARD_CS);
    GPIO_setOutputHighOnPin(LCD_PORT_IO, LCD_DC);
    GPIO_setOutputHighOnPin(LCD_TFT_CS_PORT, LCD_TFT_CS);

    GPIO_setAsOutputPin(LCD_PORT_IO, LCD_RESET);
    GPIO_setAsOutputPin(LCD_PORT_IO, LCD_CARD_CS);
    GPIO_setAsOutputPin(LCD_PORT_IO, LCD_DC);
    GPIO_setAsOutputPin(LCD_TFT_CS_PORT, LCD_TFT_CS);

    USCI_B_SPI_enable(USCI_B1_BASE);

    //Init IO

    /*

    //configure port 10 to use UCB3 SPI mode
    P10->SEL0 |= BIT3 | BIT2 | BIT1;
    P10->SEL1 &= ~(BIT3 | BIT2 | BIT1);

    //Configure UCB3CTLW0
    SPI_initMaster(EUSCI_B3_BASE, &LCD_SPI_CONFIG);

    //configure port 10.4 and 10.5 as CS with 1 by default
    P10->DIR |= BIT4 | BIT5;
    P10->OUT |= BIT4 | BIT5;

    //enable SPI
    SPI_enableModule(EUSCI_B3_BASE);
    */
}

/*******************************************************************************
 * Function Name  : LCD_reset
 * Description    : Resets LCD
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : Uses P10.0 for reset
 *******************************************************************************/
static void LCD_reset()
{
    GPIO_setOutputLowOnPin(LCD_PORT_IO, LCD_RESET); //set reset pin low to trigger hardware reset
    Delay(200);
    GPIO_setOutputHighOnPin(LCD_PORT_IO, LCD_RESET); //set reset pin high to end hardware reset
    Delay(200);
    /*
    P10DIR |= BIT0;
    P10OUT |= BIT0;  // high
    Delay(100);
    P10OUT &= ~BIT0; // low
    Delay(100);
    P10OUT |= BIT0;  // high
    */
}

/************************************  Private Functions  *******************************************/


/************************************  Public Functions  *******************************************/

/*******************************************************************************
 * Function Name  : LCD_DrawRectangle
 * Description    : Draw a rectangle as the specified color
 * Input          : xStart, xEnd, yStart, yEnd, Color
 * Output         : None
 * Return         : None
 * Attention      : Must draw from left to right, top to bottom!
 *******************************************************************************/
void LCD_DrawRectangle(int16_t xStart, int16_t xEnd, int16_t yStart, int16_t yEnd, uint16_t Color)
{

    /* Check special cases for out of bounds */
    if (xEnd >= MAX_SCREEN_X || yEnd >= MAX_SCREEN_Y || xStart < MIN_SCREEN_X || yEnd < MIN_SCREEN_Y);

    else {
        /* Set window area for high-speed RAM write */
        //LCD_WriteReg(HOR_ADDR_START_POS, yStart);     /* Horizontal GRAM Start Address */
        //LCD_WriteReg(HOR_ADDR_END_POS, yEnd);  /* Horizontal GRAM End Address */
        //LCD_WriteReg(VERT_ADDR_START_POS, xStart);    /* Vertical GRAM Start Address */
        //LCD_WriteReg(VERT_ADDR_END_POS, xEnd); /* Vertical GRAM Start Address */

        /* Set cursor */
        //LCD_SetCursor(xStart,yStart);

        /* Set index to GRAM */
        //LCD_WriteIndex(GRAM);

        /* Send out data only to the entire area */

        SPI_CS_LOW;
        //LCD_Write_Data_Start();
        LCD_setAddrWindow(xStart, yStart, xEnd-xStart+1, yEnd-yStart+1);

        //write out color data only screenSize times
        uint32_t screenSize = ((uint32_t)(xEnd-xStart+1)) * ((uint32_t)(yEnd-yStart+1));
        int i = 0;
        for (i = 0; i < screenSize; i++) {
            LCD_Write_Data_Only(Color);
        }
        SPI_CS_HIGH;
    }
}

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
inline void PutChar( uint16_t Xpos, uint16_t Ypos, uint8_t ASCI, uint16_t charColor)
{
    uint16_t i, j;
    uint8_t buffer[16], tmp_char;
    GetASCIICode(buffer,ASCI);  /* get font data */
    for( i=0; i<16; i++ )
    {
        tmp_char = buffer[i];
        for( j=0; j<8; j++ )
        {
            if( (tmp_char >> 7 - j) & 0x01 == 0x01 )
            {
                LCD_SetPoint( Xpos + j, Ypos + i, charColor );  /* Character color */
            }
        }
    }
}

/******************************************************************************
 * Function Name  : GUI_Text
 * Description    : Displays the string
 * Input          : - Xpos: Horizontal coordinate
 *                  - Ypos: Vertical coordinate
 *                  - str: Displayed string
 *                  - charColor: Character color
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void LCD_Text(uint16_t Xpos, uint16_t Ypos, uint8_t *str, uint16_t Color)
{
    uint8_t TempChar;

    /* Set area back to span the entire LCD */
    //LCD_WriteReg(HOR_ADDR_START_POS, 0x0000);     /* Horizontal GRAM Start Address */
    //LCD_WriteReg(HOR_ADDR_END_POS, (MAX_SCREEN_Y - 1));  /* Horizontal GRAM End Address */
    //LCD_WriteReg(VERT_ADDR_START_POS, 0x0000);    /* Vertical GRAM Start Address */
    //LCD_WriteReg(VERT_ADDR_END_POS, (MAX_SCREEN_X - 1)); /* Vertical GRAM Start Address */
    do
    {
        TempChar = *str++;
        PutChar( Xpos, Ypos, TempChar, Color);
        if( Xpos < MAX_SCREEN_X - 8)
        {
            Xpos += 8;
        }
        else if ( Ypos < MAX_SCREEN_X - 16)
        {
            Xpos = 0;
            Ypos += 16;
        }
        else
        {
            Xpos = 0;
            Ypos = 0;
        }
    }
    while ( *str != 0 );
}


/*******************************************************************************
 * Function Name  : LCD_Clear
 * Description    : Fill the screen as the specified color
 * Input          : - Color: Screen Color
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void LCD_Clear(uint16_t Color)
{
    /* Set area back to span the entire LCD */
    //LCD_WriteReg(HOR_ADDR_START_POS, MIN_SCREEN_Y);
    //LCD_WriteReg(HOR_ADDR_END_POS, MAX_SCREEN_Y-1);
    //LCD_WriteReg(VERT_ADDR_START_POS, MIN_SCREEN_X);
    //LCD_WriteReg(VERT_ADDR_END_POS, MAX_SCREEN_X-1);

    /* Set cursor to (0,0) */ 
    //LCD_SetCursor(0,0);

    /* Set write index to GRAM */
    //LCD_WriteIndex(GRAM);

    /* Start data transmittion */ 
    SPI_CS_LOW;
    //LCD_Write_Data_Start();

    LCD_setAddrWindow(col_start, row_start, MAX_SCREEN_Y, MAX_SCREEN_X);

    //write out color data only SCREEN_SIZE times
    int i = 0;
    for (i = 0; i < SCREEN_SIZE; i++) {
        LCD_Write_Data_Only(Color);
    }
    SPI_CS_HIGH;

    // You'll need to call LCD_Write_Data_Start() and then send out only data to fill entire screen with color 
}

/******************************************************************************
 * Function Name  : LCD_SetPoint
 * Description    : Drawn at a specified point coordinates
 * Input          : - Xpos: Row Coordinate
 *                  - Ypos: Line Coordinate
 * Output         : None
 * Return         : None
 * Attention      : 18N Bytes Written
 *******************************************************************************/
void LCD_SetPoint(uint16_t Xpos, uint16_t Ypos, uint16_t color)
{
    /* Should check for out of bounds */ 
    if (Xpos >= MAX_SCREEN_X || Ypos >= MAX_SCREEN_Y);

    else {
        SPI_CS_LOW;
        LCD_setAddrWindow(Xpos, Ypos, 1, 1);
        LCD_Write_Data_Only(color);
        SPI_CS_HIGH;
    }
}

/*******************************************************************************
 * Function Name  : LCD_Write_Data_Only
 * Description    : Data writing to the LCD controller
 * Input          : - data: data to be written
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
/*
inline void LCD_Write_Data_Only(uint16_t data)
{
    SPISendRecvByte(data & 0x00FF);
    SPISendRecvByte (data>>8);

}
*/

inline void LCD_Write_Data_Only(uint16_t data)
{

    SPISendRecvByte(data>>8);

    SPISendRecvByte(data & 0x00FF);

}

/*******************************************************************************
 * Function Name  : LCD_WriteData
 * Description    : LCD write register data
 * Input          : - data: register data
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_WriteData(uint16_t data)
{
    SPI_CS_LOW;

    SPISendRecvByte(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0       */
    SPISendRecvByte((data >>   8));                    /* Write D8..D15                */
    SPISendRecvByte((data & 0xFF));                    /* Write D0..D7                 */

    SPI_CS_HIGH;
}

/*******************************************************************************
 * Function Name  : LCD_WriteReg
 * Description    : Reads the selected LCD Register.
 * Input          : None
 * Output         : None
 * Return         : LCD Register Value.
 * Attention      : None
 *******************************************************************************/
inline uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
    /* Write 16-bit Index */
    LCD_WriteIndex(LCD_Reg);

    /* Return 16-bit Reg using LCD_ReadData() */
    return LCD_ReadData();
}

/*******************************************************************************
 * Function Name  : LCD_WriteIndex
 * Description    : LCD write register address
 * Input          : - index: register address
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_WriteIndex(uint16_t index)
{
    SPI_CS_LOW;

    /* SPI write data */
    SPISendRecvByte(SPI_START | SPI_WR | SPI_INDEX);   /* Write : RS = 0, RW = 0  */
    SPISendRecvByte(0);
    SPISendRecvByte(index);

    SPI_CS_HIGH;
}

/*******************************************************************************
 * Function Name  : SPISendRecvByte
 * Description    : Send one byte then receive one byte of response
 * Input          : uint8_t: byte
 * Output         : None
 * Return         : Recieved value 
 * Attention      : None
 *******************************************************************************/
inline uint8_t SPISendRecvByte (uint8_t byte)
{
    USCI_B_SPI_transmitData(USCI_B1_BASE, byte);
    while(!(UCB1IFG & UCRXIFG));
    UCB1IFG &= ~UCRXIFG;
    return USCI_B_SPI_receiveData(USCI_B1_BASE);

    /* Send byte of data */
    //SPI_transmitData(EUSCI_B3_BASE, byte);

    /* Wait as long as busy */ 
    //while(!(UCB3IFG & UCRXIFG0));

    //UCB3IFG &= ~UCRXIFG0;

    /* Return received value*/
    //return SPI_receiveData(EUSCI_B3_BASE);
}

/*******************************************************************************
 * Function Name  : LCD_Write_Data_Start
 * Description    : Start of data writing to the LCD controller
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_Write_Data_Start(void)
{
    SPISendRecvByte(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0 */
}

/*******************************************************************************
 * Function Name  : LCD_ReadData
 * Description    : LCD read data
 * Input          : None
 * Output         : None
 * Return         : return data
 * Attention      : Diagram (d) in datasheet
 *******************************************************************************/
inline uint16_t LCD_ReadData()
{
    uint16_t value;
    SPI_CS_LOW;

    SPISendRecvByte(SPI_START | SPI_RD | SPI_DATA);   /* Read: RS = 1, RW = 1   */
    SPISendRecvByte(0);                               /* Dummy read 1           */
    SPISendRecvByte(0);
    SPISendRecvByte(0);
    SPISendRecvByte(0);
    SPISendRecvByte(0);

    value = (SPISendRecvByte(0) << 8);                /* Read D8..D15           */
    value |= SPISendRecvByte(0);                      /* Read D0..D7            */

    SPI_CS_HIGH;
    return value;
}

inline uint16_t LCD_ReadPixelColor(uint16_t x, uint16_t y) {
    LCD_SetCursor(x,y);
    return LCD_ReadData();
}

/*******************************************************************************
 * Function Name  : LCD_WriteReg
 * Description    : Writes to the selected LCD register.
 * Input          : - LCD_Reg: address of the selected register.
 *                  - LCD_RegValue: value to write to the selected register.
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{
    /* Write 16-bit Index */
    LCD_WriteIndex(LCD_Reg);

    /* Write 16-bit Reg Data */
    LCD_WriteData(LCD_RegValue);

    SPI_CS_HIGH;
}

/*******************************************************************************
 * Function Name  : LCD_SetCursor
 * Description    : Sets the cursor position.
 * Input          : - Xpos: specifies the X position.
 *                  - Ypos: specifies the Y position.
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
//inline void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos )
//{
    /* Should just be two LCD_WriteReg to appropriate registers */ 
    /* Set horizonal GRAM coordinate (Ypos) */ 
//    LCD_WriteReg(HORIZONTAL_GRAM_SET, Ypos);

    /* Set vertical GRAM coordinate (Xpos) */
//    LCD_WriteReg(VERTICAL_GRAM_SET, Xpos);
//}

/*******************************************************************************
 * Function Name  : LCD_Init
 * Description    : Configures LCD Control lines, sets whole screen black
 * Input          : bool usingTP: determines whether or not to enable TP interrupt 
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void LCD_Init(void)
{

    LCD_initSPI();
    LCD_reset();

    LCD_initDisplay(Rcmd1);
    LCD_initDisplay(Rcmd2green);
    LCD_initDisplay(Rcmd3);

    //uint8_t madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST7735_MADCTL_BGR;
    //sendCommand(ST77XX_MADCTL, &madctl, 1);

    //LCD_reset();

    LCD_Clear(LCD_BLACK);

    //set PWM for LCD backlight
    //Timer_A_outputPWM(LCD_PWM_TIMER, &LCD_PWM_CONFIG);
    Timer_B_initCompareModeParam compareParam = {
                                                 TIMER_B_CAPTURECOMPARE_REGISTER_1,
                                                 TIMER_B_CAPTURECOMPARE_INTERRUPT_DISABLE,
                                                 TIMER_B_OUTPUTMODE_RESET_SET,
                                                 10000
    };

    //OUTMOD_0 + OUT
    //OUTMOD_5 + OUT

    Timer_B_initContinuousModeParam continuousParam = {
                                                       TIMER_B_CLOCKSOURCE_SMCLK,
                                                       TIMER_B_CLOCKSOURCE_DIVIDER_1,
                                                       TIMER_B_TBIE_INTERRUPT_DISABLE,
                                                       TIMER_B_DO_CLEAR,
                                                       true
    };
    Timer_B_initContinuousMode(TIMER_B0_BASE, &continuousParam);
    Timer_B_initCompareMode(TIMER_B0_BASE, &compareParam);
    Timer_B_setCompareValue(TIMER_B0_BASE, TIMER_B_CAPTURECOMPARE_REGISTER_0, 0xFFFF);

#ifdef __MSP430F5529__
    P1DIR |= BIT2;
    P1SEL |= BIT2;
#elif defined __MSP430F6438__

    //Timer_A_startCounter(LCD_PWM_TIMER, TIMER_A_CONTINUOUS_MODE); //use continuous mode to use ccr0
    P4DIR |= BIT1;
    P4SEL |= BIT1;
#endif
}

#pragma vector=TIMER1_A1_VECTOR
__interrupt void TIMERA1_OVF(void) {
    if (TA1IV & TA1IV_TAIFG) {//check for overflow flag
        Timer_A_setOutputForOutputModeOutBitValue(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0,
                                                  TIMER_A_OUTPUTMODE_OUTBITVALUE_HIGH);

    }
}

void LCD_WriteInt(uint16_t Xpos, uint16_t Ypos, uint32_t val, uint16_t Color) {
    char num_str[16]; //create output array to write to LCD
    uint8_t i = 0;
    for (i = 0; i < 16; i++) {
        num_str[i] = 0;
    }

    i = 0;
    while(val) {
        //generate integer part of number
        num_str[i] = (val % 10) + 0x30;
        val /= 10;
        i++;
    }
    char temp;
    for (uint8_t k = 0; k < (i+1)/2; k++) { //reverse string
        temp = num_str[k];
        num_str[k] = num_str[i-1-k];
        num_str[i-1-k] = temp;
    }

    LCD_Text(Xpos, Ypos, num_str, Color);
}


/*
 * Increments duty cycle value in timer by defined inc dec value
 */
inline void LCD_incBacklight(void) {
    uint16_t currDutyCycle = TB0CCR1; //Timer_A_getCaptureCompareCount(LCD_PWM_TIMER, LCD_PWM_REG);
    currDutyCycle = (currDutyCycle + LCD_PWM_INCDEC) < currDutyCycle ? 0xFFFF : currDutyCycle + LCD_PWM_INCDEC;
    TB0CCR1 = currDutyCycle; //Timer_A_setCompareValue(LCD_PWM_TIMER, LCD_PWM_REG, currDutyCycle);
}

/*
 * Decrements duty cycle value in timer by defined inc dec value.
 */
inline void LCD_decBacklight(void) {
    uint16_t currDutyCycle = TB0CCR1; //Timer_A_getCaptureCompareCount(LCD_PWM_TIMER, LCD_PWM_REG);
    currDutyCycle = (currDutyCycle - LCD_PWM_INCDEC) > currDutyCycle ? 0x0000 : currDutyCycle - LCD_PWM_INCDEC;
    TB0CCR1 = currDutyCycle; //Timer_A_setCompareValue(LCD_PWM_TIMER, LCD_PWM_REG, currDutyCycle);
}


/************************************  Public Functions  *******************************************/

