// =============================================================================
// Include statements
// =============================================================================
#include <xc.h>

#include <stdint.h>
#include <stdbool.h>

#include "gpio.h"

// =============================================================================
// Private type definitions
// =============================================================================

// =============================================================================
// Global variables
// =============================================================================

// =============================================================================
// Private constants
// =============================================================================

// =============================================================================
// Private variables
// =============================================================================

// =============================================================================
// Private function declarations
// =============================================================================

// =============================================================================
// Public function definitions
// =============================================================================

void gpio_init(void)
{
    HEATER_DIR          = DIR_OUT;
    HEATER_OFF;

    PREHEAT_LED_DIR     = DIR_OUT;
    SOAK_LED_DIR        = DIR_OUT;
    REFLOW_LED_DIR      = DIR_OUT;
    COOL_LED_DIR        = DIR_OUT;

    DEBUG_1_LED_DIR     = DIR_OUT;
    DEBUG_2_LED_DIR     = DIR_OUT;
    DEBUG_3_LED_DIR     = DIR_OUT;
    DEBUG_4_LED_DIR     = DIR_OUT;

    START_BUTTON_DIR    = DIR_IN;
    STOP_BUTTON_DIR     = DIR_IN;
    LEAD_SWITCH_DIR     = DIR_IN;

    UART_TX_DIR         = DIR_OUT;
    UART_TX_PIN         = 1;
    UART_RX_DIR         = DIR_IN;

    LCD_RS_DIR          = DIR_OUT;
    LCD_RS_PIN          = 0;
    LCD_RW_DIR          = DIR_OUT;
    LCD_RW_PIN          = LCD_RW_PIN_WRITE;
    LCD_E_DIR           = DIR_OUT;
    LCD_E_PIN           = 0;
    LCD_DB0_DIR         = DIR_OUT;
    LCD_DB1_DIR         = DIR_OUT;
    LCD_DB2_DIR         = DIR_OUT;
    LCD_DB3_DIR         = DIR_OUT;
    LCD_DB4_DIR         = DIR_OUT;
    LCD_DB5_DIR         = DIR_OUT;
    LCD_DB6_DIR         = DIR_OUT;
    LCD_DB7_DIR         = DIR_OUT;
    gpio_lcd_set_data(0x00);

    MAX6675_MISO_DIR    = DIR_IN;
    MAX6675_SCK_DIR     = DIR_IN;
    MAX6675_1_CS_DIR    = DIR_OUT;
    MAX6675_1_CS_OFF;
    MAX6675_2_CS_DIR    = DIR_OUT;
    MAX6675_2_CS_OFF;

    SERVO_CONTROL_DIR   = DIR_OUT;

#define GPIO_NC_CURRENT 1
#define GPIO_NC_END     18
#include "gpio_set_nc_pins.h"
}

// =============================================================================
// Private function definitions
// =============================================================================



