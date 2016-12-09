/*
 * This file allows the use of a part of the instruction memory to be used
 * as non volatile data memory.
 *
 * In this implemententation only the 16 MSB of each instruction (24 bits) will
 * be used for data storage. This of course consumes more memory, but speeds
 * up flash address calculation wich in turn imporves read speeds significantly.
 *
 * One instruction in the flash memory:
 *
 *    -------------------------------------------------------------
 *   |        Used as data memory             |       Unused       |
 *    -------------------------------------------------------------
 *             BIT0    -    BIT15                  BIT16 -  BIT23
 *
 *
 * 
 */




// =============================================================================
// Include statements
// =============================================================================
#include "flash.h"

#include <stdbool.h>
#include <stdint.h>

#include <xc.h>

// =============================================================================
// Private type definitions
// =============================================================================

// =============================================================================
// Global variables
// =============================================================================

// =============================================================================
// Private constants
// =============================================================================

#define DATA_SECTOR_ADDR 0

// =============================================================================
// Private variables
// =============================================================================

static volatile uint8_t buffer[FLASH_MEM_SIZE];

// =============================================================================
// Private function declarations
// =============================================================================

// =============================================================================
// Public function definitions
// =============================================================================

uint8_t flash_read_byte(flash_index_t index)
{
    uint16_t data;
    uint8_t ret_val;
    data = flash_read_word(index & 0x0001);

    if (index & 0x0001)
    {
        ret_val = (uint8_t)data;
    }
    else
    {
        ret_val = (uint8_t)(data >> 8);
    }

    return ret_val;
}

uint16_t flash_read_word(flash_index_t index)
{
    uint16_t addr_offset;
    uint16_t read_word;
    uint16_t addr = DATA_SECTOR_ADDR + (uint16_t)index;
    bool odd_addr = (0 != (addr & 0x0001));

    TBLPAG = ((addr & 0x7F0000) >> 16);
    addr_offset = (addr & 0x00FFFE);
    asm("tblrdh.w [%1], %0" : "=r"(read_word) : "r"(addr_offset));

    if (odd_addr)
    {
        uint16_t high_word;

        TBLPAG = ((addr & 0x7F0000) >> 16);
        addr_offset = (addr & 0x00FFFF);
        asm("tblrdh.w [%1], %0" : "=r"(high_word) : "r"(addr_offset));

        read_word = (read_word << 8) | (0x00FF & (high_word >> 8));
    }

    return read_word;
}

uint32_t flash_read_dword(flash_index_t index)
{
    uint32_t dword = 0x00000000;

    dword |= ((uint32_t)flash_read_word(index) << 16) & (uint32_t)0xFFFF0000;
    dword |=  (uint32_t)flash_read_word(index)        & (uint32_t)0x0000FFFF;

    return dword;
}

void flash_init_write_buffer(void)
{
    uint16_t i;

    for (i = 0; i != FLASH_MEM_SIZE; i += 2)
    {
        uint16_t d = flash_read_word(i);
        buffer[i] = d >> 8;
        buffer[i + 1] = d;
    }
}

void flash_write_byte_to_buffer(flash_index_t index, uint8_t data)
{
    buffer[index] = data;
}

void flash_write_word_to_buffer(flash_index_t index, uint16_t data)
{
    buffer[index]     = (uint8_t)((data >> 8) & 0xFF);
    buffer[index + 1] = (uint8_t)( data       & 0xFF);
}

void flash_write_dword_to_buffer(flash_index_t index, uint32_t data)
{
    buffer[index]     = (uint8_t)((data >> 24) & 0xFF);
    buffer[index + 1] = (uint8_t)((data >> 16) & 0xFF);
    buffer[index + 2] = (uint8_t)((data >> 8 ) & 0xFF);
    buffer[index + 3] = (uint8_t)( data        & 0xFF);
}

/*
 * From DS39715A-page 4-17:
 *

The user can program one row of program Flash memory at a time. To do this, it is necessary to
erase the 8-row erase block containing the desired row. The general process is:
1. Read eight rows of program memory (512 instructions) and store in data RAM.
2. Update the program data in RAM with the desired new data.
3. Erase the block:
a) Set the NVMOP bits (NVMCOM<3:0>) to ?0010? to configure for block erase. Set the
ERASE (NVMCOM<6>) and WREN (NVMCOM<14>) bits.
b) Write the starting address of the block to be erased into the TBLPAG and W registers.
c) Write 55h to NVMKEY.
d) Write AAh to NVMKEY.
e) Set the WR bit (NVMCOM<15>). The erase cycle begins and the CPU stalls for the
duration of the erase cycle. When the erase is done, the WR bit is cleared automatically.
4. Write the first 64 instructions from data RAM into the program memory buffers (see Section 4.5
?Program Memory Writes?).
5. Write the program block to Flash memory:
a) Set the NVMOP bits to ?0001? to configure for row programming. Clear the ERASE
bit and set the WREN bit.
b) Write 55h to NVMKEY.
c) Write AAh to NVMKEY.
d) Set the WR bit. The programming cycle begins and the CPU stalls for the duration of the
write cycle. When the write to Flash memory is done, the WR bit is cleared automatically.
6. Repeat steps 4 and 5, using the next available 64 instructions from the block in data RAM
by incrementing the value in TBLPAG, until all 512 instructions are written back to Flash
memory.
For protection against accidental operations, the write initiate sequence for NVMKEY must be
used to allow any erase or program operation to proceed. After the programming command has
been executed, the user must wait for the programming time until programming is complete. The
two instructions following the start of the programming sequence should be NOPs, as shown in
Section 4.6.4.2 ?NVMKEY Register?.

 */
void flash_write_buffer_to_flash(void)
{
    //
    // Block erase
    //
    NVMCONbits.NVMOP = 0x2;     // Erase block

    // Perform the erase operation specified by NVMOP3:NVMOP0 on the next WR command
    NVMCONbits.ERASE = 1;

    // Enable Flash program/erase operations
    NVMCONbits.WREN = 1;

    // Start sequence accoding to doc: DS39715A-page 4-16
    NVMKEY = 0x55;
    NVMKEY = 0xAA;
    NVMCONbits.WR = 1;
    Nop();
    Nop();
}

// =============================================================================
// Private function definitions
// =============================================================================



