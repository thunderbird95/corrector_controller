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
#include "amis_30621.h"
#include "common_memory.h"
#include "settings_mgr.h"
#include "ext_control.h"

uint8_t correctorIndex;
//uint8_t j;
uint8_t counter;
//uint16_t temp;

#define MOTOR_START_SET_TIME    15  //  TIME IN 100MS intervals
#define READ_ATTEMPTS_NUM   3

void readCorrectorPositionWithRepeats();

void main(void) {

    currentValues.flags.ALL_FLAGS = 0;
    //counter = 0;
    currentValues.internalErrors.ALL_ERRORS = 0;
    currentValues.motorErrors[0].ALL_ERRORS = 0;
    currentValues.motorErrors[1].ALL_ERRORS = 0;
    currentValues.positionIndex = 255;
//    currentValues.rdCorrectorValues[0] = 0;
//    currentValues.rdCorrectorValues[1] = 0;
//    currentValues.wrCorrectorValues[0] = 0;
//    currentValues.wrCorrectorValues[1] = 0;
    currentFrameIndex = 0;
        
    initHardware();
    
    loadSettings();
    if (checkSettings())
    {
        currentValues.internalErrors.SETTINGS_ERROR = 1;
        loadDefaultSettings();
    }
    
    //sendCurrentValues();
    
    setAdcChannel(ADC_TEMPERATURE_CHANNEL);
    LED_ON;
    writeLinSync();
    sleep(100);
    LED_OFF;
    sleep(100);
    
    if (!currentValues.internalErrors.SETTINGS_EMPTY)
    {
        LED_ON;         
        for (correctorIndex = 0; correctorIndex < settings.correctorsNum; correctorIndex++)
        {
//            for (j = 0; j < settings.readAttemptsNum; j++)
//            {
//                temp = readCorrectorPosition(&(currentValues.rdCorrectorValues[i]), settings.correctorsAdresses[i]);
//                if (temp == 0)
//                    break;
//                LED_OFF;
//                sleep(100);
//            }
//            LED_ON;
            readCorrectorPositionWithRepeats();
            if (currentValues.motorErrors[correctorIndex].ALL_ERRORS)
                continue;
//            if (temp != 0)
//            {
//                switch (temp)
//                {
//                    case 1:
//                        currentValues.motorErrors[i].LIN_TXRX_INIT = 1;
//                        break;
//                    case 2:
//                        currentValues.motorErrors[i].NO_ACK_INIT = 1;
//                        break;
//                    default:
//                        currentValues.motorErrors[i].BAD_CONNECTION_INIT = 1;
//                        break;
//                }
//                continue;
//            }
            if (currentValues.rdCorrectorValues[correctorIndex] != 0)
            {
                currentValues.motorErrors[correctorIndex].LOGIC_ERROR = 1;
                continue;
            }
            
            currentValues.wrCorrectorValues[correctorIndex] = settings.correctorStartPositions[correctorIndex] * settings.correctorPositionMult;
            /*temp = */writeCorrectorPosition(settings.correctorsAdresses[correctorIndex], currentValues.wrCorrectorValues[correctorIndex]);
//            if (temp != 0)
//            {
//                currentValues.motorErrors[i].LIN_TXRX_INIT = 1;
//                continue;
//            }
        }
        
        for (counter = 0; counter < MOTOR_START_SET_TIME/*settings.startPositionTimeout*/; counter++)
        {            
            //writeLinSync(); //  MOTOR CAN BE SLEEP, IF NO COMMANDS WAS RECEIVED ~1s
            sleep(98);
            currentValues.flags.INIT_CORRECTORS = 1;
            for (correctorIndex = 0; correctorIndex < settings.correctorsNum; correctorIndex++)
            {
                if (currentValues.motorErrors[correctorIndex].ALL_ERRORS)
                    continue;
                readCorrectorPositionWithRepeats();
                if (currentValues.motorErrors[correctorIndex].ALL_ERRORS)
                    continue;
                if (currentValues.rdCorrectorValues[correctorIndex] == 0)
                    writeCorrectorPosition(settings.correctorsAdresses[correctorIndex], currentValues.wrCorrectorValues[correctorIndex]);
                if (currentValues.rdCorrectorValues[correctorIndex] != currentValues.wrCorrectorValues[correctorIndex])
                    currentValues.flags.INIT_CORRECTORS = 0;
            }
            if (currentValues.flags.INIT_CORRECTORS)
                break;
#if 0
            for (correctorIndex = 0; correctorIndex < settings.correctorsNum; correctorIndex++)
            {
                if (currentValues.rdCorrectorValues[correctorIndex] != currentValues.wrCorrectorValues[correctorIndex])
                    continue;
            }
            break;
#else
//            if ((currentValues.rdCorrectorValues[0] == currentValues.wrCorrectorValues[0]) && (currentValues.rdCorrectorValues[1] == currentValues.wrCorrectorValues[1]))
//                break;
#endif
            
//            for (j = 0; j < settings.readAttemptsNum; j++)
//            {
//                temp = readCorrectorPosition(&(currentValues.rdCorrectorValues[i]), settings.correctorsAdresses[i]);
//                if (temp == 0)
//                    break;
//                LED_OFF;
//                sleep(100);
//            }
//            LED_ON;
//            if (temp != 0)
//            {
//                switch (temp)
//                {
//                    case 1:
//                        currentValues.motorErrors[i].LIN_TXRX_INIT = 1;
//                        break;
//                    case 2:
//                        currentValues.motorErrors[i].NO_ACK_INIT = 1;
//                        break;
//                    default:
//                        currentValues.motorErrors[i].BAD_CONNECTION_INIT = 1;
//                        break;
//                }
//                continue;
//            }
        }
        LED_OFF;
    }
    
    for (correctorIndex = 0; correctorIndex < settings.correctorsNum; correctorIndex++)
    {
        if (currentValues.motorErrors[correctorIndex].ALL_ERRORS)
            continue;
        if (currentValues.rdCorrectorValues[correctorIndex] != currentValues.wrCorrectorValues[correctorIndex])
            currentValues.motorErrors[correctorIndex].LOGIC_ERROR = 1;
    }
  
    counter = 0;
    
    while(1)
    {
        uint16_t temp;
        currentValues.temperature = readAdc8bit();
        setAdcChannel(ADC_POSITION_CHANNEL);        
        temp = 0;
        for (correctorIndex = 0; correctorIndex < 8; correctorIndex++)
#if 1
            temp += readAdc10bit();
        temp = temp >> 5;
        currentValues.adcPositionValue = (uint8_t)temp;
#else
            temp += readAdc8bit();
        currentValues.adcPositionValue = (temp >> 3) & 0xFF;    
#endif
        setAdcChannel(ADC_TEMPERATURE_CHANNEL);
        
        temp = 0;
        for (correctorIndex = 0; correctorIndex < (settings.positionsNum - 1); correctorIndex++)
        {
            if (currentValues.adcPositionValue > settings.adcValues[correctorIndex])
                temp = correctorIndex + 1;//currentValues.positionIndex = i + 1;
        }
        
        if ((temp != currentValues.positionIndex) || (currentValues.flags.EXT_CORRECTOR_VALUES))
        {
            currentValues.positionIndex = temp;
            for (correctorIndex = 0; correctorIndex < settings.correctorsNum; correctorIndex++)
            {
                if (currentValues.motorErrors[correctorIndex].ALL_ERRORS == 0)
                {
                    if (!currentValues.flags.EXT_CORRECTOR_VALUES)
                        currentValues.wrCorrectorValues[correctorIndex] = settings.correctorsValues[correctorIndex][currentValues.positionIndex] * settings.correctorPositionMult;
                    /*temp =*/ writeCorrectorPosition(settings.correctorsAdresses[correctorIndex], currentValues.wrCorrectorValues[correctorIndex]);
//                    if (temp != 0)
//                        currentValues.motorErrors[i].LIN_TXRX_SET = 1;
                }
            }
        }
        
        if (currentValues.flags.HALF_COUNTER == 1)
        {
            LED_OFF;
        }
        else
        {
            if ((counter % 2) == 0)
            {
                //currentValues.flags.MOTION_IN_PROGRESS = 0;
                for (correctorIndex = 0; correctorIndex < settings.correctorsNum; correctorIndex++)
                {
                    if (currentValues.motorErrors[correctorIndex].ALL_ERRORS != 0)
                        continue;
                    
//                    for (j = 0; j < settings.readAttemptsNum; j++)
//                    {
//                        temp = readCorrectorPosition(&(currentValues.rdCorrectorValues[i]), settings.correctorsAdresses[i]);
//                        if (temp == 0)
//                            break;
//                        LED_ON;
//                        sleep(100);
//                    }
//                    LED_OFF;
                    readCorrectorPositionWithRepeats();
                    
                    if (currentValues.motorErrors[correctorIndex].ALL_ERRORS)
                        continue;
                    
                    if (currentValues.rdCorrectorValues[correctorIndex] != currentValues.wrCorrectorValues[correctorIndex])
                    {
                        writeCorrectorPosition(settings.correctorsAdresses[correctorIndex], currentValues.wrCorrectorValues[correctorIndex]);
                        //currentValues.flags.MOTION_IN_PROGRESS = 1;
                    }
//                    if (temp != 0)
//                    {
//                        switch (temp)
//                        {
//                            case 1:
//                                currentValues.motorErrors[i].LIN_TXRX_INIT = 1;
//                                break;
//                            case 2:
//                                currentValues.motorErrors[i].NO_ACK_INIT = 1;
//                                break;
//                            default:
//                                currentValues.motorErrors[i].BAD_CONNECTION_INIT = 1;
//                                break;
//                        }
//                        continue;
//                    }
                }
                
                sendCurrentValues();
            }
            
//            if ((currentValues.internalErrors.ALL_ERRORS == 0) && (currentValues.motorErrors[0].ALL_ERRORS == 0) && (currentValues.motorErrors[1].ALL_ERRORS == 0))
//                currentValues.displayed_error = 0;
//            else
//            {
#if 0
                currentValues.displayed_error = 0;
                if (currentValues.internalErrors.SETTINGS_EMPTY == 1)
                    currentValues.displayed_error += 4;
                if (currentValues.motorErrors[0].ALL_ERRORS != 0)
                    currentValues.displayed_error += 1;
                if (currentValues.motorErrors[1].ALL_ERRORS != 0)
                    currentValues.displayed_error += 2;
#else
                if ((currentValues.internalErrors.ALL_ERRORS == 0) && (currentValues.motorErrors[0].ALL_ERRORS == 0) && (currentValues.motorErrors[1].ALL_ERRORS == 0))
                    currentValues.displayed_error = 0;
                else if (currentValues.internalErrors.SETTINGS_EMPTY)
                    currentValues.displayed_error = 5;
//                else if (currentValues.internalErrors.LIN_TXRX)
//                    currentValues.displayed_error = 7;
#endif
//                if (currentValues.internalErrors.LIN_TXRX == 1)
//                    currentValues.displayed_error += 8;
//            }
//            else if ((currentValues.motorErrors[0].ALL_ERRORS != 0) && (currentValues.motorErrors[1].ALL_ERRORS == 0))
//                currentValues.displayed_error = 2;
//            else if ((currentValues.motorErrors[0].ALL_ERRORS == 0) && (currentValues.motorErrors[1].ALL_ERRORS != 0))
//                currentValues.displayed_error = 3;
//            else if ((currentValues.motorErrors[0].ALL_ERRORS != 0) && (currentValues.motorErrors[1].ALL_ERRORS != 0))
//                currentValues.displayed_error = 4;

            // ERRORS DISPLAYING
            if (currentValues.displayed_error > counter)
                LED_ON;
            
            counter++;
            if (counter > 11)
                counter = 0;
        }        
                
        waitExtCommand();
        
        currentValues.flags.HALF_COUNTER ^= 1;        
    }
    
    return;
}

void readCorrectorPositionWithRepeats()
{
    uint8_t i, result;
    for (i = 0; i < settings.readAttemptsNum; i++)
    {
        result = readCorrectorPosition(&(currentValues.rdCorrectorValues[correctorIndex]), settings.correctorsAdresses[correctorIndex]);
        if (result == 0)
            break;
        LED_TOGGLE;
        sleep(100);
        LED_TOGGLE;
    }
    if (result != 0)
    {
        switch (result)
        {
            case 1:
                currentValues.internalErrors.LIN_TXRX = 1;//currentValues.motorErrors[i].LIN_TXRX_INIT = 1;
                break;
            case 2:
                currentValues.motorErrors[correctorIndex].NO_ACK_INIT = 1;
                break;
            default:
                currentValues.motorErrors[correctorIndex].BAD_CONNECTION_INIT = 1;
                break;
        }
        //continue;
    }
}