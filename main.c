/*
 * File:   main.c
 * Author: t-bird
 *
 * Created on 24 06 2020 , 14:37
 */

// PIC12F1822 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is digital input)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = OFF      // PLL Enable (4x PLL disabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include "hardware.h"
#include "definitions.h"

uint8_t i;
uint16_t temp;
uint8_t l;

uint8_t currentFrameIndex;
uint8_t linFrame[11];

settings_t settings;
currentValues_t currentValues;

void loadSettings();
uint8_t checkSettings();
void loadDefaultSettings();
uint8_t writeSettings();
uint8_t linChecksum(uint8_t* packet);

uint8_t readCorrectorPosition(uint16_t* position, uint8_t address);
uint8_t writeCorrectorPosition(uint8_t address, uint16_t position);

void sendCurrentValues();
void checkExtCommand();
void sendAck(uint8_t code);
//uint8_t resetCorrectorPosition(uint8_t address);

void main(void) {

    currentValues.flags.ALL_FLAGS = 0;
    currentValues.counter = 0;
    currentValues.internalErrors.ALL_ERRORS = 0;
    currentValues.motorErrors[0].ALL_ERRORS = 0;
    currentValues.motorErrors[1].ALL_ERRORS = 0;
    currentValues.positionIndex = 255;
    currentFrameIndex = 0;
    
    initHardware();
    
    loadSettings();
    if (checkSettings())
    {
        currentValues.internalErrors.SETTINGS_ERROR = 1;
        loadDefaultSettings();
    }
    
    setAdcChannel(ADC_TEMPERATURE_CHANNEL);
    LED_ON;
    writeLinSync();
    sleep(100);
    LED_OFF;
    sleep(100);
    
    if (!currentValues.internalErrors.SETTINGS_EMPTY)
    {
        LED_ON;         
        for (i = 0; i < settings.correctorsNum; i++)
        {
            temp = readCorrectorPosition(&(currentValues.rdCorrectorValues[i]), settings.correctorsAdresses[i]);
            if (temp != 0)
            {
                switch (temp)
                {
                    case 1:
                        currentValues.motorErrors[i].LIN_TXRX_INIT = 1;
                        break;
                    case 2:
                        currentValues.motorErrors[i].NO_ACK_INIT = 1;
                        break;
                    default:
                        currentValues.motorErrors[i].BAD_CONNECTION_INIT = 1;
                        break;
                }
                continue;
            }
            
            currentValues.wrCorrectorValues[i] = settings.correctorStartPositions[i] * settings.correctorPositionMult;
            temp = writeCorrectorPosition(settings.correctorsAdresses[i], currentValues.wrCorrectorValues[i]);
            if (temp != 0)
            {
                currentValues.motorErrors[i].LIN_TXRX_INIT = 1;
                continue;
            }
        }            
        LED_OFF;
    }
    
    while(1)
    {
        currentValues.temperature = readAdc8bit();
        setAdcChannel(ADC_POSITION_CHANNEL);        
        temp = 0;
        for (i = 0; i < 8; i++)
            temp += readAdc10bit();
        temp = temp >> 5;
        currentValues.adcPositionValue = (uint8_t)temp;
        setAdcChannel(ADC_TEMPERATURE_CHANNEL);
        
        temp = 0;
        //currentValues.positionIndex = 0;
        for (i = 0; i < (settings.positionsNum - 1); i++)
        {
            if (currentValues.adcPositionValue > settings.adcValues[i])
                temp = i + 1;//currentValues.positionIndex = i + 1;
        }
        
        if ((temp != currentValues.positionIndex) || (currentValues.flags.EXT_CORRECTOR_VALUES))
        {
            currentValues.positionIndex = temp;
            for (i = 0; i < settings.correctorsNum; i++)
            {
                if (currentValues.motorErrors[i].ALL_ERRORS == 0)
                {
                    if (!currentValues.flags.EXT_CORRECTOR_VALUES)
                        currentValues.wrCorrectorValues[i] = settings.correctorsValues[i][currentValues.positionIndex] * settings.correctorPositionMult;
                    temp = writeCorrectorPosition(settings.correctorsAdresses[i], currentValues.wrCorrectorValues[i]);
                    if (temp != 0)
                        currentValues.motorErrors[i].LIN_TXRX_SET = 1;
                }
            }
        }
        
        if (currentValues.flags.HALF_COUNTER == 1)
        {
            LED_OFF;
        }
        else
        {
            if ((currentValues.counter % 4) == 0)
            {
                for (i = 0; i < settings.correctorsNum; i++)
                {
                    if (currentValues.motorErrors[i].ALL_ERRORS != 0)
                        continue;
                    temp = readCorrectorPosition(&(currentValues.rdCorrectorValues[i]), settings.correctorsAdresses[i]);
                    if (temp != 0)
                    {
                        switch (temp)
                        {
                            case 1:
                                currentValues.motorErrors[i].LIN_TXRX_INIT = 1;
                                break;
                            case 2:
                                currentValues.motorErrors[i].NO_ACK_INIT = 1;
                                break;
                            default:
                                currentValues.motorErrors[i].BAD_CONNECTION_INIT = 1;
                                break;
                        }
                        continue;
                    }
                }
                // TODO: ADD FRAME TO PC
                
                sendCurrentValues();
            }
            
            //if (currentValues.counter  == 0)
            //    sendCurrentValues();
            
            if ((currentValues.internalErrors.ALL_ERRORS == 0) && (currentValues.motorErrors[0].ALL_ERRORS == 0) && (currentValues.motorErrors[1].ALL_ERRORS == 0))
                currentValues.displayed_error = 0;
            else
            {
                if (currentValues.internalErrors.SETTINGS_EMPTY)
                    currentValues.displayed_error = 3;
                else
                {
                    currentValues.displayed_error = 3;
                    if ((currentValues.motorErrors[0].ALL_ERRORS) && (currentValues.motorErrors[1].ALL_ERRORS))
                        currentValues.displayed_error += 28;
                    else if (currentValues.motorErrors[0].ALL_ERRORS)
                        currentValues.displayed_error += 4;
                    else if (currentValues.motorErrors[1].ALL_ERRORS)
                        currentValues.displayed_error += 12;
                }
            }

            // ERRORS DISPLAYING
            if ((currentValues.counter < 8) && (currentValues.displayed_error != 0))
            {
                if (currentValues.displayed_error & (1 << currentValues.counter))
                    LED_ON;
            }
            
            currentValues.counter++;
            if (currentValues.counter > 11)
                currentValues.counter = 0;
        }
                
        currentFrameIndex += linWaitData(&(linFrame[currentFrameIndex]), sizeof(linFrame) - currentFrameIndex, 98);
                
        checkExtCommand();
        
        currentValues.flags.HALF_COUNTER ^= 1;

//            temp = 0xFF;
//            for (i = 0; i < settings.positionsNum; i++)
//            {
//                if (((currentValues.adcPositionValue >= settings.adcValues[i]) && ((currentValues.adcPositionValue - settings.adcValues[i]) <= settings.adcMaxDiff)) ||
//                        ((currentValues.adcPositionValue < settings.adcValues[i]) && ((settings.adcValues[i] - currentValues.adcPositionValue) <= settings.adcMaxDiff)))
//                    temp = i;
//            }
//            if (temp == 0xFF)
//            {
//                LED_ON;
//                sleep(50);
//                LED_OFF;
//            }
//            else
//                currentValues.positionIndex = temp;
//            temp = 0;
//            

            
//            for (j = 0; j < settings.correctorsNum; j++)
//            {
//                readCorrectorPosition(&(currentValues.rdCorrectorValues[i]), settings.correctorsAdresses[i]);
//                
//            }
        //else
            
        //LED_LATCH ^= 1;
        
    }
    
    return;
}

uint8_t readCorrectorPosition(uint16_t* position, uint8_t address)
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
        
        for (cnt2 = 1; cnt2 < 10; cnt2++)
        {
            if (linReadByte(&(linFrame[cnt2]), 20))
                return 2;
        }
        
        // TODO: maybe repeat can be useful
        if (linFrame[9] != linChecksum(linFrame))
            return 3;
        
        if (temp == 1)
            position[0] = (linFrame[2] << 8) | linFrame[3];
    }    
    
    return 0;
}

uint8_t writeCorrectorPosition(uint8_t address, uint16_t position)
{
#if 1
    uint8_t cnt;
    linFrame[0] = 0x3C; linFrame[1] = 0x80; linFrame[2] = 0x8B;
    linFrame[3] = address;
    linFrame[4] = (position >> 8) & 0xFF; linFrame[5] = (position >> 0) & 0xFF;
    linFrame[6] = 0xFF;
    linFrame[7] = 0xFF;                   linFrame[8] = 0xFF;
    linFrame[9] = linChecksum(linFrame);

    writeLinSync();
    for (cnt = 0; cnt < 10; cnt++)
    {
        if (linWriteByte(linFrame[cnt]))
            return 1;
    }
#else    
    linFrame[0] = 0x2C; linFrame[1] = 0x80; linFrame[2] = 0x8B;
    linFrame[3] = address;
    linFrame[4] = (position >> 8) & 0xFF; linFrame[5] = (position >> 0) & 0xFF;
    linFrame[6] = 0xFF;
    linFrame[7] = 0xFF;                   linFrame[8] = 0xFF;
    linFrame[9] = linChecksum(linFrame);

    writeLinSync();
    for (i = 0; i < 9; i++)
    {
        if (linWriteByte(linFrame[i]))
            return 1;
    }
#endif
    
//        data[0] = 0xC1; data[1] = 0x00;
//    data[2] = 0x55; data[3] = 0x3C;
//    data[4] = 0x80; data[5] = 0x8B; data[6] = ui->motorAddress->value() & 0xFF;
//    data[7] = (ui->motorPosition->value() & 0xFF00) >> 8;
//    data[8] = ui->motorPosition->value() & 0xFF;
//    data[9] = 0xE0; data[10] = 0xFF; data[11] = 0xFF;
}

//uint8_t resetCorrectorPosition(uint8_t address)
//{
//
//}

void loadSettings()
{
    uint8_t* settingsAddress = (uint8_t*)(&settings);
    uint8_t cnt;
    for (cnt = 0; cnt < sizeof(settings); cnt++)
        settingsAddress[cnt] = DATAEE_ReadByte(cnt);
}

uint8_t checkSettings()
{
    if (settings.correctorsNum > 2)
        return 1;
    if (settings.positionsNum > 16)
        return 2;
    if ((settings.correctorsNum == 0) || (settings.positionsNum == 0))
        currentValues.internalErrors.SETTINGS_EMPTY = 1;
    return 0;
}

void loadDefaultSettings()
{
    settings.correctorsNum = 0;
    settings.positionsNum = 0;
    currentValues.internalErrors.SETTINGS_EMPTY = 1;
}

uint8_t writeSettings()
{
    uint8_t cnt;
    uint8_t* settingsAddress = (uint8_t*)(&settings);
    for (cnt = 0; cnt < sizeof(settings); cnt++)
        DATAEE_WriteByte(cnt, settingsAddress[cnt]);    
    for (cnt = 0; cnt < sizeof(settings); cnt++)
    {
        if (settingsAddress[cnt] != DATAEE_ReadByte(cnt))
            return 1;
    }
    return 0;
}

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

// EXT COMMANDS: input frame has 11 bytes: E2 code data[0] data[1] data[2] data[3] data[4] data[5] data[6] data[7] checksum
//               output frame can have any bytes number
//               output frame codes:    0x35 - sendCurrentValues, 0x15 - sendSettings, 0x25 - sendAck
//               input frames codes:    0x00-0x07 - writePartOfSettings (part number = code - 0x07), 0x10 - writeSettingsToEeprom, 
//                                      0x11 - readSettingsFromEeprom, 0x12 - settingsRequest, 0x17 - writeCorrectorPositions, 0x18 - clearErrors
void sendCurrentValues()
{
    uint8_t* currentValuesAddress = (uint8_t*)(&currentValues);
    uint8_t cnt;
    temp = 0x35;
    linWriteByte(0xE2);
    linWriteByte(0x35);
    for (cnt = 0; cnt < sizeof(currentValues); cnt++)
    {
        linWriteByte(currentValuesAddress[cnt]);
        temp += currentValuesAddress[cnt];
    }
    linWriteByte(temp);
    currentFrameIndex = 0;
}

void checkExtCommand()
{
    uint8_t cnt;
    if (currentFrameIndex < 11)
        return;
    if (linFrame[0] != 0xE2)
    {
        sendAck(3);
        currentFrameIndex = 0;
        return;
    }
    temp = 0;
    for (cnt = 1; cnt < 10; cnt++)
        temp += linFrame[cnt];
    if ((temp & 0xFF) != linFrame[10])
    {
        sendAck(2);
        currentFrameIndex = 0;
        return;
    }
    
    uint8_t* settingsAddress = (uint8_t*)(&settings);
    switch (linFrame[1])
    {
        case 0x10:
            // WRITE SETTINGS TO EEPROM
            temp = writeSettings();
            sendAck(0);
            break;
        case 0x11:
            // LOAD SETTINGS FROM EEPROM
            loadSettings();
            sendAck(0);
            break;
        case 0x12:
            // READ SETTINGS
            linWriteByte(0xE2);
            linWriteByte(0x15);
            temp = 0x15;
            for (cnt = 0; cnt < sizeof(settings); cnt++)
            {
                linWriteByte(settingsAddress[cnt]);
                temp += settingsAddress[cnt];
            }
            linWriteByte(temp);
            break;
        case 0x17:
            // WRITE CORRECTORS POSITIONS
            currentValues.wrCorrectorValues[0] = linFrame[2] | (linFrame[3] << 8);
            currentValues.wrCorrectorValues[1] = linFrame[4] | (linFrame[5] << 8);
            if (linFrame[5] == 0)
                currentValues.flags.EXT_CORRECTOR_VALUES = 0;
            else
                currentValues.flags.EXT_CORRECTOR_VALUES = 1;             
            sendAck(0);
            break;
        case 0x18:
            // CLEAR ERRORS
            currentValues.internalErrors.ALL_ERRORS = 0;
            currentValues.motorErrors[0].ALL_ERRORS = 0;
            currentValues.motorErrors[1].ALL_ERRORS = 0;
            sendAck(0);
            break;
        default:
            // WRITE DATE TO SETTINGS
            if (linFrame[1] > 0x07)
            {              
                sendAck(1);
                currentFrameIndex = 0;
                return;
            }
            for (cnt = 0; cnt < 8; cnt++)
            {
                if ((linFrame[1] * 8 + cnt) < sizeof(settings))
                    settingsAddress[linFrame[1] * 8 + cnt] = linFrame[cnt + 2];
            }
            sendAck(0);
            break;
    }    
    currentFrameIndex = 0;
}

void sendAck(uint8_t code)
{
    linWriteByte(0xE2);
    linWriteByte(0x25);            
    linWriteByte(code);
    linWriteByte(code + 0x25);
}

//            data[0] = 0xC1; data[1] = 0xC0;
//            data[2] = 0x55; data[3] = 0x3C;
//            data[4] = 0x80; data[5] = 0x81; data[6] = ui->motorAddress->value() & 0xFF; data[7] = 0xFF;
//            data[8] = 0xFF; data[9] = 0xFF; data[10] = 0xFF; data[11] = 0xFF;
//            checksum = 0; checksum2 = 0;
//            for (int i = 4; i < 12; i++)
//                checksum = checksum + (((unsigned int)data[i]) & 0xFF);
//            checksum2 = ~((checksum & 0xFF) + ((checksum >> 8) & 0xFF));
//            data[12] = checksum2;