/* 
 * File:   definitions.h
 * Author: t-bird
 *
 * Created on 27 ??????? 2020 ?., 21:23
 */

#ifndef DEFINITIONS_H
#define	DEFINITIONS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdint.h>        

typedef union {
    struct {
        unsigned SETTINGS_ERROR         :1;
        unsigned SETTINGS_EMPTY         :1;        
        unsigned ADC_VALUE_UNCORRECT    :1;
        unsigned LIN_TXRX               :1;
        unsigned UNKNOWN4               :1;
        unsigned UNKNOWN5               :1;
        unsigned UNKNOWN6               :1;
        unsigned UNKNOWN7               :1;
        //unsigned UNKNOWN7               :1;
    };
    struct {
        unsigned ALL_ERRORS             :8;
    };
} internalErrorFlags_t;

typedef union {
    struct {
        unsigned LIN_TXRX_INIT              :1;
        unsigned NO_ACK_INIT                :1;        
        unsigned BAD_CONNECTION_INIT        :1;       
        unsigned LIN_TXRX_PROCESSING        :1;
        unsigned NO_ACK_PROCESSING          :1;
        unsigned BAD_CONNECTION_PROCESSING  :1;
        unsigned LIN_TXRX_SET               :1;
        unsigned UNKNOWN7                   :1;
    };
    struct {
        unsigned ALL_ERRORS             :8;
    };
} motorErrorFlags_t;

typedef union {
    struct {
        unsigned INIT_CORRECTORS        :2;
        //unsigned UNKNOWN1               :1;
        unsigned HALF_COUNTER           :1;
        unsigned EXT_CORRECTOR_VALUES   :1;
        unsigned UNKNOWN4               :1;
        unsigned UNKNOWN5               :1;
        unsigned UNKNOWN6               :1;
        unsigned UNKNOWN7               :1;
    };
    struct {
        unsigned ALL_FLAGS             :8;
    };
} currentFlags_t;

typedef struct  {
    uint8_t correctorsNum;
    //uint8_t defaultPosition;
    uint8_t positionsNum;
    uint8_t correctorPositionMult;
    uint8_t correctorsAdresses[2];
    int8_t correctorStartPositions[2];
    uint8_t adcValues[16-1];
    //uint8_t adcMaxDiff;
    int8_t correctorsValues[2][16];
    //uint8_t checksum;
} settings_t;
//52

typedef struct  {
    uint8_t temperature;
    uint8_t adcPositionValue;
    uint8_t positionIndex;
    int16_t rdCorrectorValues[2];
    int16_t wrCorrectorValues[2];
    //uint16_t extCorrectorValues[2];
    currentFlags_t flags;
    internalErrorFlags_t internalErrors;
    motorErrorFlags_t motorErrors[2];
    //uint8_t counter;
    uint8_t displayed_error;
} currentValues_t;
    
#ifdef	__cplusplus
}
#endif

#endif	/* DEFINITIONS_H */

