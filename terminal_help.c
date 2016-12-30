/*
This file is an auto generated file.
Do not modify its contents manually!
*/
#include <string.h>
#include <stddef.h>
#include "uart.h"
void terminal_help(char* in)
{
    if (NULL != strstr(in, "hello"))
    {
        uart_write_string("\tSay hi!\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "test temp"))
    {
        uart_write_string("\tReads one value from the first MAX6675 ic.\n\r\tReturns: <read value as 16 bits in hex format>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "get flash"))
    {
        uart_write_string("\tGets one byte from the flash data memory.\n\r\tParameter: <index in hex format>\n\r\tReturns: <hex value of byte at specified index>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "set heater"))
    {
        uart_write_string("\tSets the heater on or off.\n\r\tParameter: <'on' or 'off'>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "set servo pos"))
    {
        uart_write_string("\tSets the servo position.\n\r\tParameter: <position as integer in [0, 1200]>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "set flash"))
    {
        uart_write_string("\tSets one byte in the flash data memory.\n\r\tParamter: <index in hex format> <one byte value in hex format>\n\r\t\n\r");
    }
    else
    {
        uart_write_string("\tType \"help <command>\" for more info\n\r");
        uart_write_string("\tAvailible commands:\n\r");
        uart_write_string("\t------------------------------------\n\r");
        uart_write_string("\tget flash\n\r\thello\n\r\tset flash\n\r\tset heater\n\r\tset servo pos\n\r\ttest temp\n\r\t");
        uart_write_string("\n\r");
    }
}
