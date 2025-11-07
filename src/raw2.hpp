/* Display driver for Good Display GDEY075Z08
 * RBW (Red/Black/White) e-paper display
 * Based on Good Display example code
 *
 * Board:   LaskaKit ESPink ESP32 e-Paper   https://www.laskakit.cz/laskakit-espink-esp32-e-paper-pcb-antenna/
 * Display: Good Display GDEY075Z08          https://www.laskakit.cz/good-display-gdey075z08-7-5--800x480-epaper-displej-cerveno-cerno-bily/
 *
 * Email:podpora@laskakit.cz
 * Web:laskakit.cz
 */

#include <SPI.h>

// Pin definitions - using LaskaKit ESPink board pinout
#define MISO -1
#define MOSI PIN_EPD_SDA
#define SCK PIN_EPS_SCL
#define CS PIN_EPD_CS
#define DC PIN_EPD_DC
#define RST PIN_EPD_RST
#define BUSY PIN_EPD_BUSY
#define POWER PIN_PWR

// Pin control macros
#define EPD_W21_CS_0 digitalWrite(CS, LOW)
#define EPD_W21_CS_1 digitalWrite(CS, HIGH)
#define EPD_W21_DC_0 digitalWrite(DC, LOW)
#define EPD_W21_DC_1 digitalWrite(DC, HIGH)
#define EPD_W21_RST_0 digitalWrite(RST, LOW)
#define EPD_W21_RST_1 digitalWrite(RST, HIGH)
#define isEPD_W21_BUSY digitalRead(BUSY)

////////FUNCTION DECLARATIONS//////
void driver_delay_us(unsigned int xus);
void driver_delay_xms(unsigned long xms);
void DELAY_S(unsigned int delaytime);
void SPI_Delay(unsigned char xrate);
void SPI_Write(unsigned char value);
void EPD_W21_WriteDATA(unsigned char command);
void EPD_W21_WriteCMD(unsigned char command);
// EPD functions
void EPD_W21_Init(void);
void EPD_init_RBW(void);
void EPD_sleep(void);
void EPD_refresh(void);
void lcd_chkstatus(void);
void PIC_display_Clean(void);

// Tips//
/* When the electronic paper is refreshed in full screen, the picture flicker is a normal phenomenon,
   and the main function is to clear the display afterimage in the previous picture. */
/* When you need to transplant the driver, you only need to change the corresponding IO.
   The BUSY pin is the input mode and the others are the output mode. */

///////////////////DELAY FUNCTIONS////////////////////////////////////////////////////////////////////////
void driver_delay_us(unsigned int xus) // 1us
{
    for (; xus > 1; xus--)
        ;
}

void driver_delay_xms(unsigned long xms) // 1ms
{
    unsigned long i = 0, j = 0;

    for (j = 0; j < xms; j++)
    {
        for (i = 0; i < 256; i++)
            ;
    }
}

void DELAY_S(unsigned int delaytime)
{
    int i, j, k;
    for (i = 0; i < delaytime; i++)
    {
        for (j = 0; j < 4000; j++)
        {
            for (k = 0; k < 222; k++)
                ;
        }
    }
}

//////////////////////SPI FUNCTIONS///////////////////////////////////
void SPI_Delay(unsigned char xrate)
{
    unsigned char i;
    while (xrate)
    {
        for (i = 0; i < 2; i++)
            ;
        xrate--;
    }
}

void SPI_Write(unsigned char value)
{
    SPI.transfer(value);
}

void EPD_W21_WriteCMD(unsigned char command)
{
    SPI_Delay(1);
    EPD_W21_CS_0;
    EPD_W21_DC_0; // command write
    SPI_Write(command);
    EPD_W21_CS_1;
}

void EPD_W21_WriteDATA(unsigned char command)
{
    SPI_Delay(1);
    EPD_W21_CS_0;
    EPD_W21_DC_1; // data write
    SPI_Write(command);
    EPD_W21_CS_1;
}

/////////////////EPD SETTINGS FUNCTIONS/////////////////////
void EPD_W21_Init(void)
{
    EPD_W21_RST_0; // Module reset
    delay(10);     // At least 10ms
    EPD_W21_RST_1;
    delay(10);     // At least 10ms
}

// RBW (Red/Black/White) initialization
void EPD_init_RBW(void)
{
    unsigned char HRES_byte1 = 0x03; // 800
    unsigned char HRES_byte2 = 0x20;
    unsigned char VRES_byte1 = 0x01; // 480
    unsigned char VRES_byte2 = 0xE0;

    EPD_W21_Init(); // Electronic paper IC reset

    EPD_W21_WriteCMD(0x01);     // POWER SETTING
    EPD_W21_WriteDATA(0x07);
    EPD_W21_WriteDATA(0x07);    // VGH=20V,VGL=-20V
    EPD_W21_WriteDATA(0x3f);    // VDH=15V
    EPD_W21_WriteDATA(0x3f);    // VDL=-15V

    // Enhanced display drive
    EPD_W21_WriteCMD(0x06);     // Booster Soft Start
    EPD_W21_WriteDATA(0x17);
    EPD_W21_WriteDATA(0x17);
    EPD_W21_WriteDATA(0x28);
    EPD_W21_WriteDATA(0x17);

    EPD_W21_WriteCMD(0x04);     // Power on
    lcd_chkstatus();            // waiting for the electronic paper IC to release the idle signal

    EPD_W21_WriteCMD(0x00);     // PANEL SETTING
    EPD_W21_WriteDATA(0x0F);    // KWR mode - Red/Black/White (0x0F for RBW, not 0xBF for grayscale)

    EPD_W21_WriteCMD(0x61);        // Resolution setting
    EPD_W21_WriteDATA(HRES_byte1); // source 800
    EPD_W21_WriteDATA(HRES_byte2);
    EPD_W21_WriteDATA(VRES_byte1); // gate 480
    EPD_W21_WriteDATA(VRES_byte2);

    EPD_W21_WriteCMD(0x15);
    EPD_W21_WriteDATA(0x00);

    EPD_W21_WriteCMD(0x50);     // VCOM AND DATA INTERVAL SETTING
    EPD_W21_WriteDATA(0x11);
    EPD_W21_WriteDATA(0x07);

    EPD_W21_WriteCMD(0x60);     // TCON SETTING
    EPD_W21_WriteDATA(0x22);
}

void EPD_refresh(void)
{
    EPD_W21_WriteCMD(0x12);     // DISPLAY REFRESH
    driver_delay_xms(1);        // The delay here is necessary, 200uS at least
    lcd_chkstatus();
}

void EPD_sleep(void)
{
    EPD_W21_WriteCMD(0x50);     // VCOM AND DATA INTERVAL SETTING
    EPD_W21_WriteDATA(0xf7);    // WBRmode:VBDF F7 VBDW 77 VBDB 37 VBDR B7

    EPD_W21_WriteCMD(0x02);     // power off
    lcd_chkstatus();
    delay(100);                 // The delay here is necessary, 200uS at least
    EPD_W21_WriteCMD(0x07);     // deep sleep
    EPD_W21_WriteDATA(0xA5);
}

void PIC_display_Clean(void)
{
    unsigned int i;
    EPD_W21_WriteCMD(0x10);     // Transfer old data (BW plane)
    for (i = 0; i < 48000; i++)
    {
        EPD_W21_WriteDATA(0x00);  // Inverted white (will be inverted to 0xFF)
    }

    EPD_W21_WriteCMD(0x13);     // Transfer new data (RW plane)
    for (i = 0; i < 48000; i++)
    {
        EPD_W21_WriteDATA(0x00);  // No red
    }
}

void lcd_chkstatus(void)
{
    unsigned char busy;
    do
    {
        EPD_W21_WriteCMD(0x71);
        busy = isEPD_W21_BUSY;
        busy = !(busy & 0x01);
    } while (busy);
    driver_delay_xms(200);
}
