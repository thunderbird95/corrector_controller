/* 
 * File:   amis_30621.h
 * Author: t-bird
 *
 * Created on 16 10 2020 , 23:56
 */
#include <stdint.h>
#include "hardware.h"

#ifndef AMIS_30621_H
#define	AMIS_30621_H

#ifdef	__cplusplus
extern "C" {
#endif
    
//
uint8_t writeCorrectorPosition(uint8_t address, int16_t position);

//
uint8_t readCorrectorPosition(int16_t* position, uint8_t address);

#ifdef	__cplusplus
}
#endif

#endif	/* AMIS_30621_H */

