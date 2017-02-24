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
    else if (NULL != strstr(in, "temp curve eval"))
    {
        uart_write_string("\tEvalues the temperature curve at one point in time.\n\r\tParameters: <seconds in to reflow program>\n\r\tReturns: <target temperature>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "init flash bufffer"))
    {
        uart_write_string("\tInitiates the flash write buffer with the contents of theflash data memory.\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "buffered write"))
    {
        uart_write_string("\tWrites one byte to the flash buffer.\n\r\tParamters: <index in hex format> <one byte value in hex format>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "flush flash buffer"))
    {
        uart_write_string("\tWrite the contents of the flash buffer to the flash memory.\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "get flash"))
    {
        uart_write_string("\tGets one byte from the flash data memory.\n\r\tParameter: <index in hex format>\n\r\tReturns: <hex value of byte at specified index>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "get kp"))
    {
        uart_write_string("\tGets the Kp constant in the PID regulator.\n\r\tReturns: <Kp value as  a decimal value>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "get ki"))
    {
        uart_write_string("\tGets the Ip constant in the PID regulator.\n\r\tReturns: <Ip value as  a decimal value>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "get kd"))
    {
        uart_write_string("\tGets the Dp constant in the PID regulator.\n\r\tReturns: <Dp value as  a decimal value>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "get ttr"))
    {
        uart_write_string("\tGets the PID integral tracking time constant.\n\r\tReturns: <PID integral tracking time constant>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "get d max gain"))
    {
        uart_write_string("\tGets the max gain of the PID derivate term.\n\r\tReturns: <Max PID derivate part gain>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "get pid servo factor"))
    {
        uart_write_string("\tGets the servo output scaling factor.\n\r\tReturn: <scaling factor as positive float value>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "get start of soak"))
    {
        uart_write_string("\tGets the timestamp in [s] for when the soak period starts.\n\r\tThis is done for the profile selected by the reflow profile switch.\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "get start of reflow"))
    {
        uart_write_string("\tGets the timestamp in [s] for when the reflow period starts.\n\r\tThis is done for the profile selected by the reflow profile switch.\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "get start of cool"))
    {
        uart_write_string("\tGets the timestamp in [s] for when the cooling period starts.\n\r\tThis is done for the profile selected by the reflow profile switch.\n\r\t\n\r");
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
    else if (NULL != strstr(in, "set kp"))
    {
        uart_write_string("\tSets the Kp constant in the PID regulator.\n\r\tParameter: <Value in the range [-65535.0, 65535.0]>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "set ki"))
    {
        uart_write_string("\tSets the Ki constant in the PID regulator.\n\r\tParameter: <Value in the range [-65535.0, 65535.0]>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "set kd"))
    {
        uart_write_string("\tSets the Kd constant in the PID regulator.\n\r\tParameter: <Value in the range [-65535.0, 65535.0]>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "set ttr"))
    {
        uart_write_string("\tSets the integrator tracking time constant in the PID regulator.\n\r\tParameter: <Value in the range [0.0, 65535.0]>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "set d max gain"))
    {
        uart_write_string("\tSets the max gain of the PID derivative term.\n\r\tParameter: <Value in the range [0.0, 65535.0]>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "set pid servo factor"))
    {
        uart_write_string("\tSets how much the servo should extend for negative control values.\n\r\tParamter: <positive float scaling value>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "set heat pwm"))
    {
        uart_write_string("\tActivates the heater PWM and sets the pwm value.\n\r\tParameter: <Duty in range [0, 50]>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "set start of soak"))
    {
        uart_write_string("\tSets the timestamp in [s] for when the soak period starts.\n\r\tThis is done for the profile selected by the reflow profile switch.\n\r\tParameter: <timestamp in seconds>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "set start of reflow"))
    {
        uart_write_string("\tSets the timestamp in [s] for when the reflow period starts.\n\r\tThis is done for the profile selected by the reflow profile switch.\n\r\tParameter: <timestamp in seconds>\n\r\t\n\r");
    }
    else if (NULL != strstr(in, "set start of cool"))
    {
        uart_write_string("\tSets the timestamp in [s] for when the cool period starts.\n\r\tThis is done for the profile selected by the reflow profile switch.\n\r\tParameter: <timestamp in seconds>\n\r\t\n\r");
    }
    else
    {
        uart_write_string("\tType \"help <command>\" for more info\n\r");
        uart_write_string("\tAvailible commands:\n\r");
        uart_write_string("\t------------------------------------\n\r");
        uart_write_string("\tbuffered write\n\r\tflush flash buffer\n\r\tget d max gain\n\r\tget flash\n\r\tget kd\n\r\tget ki\n\r\tget kp\n\r\tget pid servo factor\n\r\tget start of cool\n\r\tget start of reflow\n\r\tget start of soak\n\r\tget ttr\n\r\thello\n\r\tinit flash bufffer\n\r\tset d max gain\n\r\tset flash\n\r\tset heat pwm\n\r\tset heater\n\r\tset kd\n\r\tset ki\n\r\tset kp\n\r\tset pid servo factor\n\r\tset servo pos\n\r\tset start of cool\n\r\tset start of reflow\n\r\tset start of soak\n\r\tset ttr\n\r\ttemp curve eval\n\r\ttest temp\n\r\t");
        uart_write_string("\n\r");
    }
}
