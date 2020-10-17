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

uint8_t i;
uint16_t temp;
uint8_t counter;

#define MOTOR_START_SET_TIME    15  //  TIME IN 100MS intervals
#define READ_ATTEMPTS_NUM   3

void main(void) {

    currentValues.flags.ALL_FLAGS = 0;
    counter = 0;
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
        
        for (i = 0; i < MOTOR_START_SET_TIME; i++)
        {            
            writeLinSync(); //  MOTOR CAN BE SLEEP, IF NO COMMANDS WAS RECEIVED ~1s
            sleep(98);
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
            if ((counter % 4) == 0)
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
                
                sendCurrentValues();
            }
            
            if ((currentValues.internalErrors.ALL_ERRORS == 0) && (currentValues.motorErrors[0].ALL_ERRORS == 0) && (currentValues.motorErrors[1].ALL_ERRORS == 0))
                currentValues.displayed_error = 0;
            else
            {
                if (currentValues.internalErrors.SETTINGS_EMPTY)
                    currentValues.displayed_error = 1;
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
            if ((counter < 8) && (currentValues.displayed_error != 0))
            {
                if (currentValues.displayed_error & (1 << counter))
                    LED_ON;
            }
            
            counter++;
            if (counter > 11)
                counter = 0;
        }        
                
        waitExtCommand();
        
        currentValues.flags.HALF_COUNTER ^= 1;        
    }
    
    return;
}

