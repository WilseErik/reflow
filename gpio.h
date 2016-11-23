#ifndef GPIO_H
#define	GPIO_H

#ifdef	__cplusplus
extern "C" {
#endif

// =============================================================================
// Include statements
// =============================================================================
#include <xc.h>

#include <stdint.h>
#include <stdbool.h>


// =============================================================================
// Public type definitions
// =============================================================================
    
#define DIR_IN 1
#define DIR_OUT 0

//
// Heater
//
#define HEATER_PIN                  LATFbits.LATF0
#define HEATER_DIR                  TRISFbits.TRISF0
#define HEATER_OFF                  do {HEATER_PIN = 0;} while (0)
#define HEATER_ON                   do {HEATER_PIN = 1;} while (0)
    
//
// LEDs
//
#define PREHEAT_LED_PIN             LATGbits.LATG6
#define PREHEAT_LED_DIR             TRISGbits.TRISG6

#define SOAK_LED_PIN                LATGbits.LATG7
#define SOAK_LED_DIR                TRISGbits.TRISG7

#define REFLOW_LED_PIN              LATGbits.LATG8
#define REFLOW_LED_DIR              TRISGbits.TRISG9

#define COOL_LED_PIN                LATGbits.LATG9
#define COOL_LED_DIR                TRISGbits.TRISG9

#define DEBUG_1_LED_PIN             LATBbits.LATB12
#define DEBUG_1_LED_DIR             TRISBbits.TRISB12

#define DEBUG_2_LED_PIN             LATBbits.LATB13
#define DEBUG_2_LED_DIR             TRISBbits.TRISB13

#define DEBUG_3_LED_PIN             LATBbits.LATB14
#define DEBUG_3_LED_DIR             TRISBbits.TRISB14

#define DEBUG_4_LED_PIN             LATBbits.LATB15
#define DEBUG_4_LED_DIR             TRISBbits.TRISB15

//
// Buttons
//
#define START_BUTTON_PIN            PORTDbits.PORTD8
#define START_BUTTON_DIR            TRISDbits.TRISD8

#define STOP_BUTTON_PIN             PORTDbits.PORTD9
#define STOP_BUTTON_DIR             TRISDbits.TRISD9

#define LEAD_SWITCH_PIN             PORTDbits.PORTD10
#define LEAD_SWITCH_DIR             TRISDbits.TRISD10

//
// UART
//
#define UART_TX_PIN                 LATFbits.LATF5
#define UART_TX_DIR                 TRISFbits.TRISF5

#define UART_RX_PIN                 LATFbits.LATF4
#define UART_RX_DIR                 TRISFbits.TRISF4

//
// LCD
//
#define LCD_RS_PIN                  LATDbits.LATD5
#define LCD_RS_DIR                  TRISDbits.TRISD5

#define LCD_RW_PIN_READ             1
#define LCD_RW_PIN_WRITE            0
#define LCD_RW_PIN                  LATDbits.LATD6
#define LCD_RW_DIR                  TRISDbits.TRISD9

#define LCD_E_PIN                   LATDbits.LATD7
#define LCD_E_DIR                   TRISDbits.TRISD7

#define LCD_DATA_PORT_REG           LATE
#define LCD_DATA_DIR_REG            TRISE
#define LCD_DATA_MASK               0xFF00

#define LCD_DB0_PIN                 LATEbits.LATE0
#define LCD_DB0_DIR                 TRISEbits.TRISE0

#define LCD_DB1_PIN                 LATEbits.LATE1
#define LCD_DB1_DIR                 TRISEbits.TRISE1

#define LCD_DB2_PIN                 LATEbits.LATE2
#define LCD_DB2_DIR                 TRISEbits.TRISE2

#define LCD_DB3_PIN                 LATEbits.LATE3
#define LCD_DB3_DIR                 TRISEbits.TRISE3

#define LCD_DB4_PIN                 LATEbits.LATE4
#define LCD_DB4_DIR                 TRISEbits.TRISE4

#define LCD_DB5_PIN                 LATEbits.LATE5
#define LCD_DB5_DIR                 TRISEbits.TRISE5

#define LCD_DB6_PIN                 LATEbits.LATE6
#define LCD_DB6_DIR                 TRISEbits.TRISE6

#define LCD_DB7_PIN                 LATEbits.LATE7
#define LCD_DB7_DIR                 TRISEbits.TRISE7

//
// MAX6675
//
#define MAX6675_MISO_PIN            LATFbits.LATF
#define MAX6675_MISO_DIR            TRISFbits.TRISF2

#define MAX6675_SCK_PIN             LATFbits.LATF6
#define MAX6675_SCK_DIR             TRISFbits.TRISF6

#define MAX6675_1_CS_PIN            LATGbits.LATG3
#define MAX6675_1_CS_DIR            TRISGbits.TRISG3

#define MAX6675_2_CS_PIN            LATGbits.LATG2
#define MAX6675_2_CS_DIR            TRISGbits.TRISG2

#define MAX6675_1_CS_ON             do {MAX6675_1_CS_PIN = 0;} while(0)
#define MAX6675_1_CS_OFF            do {MAX6675_1_CS_PIN = 1;} while(0)
#define MAX6675_2_CS_ON             do {MAX6675_2_CS_PIN = 0;} while(0)
#define MAX6675_2_CS_OFF            do {MAX6675_2_CS_PIN = 1;} while(0)

//
// Servo control
//
#define SERVO_CONTROL_PIN           LATDbits.LATD1
#define SERVO_CONTROL_DIR           TRISDbits.TRISD1

//
// Unused pins
//
#define NC1_DIR                     TRISBbits.TRISB2
#define NC1_PIN                     LATBbits.LATB2

#define NC2_DIR                     TRISBbits.TRISB3
#define NC2_PIN                     LATBbits.LATB3

#define NC3_DIR                     TRISBbits.TRISB4
#define NC3_PIN                     LATBbits.LATB4

#define NC4_DIR                     TRISBbits.TRISB5
#define NC4_PIN                     LATBbits.LATB5

#define NC5_DIR                     TRISBbits.TRISB6
#define NC5_PIN                     LATBbits.LATB6

#define NC6_DIR                     TRISBbits.TRISB7
#define NC6_PIN                     LATBbits.LATB7

#define NC7_DIR                     TRISBbits.TRISB8
#define NC7_PIN                     LATBbits.LATB8

#define NC8_DIR                     TRISBbits.TRISB9
#define NC8_PIN                     LATBbits.LATB9

#define NC9_DIR                     TRISBbits.TRISB10
#define NC9_PIN                     LATBbits.LATB10

#define NC10_DIR                    TRISBbits.TRISB11
#define NC10_PIN                    LATBbits.LATB11

#define NC11_DIR                    TRISCbits.TRISC12
#define NC11_PIN                    LATCbits.LATC12

#define NC12_DIR                    TRISCbits.TRISC13
#define NC12_PIN                    LATCbits.LATC13

#define NC13_DIR                    TRISCbits.TRISC14
#define NC13_PIN                    LATCbits.LATC14

#define NC14_DIR                    TRISCbits.TRISC15
#define NC14_PIN                    LATCbits.LATC15

#define NC15_DIR                    TRISDbits.TRISD0
#define NC15_PIN                    LATDbits.LATD0

#define NC16_DIR                    TRISDbits.TRISD2
#define NC16_PIN                    LATDbits.LATD2

#define NC17_DIR                    TRISDbits.TRISD3
#define NC17_PIN                    LATDbits.LATD3

#define NC18_DIR                    TRISDbits.TRISD4
#define NC18_PIN                    LATDbits.LATD4

#define GPIO_CREATE_NC_NAME(a, b, c) a ## b ## c
#define NC_PIN(n) GPIO_CREATE_NC_NAME(NC, n, _PIN)
#define NC_DIR(n) GPIO_CREATE_NC_NAME(NC, n, _DIR)

// =============================================================================
// Global variable declarations
// =============================================================================

// =============================================================================
// Global constatants
// =============================================================================

// =============================================================================
// Public function declarations
// =============================================================================

/**
 * @brief Sets up the GPIOs to their default state.
 * @param void
 * @return void
 */
void gpio_init(void);

/**
 * @brief Sets the data pins to the LCD.
 * @param data
 */
static inline void gpio_lcd_set_data(uint8_t data)
{
    LCD_DATA_PORT_REG &= ~LCD_DATA_MASK;
    LCD_DATA_PORT_REG |= data & LCD_DATA_MASK;
}


#ifdef	__cplusplus
}
#endif

#endif	/* GPIO_H */

