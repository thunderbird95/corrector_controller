
#include "ext_control.h"
#include "settings_mgr.h"
#include "hardware.h"

// EXT COMMANDS: input frame has 11 bytes: E2 code data[0] data[1] data[2] data[3] data[4] data[5] data[6] data[7] checksum
//               output frame can have any bytes number
//               output frame codes:    0x35 - sendCurrentValues, 0x15 - sendSettings, 0x25 - sendAck
//               input frames codes:    0x00-0x07 - writePartOfSettings (part number = code - 0x07), 0x10 - writeSettingsToEeprom, 
//                                      0x11 - readSettingsFromEeprom, 0x12 - settingsRequest, 0x17 - writeCorrectorPositions, 0x18 - clearErrors
void sendCurrentValues()
{
    uint8_t* currentValuesAddress = (uint8_t*)(&currentValues);
    uint8_t cnt;
    uint8_t sum = CURRENT_VALUES_HDR;
    linWriteByte(0xE2);
    linWriteByte(CURRENT_VALUES_HDR);
    for (cnt = 0; cnt < sizeof(currentValues); cnt++)
    {
        linWriteByte(currentValuesAddress[cnt]);
        sum += currentValuesAddress[cnt];
    }
    linWriteByte(sum);
    currentFrameIndex = 0;
}

void waitExtCommand()
{
    uint8_t cnt;
    
    currentFrameIndex += linWaitData(&(linFrame[currentFrameIndex]), sizeof(linFrame) - currentFrameIndex, 98);
    
    if (currentFrameIndex < 11)
        return;
    if (linFrame[0] != 0xE2)
    {
        sendAck(3);
        currentFrameIndex = 0;
        return;
    }
    uint8_t sum = 0;
    for (cnt = 1; cnt < 10; cnt++)
        sum += linFrame[cnt];
    if (sum != linFrame[10])
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
            sum = writeSettings();
            sendAck(sum);
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
            sum = 0x15;
            for (cnt = 0; cnt < sizeof(settings); cnt++)
            {
                linWriteByte(settingsAddress[cnt]);
                sum += settingsAddress[cnt];
            }
            linWriteByte(sum);
            break;
        case 0x17:
            // WRITE CORRECTORS POSITIONS
            currentValues.wrCorrectorValues[0] = linFrame[2] | (linFrame[3] << 8);
            currentValues.wrCorrectorValues[1] = linFrame[4] | (linFrame[5] << 8);
            if (linFrame[6] == 0)
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