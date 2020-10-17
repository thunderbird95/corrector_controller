/* 
 * File:   settings_mgr.h
 * Author: t-bird
 *
 * Created on 17 10 2020 , 1:08
 */
#include "common_memory.h"

#ifndef SETTINGS_MGR_H
#define	SETTINGS_MGR_H

#ifdef	__cplusplus
extern "C" {
#endif

void loadSettings();
uint8_t checkSettings();
void loadDefaultSettings();
uint8_t writeSettings();


#ifdef	__cplusplus
}
#endif

#endif	/* SETTINGS_MGR_H */

