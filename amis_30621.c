#include "amis_30621.h"
#include "common_memory.h"

#define NOT_CORRECTOR_ADDRESS   0xFF

//uint8_t cnt;

uint8_t linChecksum(uint8_t* packet)
{
    uint16_t sum = 0;
    uint8_t cnt;
    for (cnt = 1; cnt < 9; cnt++)
        sum += packet[cnt];
    ((uint8_t*)(&sum))[0] += ((uint8_t*)(&sum))[1];
    ((uint8_t*)(&sum))[0] = ~((uint8_t*)(&sum))[0];
    return ((uint8_t*)(&sum))[0];
}

uint8_t writeCorrectorPosition(uint8_t address, int16_t position)
{
    uint8_t cnt;
    linFrame[0] = 0x3C; linFrame[1] = 0x80; linFrame[2] = 0x8B;
    linFrame[3] = address;
    linFrame[4] = (position >> 8) & 0xFF; linFrame[5] = (position >> 0) & 0xFF;
    linFrame[6] = NOT_CORRECTOR_ADDRESS;
    linFrame[7] = 0xFF;                   linFrame[8] = 0xFF;
    linFrame[9] = linChecksum(linFrame);

    writeLinSync();
    for (cnt = 0; cnt < 10; cnt++)
    {
        if (linWriteByte(linFrame[cnt]))
            return 1;
    }
    return 0;
}

uint8_t readCorrectorPosition(int16_t* position, uint8_t address)
{
    uint8_t cnt;
    uint8_t cnt2;
    linFrame[0] = 0x3C; linFrame[1] = 0x80; linFrame[2] = 0x81; linFrame[3] = address;
    linFrame[4] = 0xFF; linFrame[5] = 0xFF; linFrame[6] = 0xFF; linFrame[7] = 0xFF;
    linFrame[8] = 0xFF;
    linFrame[9] = linChecksum(linFrame);

    writeLinSync();
    for (cnt = 0; cnt < 10; cnt++)
    {
        if (linWriteByte(linFrame[cnt]))
            return 1;
    }
    
    for (cnt = 0; cnt < 2; cnt++)
    {        
        writeLinSync();
        linWriteByte(0x7D);
        
#if 0
        for (cnt2 = 1; cnt2 < 10; cnt2++)
        {
            if (linReadByte(&(linFrame[cnt2]), 20))
                return 2;
        }
#else
        cnt2 = linWaitData(&(linFrame[1]), 9, 30);
        if (cnt2 < 9)
            return 2;
#endif
        
        // TODO: maybe repeat can be useful
        if (linFrame[9] != linChecksum(linFrame))
            return 3;
        
        if (cnt == 1)
            position[0] = (linFrame[2] << 8) | linFrame[3];
    }    
    
    return 0;
}