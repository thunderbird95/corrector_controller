/* 
 * File:   ext_protocol.h
 * Author: t-bird
 *
 * Created on 17 ??????? 2020 ?., 1:23
 */

#include "common_memory.h"

#ifndef EXT_PROTOCOL_H
#define	EXT_PROTOCOL_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define CURRENT_VALUES_HDR  0x35

//
void sendCurrentValues();

//
void waitExtCommand();

//
void sendAck(uint8_t code);

#ifdef	__cplusplus
}
#endif

#endif	/* EXT_PROTOCOL_H */

