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
    else
    {
        uart_write_string("\tType \"help <command>\" for more info\n\r");
        uart_write_string("\tAvailible commands:\n\r");
        uart_write_string("\t------------------------------------\n\r");
        uart_write_string("\thello\n\r\t");
        uart_write_string("\n\r");
    }
}
