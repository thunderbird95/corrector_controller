/* 
 * File:   common_memory.h
 * Author: t-bird
 *
 * Created on 17 ??????? 2020 ?., 0:45
 */

#include "definitions.h"

#ifndef COMMON_MEMORY_H
#define	COMMON_MEMORY_H

#ifdef	__cplusplus
extern "C" {
#endif

uint8_t linFrame[11];
settings_t settings;
currentValues_t currentValues;
uint8_t currentFrameIndex;

#ifdef	__cplusplus
}
#endif

#endif	/* COMMON_MEMORY_H */

