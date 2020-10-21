/* 
 * File:   hardware.h
 * Author: t-bird
 *
 * Created on 27 ??????? 2020 ?., 21:22
 */

#ifndef HARDWARE_H
#define	HARDWARE_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define ADC_POSITION_CHANNEL    0x02
#define ADC_TEMPERATURE_CHANNEL 0x1D
    
#define LED_ON          (LATAbits.LATA0 = 1)
#define LED_OFF         (LATAbits.LATA0 = 0)
#define LED_LATCH       (LATAbits.LATA0)
#define LED_TOGGLE      (LATAbits.LATA0 ^= 1)
    
#include <stdint.h>

    void initHardware();
    void setAdcChannel(uint8_t channelCode);
    uint8_t readAdc8bit();
    uint16_t readAdc10bit();
    void writeLinSync();
    uint8_t linWriteByte(uint8_t byte);
    uint8_t linReadByte(uint8_t* byte, uint8_t timeout);
    uint8_t linWaitData(uint8_t* data, uint8_t maxDataLen, uint8_t timeout);
    void sleep(uint8_t timeMs);
    void setTmr1Timeout(uint16_t timeoutIn32ns);
    void DATAEE_WriteByte(uint8_t address, uint8_t data);
    uint8_t DATAEE_ReadByte(uint8_t address);


#ifdef	__cplusplus
}
#endif

#endif	/* HARDWARE_H */

