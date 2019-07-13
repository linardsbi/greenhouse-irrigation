#include <vector>
#include "Lora.h"
#include "../Define.h"

SoftwareSerial softSerial(SOFT_RX, SOFT_TX);

RET_STATUS Lora::startLoraModule() {
    RET_STATUS STATUS = RET_SUCCESS;
    
    struct CFGstruct CFG;
    struct MVerstruct MVer;

    pinMode(M0_PIN, OUTPUT);
    pinMode(M1_PIN, OUTPUT);
    pinMode(AUX_PIN, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    softSerial.begin(9600);

    STATUS = SleepModeCmd(R_CFG, (void* )&CFG); // read configuration
    STATUS = writeSettings(&CFG); // write settings from Lora.h

    STATUS = SleepModeCmd(R_MODULE_VERSION, (void* )&MVer);

    // Mode 0 | normal operation
    SwitchMode(MODE_0_NORMAL);

    //self-check initialization.
    WaitAUX_H();
    delay(10);

    return STATUS;
}

RET_STATUS Lora::pingModule() {
    uint8_t buffer[7] = {DEVICE_ADDR_H, DEVICE_ADDR_L, DEVICE_ADDR_CHANNEL, T_PING, 0x01, MSG_TERMINATOR};

    return SendMsg(buffer, 7);
}

// todo: find a better place for this
enum {
    SENS_TYPE = 0,
    SENS_VALUE
};

// todo: this. make it less messy
// segments large numbers
RET_STATUS Lora::createAndSendBuffer(const std::vector<std::vector<unsigned>> &buffer) {

    int tempVal = 0;
    std::vector<uint8_t> newBuffer = {DEVICE_ADDR_H, DEVICE_ADDR_L, DEVICE_ADDR_CHANNEL, T_DATA};

    for (auto pair: buffer) {
        newBuffer.push_back(MSG_PAIR_START);
        newBuffer.push_back(pair[SENS_TYPE]);

        tempVal = pair[SENS_VALUE];

        if (tempVal < 252) {
            newBuffer.push_back(pair[SENS_VALUE]);
        } else {
            for (;;) {
                tempVal -= 252;
                if (tempVal < 0) {
                    newBuffer.push_back(tempVal + 252);
                    break;
                } else if (tempVal == 0) {
                    newBuffer.push_back(252);
                    break;
                } else {
                    newBuffer.push_back(252);
                }
            }
        }

        newBuffer.push_back(MSG_PAIR_END);
    }

    newBuffer.push_back(MSG_TERMINATOR);

    // Serial.print("newBuffer: ");
    // for (auto val: newBuffer) {
        
    //     Serial.print(val);
    //     Serial.print(", ");
    // }
    // Serial.println();

    return SendMsg(&newBuffer[0], newBuffer.size());
}

RET_STATUS Lora::SendMsg(const uint8_t *SendBuf, const uint8_t msgLen)
{
    RET_STATUS STATUS = RET_SUCCESS;

    // if (msgLen > 32) return RET_INVALID_PARAM;

    SwitchMode(MODE_0_NORMAL);

    if(ReadAUX()!=HIGH)
    {
        return RET_NOT_IMPLEMENT;
    }
    delay(10);
    if(ReadAUX()!=HIGH)
    {
        return RET_NOT_IMPLEMENT;
    }

    //Send format : ADDH ADDL CHAN DATA_0 DATA_1 DATA_2 ...
    softSerial.write(SendBuf, msgLen);

    return STATUS;
}

RET_STATUS Lora::ReceiveMsg(uint8_t *pdatabuf, uint8_t *data_len) {
    RET_STATUS STATUS = RET_SUCCESS;
    uint8_t idx;

    SwitchMode(MODE_0_NORMAL);
    *data_len = softSerial.available();

    if (*data_len > 0)
    {
        Serial.print("ReceiveMsg: ");  Serial.print(*data_len);  Serial.println(" bytes.");

        for(idx=0;idx<*data_len;idx++){
            *(pdatabuf+idx) = softSerial.read();
        }
        
        // todo: implement message parsing
        for(idx=0;idx<*data_len;idx++)
        {
            #ifdef DEBUG
                Serial.print(" 0x");
                Serial.print(0xFF & *(pdatabuf+idx), HEX);    // print as an ASCII-encoded hexadecimal
            #endif
        } 

        Serial.println("");
    }
    else
    {
        STATUS = RET_BUF_EMPTY;
    }

    return STATUS;
}

RET_STATUS Lora::writeSettings(struct CFGstruct *pCFG)
{
    RET_STATUS STATUS = RET_SUCCESS;

    pCFG->ADDH = DEVICE_ADDR_H;
    pCFG->ADDL = DEVICE_ADDR_L;

    pCFG->OPTION_bits.trsm_mode = TRSM_TT_MODE; // transparent transmission
    pCFG->OPTION_bits.tsmt_pwr = TSMT_PWR_10DB;

    pCFG->CHAN = AIR_CHAN_868M;

    STATUS = SleepModeCmd(W_CFG_PWR_DWN_SAVE, (void*)pCFG);

    SleepModeCmd(W_RESET_MODULE, NULL);

    STATUS = SleepModeCmd(R_CFG, (void* )pCFG);

    return STATUS;
}

RET_STATUS Lora::SleepModeCmd(uint8_t CMD, void* pBuff)
{
    RET_STATUS STATUS = RET_SUCCESS;

    #ifdef DEBUG
        Serial.print("SleepModeCmd: 0x");  Serial.println(CMD, HEX);
    #endif
    
    WaitAUX_H();

    SwitchMode(MODE_3_SLEEP);

    switch (CMD)
    {
        case W_CFG_PWR_DWN_SAVE:
            STATUS = Write_CFG_PDS((struct CFGstruct* )pBuff);
            break;
        case R_CFG:
            STATUS = Read_CFG((struct CFGstruct* )pBuff);
            break;
        case W_CFG_PWR_DWN_LOSE:
            break;
        case R_MODULE_VERSION:
            STATUS = Read_module_version((struct MVerstruct* )pBuff);
            break;
        case W_RESET_MODULE:
            resetModule();
            break;

        default:
            return RET_INVALID_PARAM;
    }

    WaitAUX_H();
    return STATUS;
}

void Lora::resetModule()
{
    triple_cmd(W_RESET_MODULE);

    WaitAUX_H();
    delay(1200);
}

RET_STATUS Lora::Read_module_version(struct MVerstruct* MVer)
{
    RET_STATUS STATUS = RET_SUCCESS;

    //1. read UART buffer.
    cleanUARTBuf();

    //2. send CMD
    triple_cmd(R_MODULE_VERSION);

    //3. Receive configure
    STATUS = ReadModuleInfo((uint8_t *)MVer, sizeof(MVerstruct));

    #ifdef DEBUG
        if(STATUS == RET_SUCCESS)
        {
            Serial.print("  HEAD:     0x");  Serial.println(MVer->HEAD, HEX);
            Serial.print("  Model:    0x");  Serial.println(MVer->Model, HEX);
            Serial.print("  Version:  0x");  Serial.println(MVer->Version, HEX);
            Serial.print("  features: 0x");  Serial.println(MVer->features, HEX);
        }
    #endif

    return STATUS;
}

RET_STATUS Lora::Read_CFG(struct CFGstruct* pCFG)
{
    RET_STATUS STATUS = RET_SUCCESS;

    //1. read UART buffer.
    cleanUARTBuf();

    //2. send CMD
    triple_cmd(R_CFG);

    //3. Receive configure
    STATUS = ReadModuleInfo((uint8_t *)pCFG, sizeof(CFGstruct));

    #ifdef DEBUG
        if(STATUS == RET_SUCCESS)
        {
            Serial.print("  HEAD:     ");  Serial.println(pCFG->HEAD, HEX);
            Serial.print("  ADDH:     ");  Serial.println(pCFG->ADDH, HEX);
            Serial.print("  ADDL:     ");  Serial.println(pCFG->ADDL, HEX);

            Serial.print("  CHAN:     ");  Serial.println(pCFG->CHAN, HEX);
        }
    #endif
    

    return STATUS;
}

RET_STATUS Lora::Write_CFG_PDS(struct CFGstruct* pCFG)
{
    softSerial.write((uint8_t *)pCFG, 6);

    WaitAUX_H();
    delay(1000);  //need ti check

    return RET_SUCCESS;
}

RET_STATUS Lora::ReadModuleInfo(uint8_t* pReadbuf, uint8_t buf_len)
{
    RET_STATUS STATUS = RET_SUCCESS;
    uint8_t Readcnt, idx;

    Readcnt = softSerial.available();
    //Serial.print("softSerial.available(): ");  Serial.print(Readcnt);  Serial.println(" bytes.");
    if (Readcnt == buf_len)
    {
        for(idx=0;idx<buf_len;idx++)
        {
            *(pReadbuf+idx) = softSerial.read();
            
            Serial.print(" 0x");
            Serial.print(0xFF & *(pReadbuf+idx), HEX);    // print as an ASCII-encoded hexadecimal
        } 

        Serial.println("");
    }
    else
    {
        STATUS = RET_DATA_SIZE_NOT_MATCH;
        Serial.print("  RET_DATA_SIZE_NOT_MATCH - Readcnt length: ");  Serial.println(Readcnt);
        cleanUARTBuf();
    }

    return STATUS;
}

void Lora::triple_cmd(SLEEP_MODE_CMD_TYPE Tcmd)
{
    uint8_t CMD[3] = {Tcmd, Tcmd, Tcmd};
    softSerial.write(CMD, 3);
    delay(50);  //need ti check
}

void Lora::cleanUARTBuf()
{
    bool IsNull = true;

    while (softSerial.available())
    {
        IsNull = false;

        softSerial.read();
    }
}

void Lora::SwitchMode(MODE_TYPE mode)
{
    if(!chkModeSame(mode))
    {
        WaitAUX_H();

        switch (mode)
        {
        case MODE_0_NORMAL:
            // Mode 0 | normal operation
            digitalWrite(M0_PIN, LOW);
            digitalWrite(M1_PIN, LOW);
            break;
        case MODE_1_WAKE_UP:
            digitalWrite(M0_PIN, HIGH);
            digitalWrite(M1_PIN, LOW);
            break;
        case MODE_2_POWER_SAVIN:
            digitalWrite(M0_PIN, LOW);
            digitalWrite(M1_PIN, HIGH);
            break;
        case MODE_3_SLEEP:
            // Mode 3 | Setting operation
            digitalWrite(M0_PIN, HIGH);
            digitalWrite(M1_PIN, HIGH);
            break;
        default:
            return RET_INVALID_PARAM;
        }

        WaitAUX_H();
        delay(10);
    }
}

bool Lora::chkModeSame(MODE_TYPE mode)
{
    static MODE_TYPE pre_mode = MODE_INIT;

    if(pre_mode == mode)
    {
        //Serial.print("SwitchMode: (no need to switch) ");  Serial.println(mode, HEX);
        return true;
    }
    else
    {
        #ifdef DEBUG
            Serial.print("SwitchMode: from ");  
            Serial.print(pre_mode, HEX);  
            Serial.print(" to ");  
            Serial.println(mode, HEX);
        #endif
        
        pre_mode = mode;
        return false;
    }
}

RET_STATUS Lora::WaitAUX_H()
{
    RET_STATUS STATUS = RET_SUCCESS;

    uint8_t cnt = 0;
    uint8_t data_buf[100], data_len;

    while((ReadAUX()==LOW) && (cnt++<TIME_OUT_CNT))
    {
        Serial.print(".");
        delay(100);
    }

    if(cnt==0) {}
    else if(cnt>=TIME_OUT_CNT)
    {
        STATUS = RET_TIMEOUT;
        Serial.println(" TimeOut");
    }
    else
    {
        Serial.println("");
    }

    return STATUS;
}

bool Lora::ReadAUX()
{
    unsigned val = analogRead(AUX_PIN);

    if(val<50) {
        return LOW;
    } else {
        return HIGH;
    }
}

void blinkLED()
{
  static bool LedStatus = LOW;

  digitalWrite(LED_BUILTIN, LedStatus);
  LedStatus = !LedStatus;
}