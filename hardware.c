
#include <pic12f1822.h>
#include <stdint.h>

//uint8 res; 

uint8_t i;

void initHardware()
{
    OSCCON = 0x58;
    APFCON = 0x84;
    TRISA = 0x24;
    ANSELA = 0x04;
    PORTA = 0x00;
    LATA = 0x00;
    WPUA = 0x00;
//    SPBRGH = 0;
//    SPBRGL = 12;
    SPBRG = 12;
    ADCON0 = 0x09;
    ADCON1 = 0x00;
    BAUDCON = 0x08;
    TXSTA = 0x24;
    RCSTA = 0x90;
    OPTION_REG = 0x87;
    INTCON = 0;
}

void setAdcChannel(uint8_t channelCode)
{
    ADCON0 = (ADCON0 & 0x83) | (channelCode << 2);
}

uint8_t readAdc8bit()
{
    ADCON1bits.ADFM = 0;
    ADCON0bits.ADGO = 1;
    while(ADCON0bits.ADGO == 1)
    {}
    return ADRESH;
}

uint16_t readAdc10bit()
{
    ADCON1bits.ADFM = 1;
    ADCON0bits.ADGO = 1;
    while(ADCON0bits.ADGO == 1)
    {}
    return ((ADRESH << 8) + ADRESL);
}

void writeLinSync()
{
    RCSTAbits.CREN = 0;
    TXSTAbits.SENDB = 1;    
    while (TXSTAbits.TRMT == 0)
    {}
    TXREG = 0x55;
    TXREG = 0x55;
    while (TXSTAbits.SENDB == 1)
    {}
    while (TXSTAbits.TRMT == 0)
    {}
    RCSTAbits.CREN = 1;
}

uint8_t linWriteByte(uint8_t byte)
{
    TXREG = byte;
    TMR0 = 0xFE;
    INTCONbits.TMR0IF = 0;
    while((PIR1bits.RCIF == 0) && (INTCONbits.TMR0IF == 0))
    {}
    if (PIR1bits.RCIF == 0)
        return 1;
    if (RCREG != byte)
        return 2;
    return 0;
}

uint8_t linReadByte(uint8_t* byte, uint8_t timeout)
{
    TMR0 = 255 - timeout;
    INTCONbits.TMR0IF = 0;
    if (RCSTAbits.OERR)
    {
        RCSTA = 0;
        asm("nop");
        RCSTA = 0x90;
    }
    while((PIR1bits.RCIF == 0) && (INTCONbits.TMR0IF == 0))
    {}
    if (PIR1bits.RCIF == 0)
        return 1;
    PIR1bits.RCIF = 0;
    byte[0] = RCREG;
    return 0;
}

uint8_t linWaitData(uint8_t* data, uint8_t maxDataLen, uint8_t timeout)
{
    TMR0 = 255 - timeout;
    INTCONbits.TMR0IF = 0;
    if (RCSTAbits.OERR)
    {
        RCSTA = 0;
        asm("nop");
        RCSTA = 0x90;
    }
    i = 0;
    while(1)
    {
        if (INTCONbits.TMR0IF)
            return i;
        if (PIR1bits.RCIF)
        {
            if (i < maxDataLen)
                data[i] = RCREG;
            PIR1bits.RCIF = 0;
            i++;
        }
    }
}

//uint8_t linReadByte(uint8_t* byte, uint8_t timeout)
//{
//    TMR0 = 255 - timeout;
//    INTCONbits.TMR0IF = 0;
//    if (RCSTAbits.OERR)
//    {
//        RCSTA = 0;
//        asm("nop");
//        RCSTA = 0x90;
//    }
//    while((PIR1bits.RCIF == 0) && (INTCONbits.TMR0IF == 0))
//    {}
//    if (PIR1bits.RCIF == 0)
//        return 1;
//    PIR1bits.RCIF = 0;
//    byte[0] = RCREG;
//    return 0;
//}

void sleep(uint8_t timeMs)
{
    TMR0 = 255 - timeMs;
    INTCONbits.TMR0IF = 0;
    while(INTCONbits.TMR0IF == 0)
    {}
}

void setTmr1Timeout(uint16_t timeoutIn32ns)
{
    TMR1 = 0xFFFF - timeoutIn32ns;
}

void DATAEE_WriteByte(uint8_t address, uint8_t data)
{
    EEADRL = (uint8_t)(address & 0x0ff);    // Data Memory Address to write
    EEDATL = data;             // Data Memory Value to write
    EECON1bits.EEPGD = 0;   // Point to DATA memory
    EECON1bits.CFGS = 0;        // Deselect Configuration space
    EECON1bits.WREN = 1;        // Enable writes

    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;      // Set WR bit to begin write
    // Wait for write to complete
    while (EECON1bits.WR)
    {
    }

    EECON1bits.WREN = 0;    // Disable writes
}

uint8_t DATAEE_ReadByte(uint8_t address)
{
    EEADRL = (uint8_t)(address & 0x0ff);    // Data Memory Address to read
    EECON1bits.CFGS = 0;    // Deselect Configuration space
    EECON1bits.EEPGD = 0;   // Point to DATA memory
    EECON1bits.RD = 1;      // EE Read
    asm("nop");  // NOPs may be required for latency at high frequencies
    asm("nop");

    return (EEDATL);
}