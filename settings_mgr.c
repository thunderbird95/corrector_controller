
#include "settings_mgr.h"
#include "hardware.h"

void loadSettings()
{
    uint8_t* settingsAddress = (uint8_t*)(&settings);
    uint8_t cnt;
    for (cnt = 0; cnt < sizeof(settings); cnt++)
        settingsAddress[cnt] = DATAEE_ReadByte(cnt);
}

uint8_t checkSettings()
{
    uint8_t* settingsAddress = (uint8_t*)(&settings);
    //uint8_t checksum = 0;
    uint8_t cnt;
    if (settings.correctorsNum > 2)
        return 1;
    if (settings.positionsNum > 16)
        return 2;
//    for (cnt = 0; cnt < (sizeof(settings) - 1); cnt++)
//        checksum += settingsAddress[cnt];
//    if (settings.checksum != checksum)
//        return 3;
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