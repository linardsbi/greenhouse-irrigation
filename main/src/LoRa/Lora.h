/**
 * E32-TTL-100 Transceiver Interface
 *
 * @author Bob Chen (bob-0505@gotmail.com)
 * @date 1 November 2017
 * https://github.com/Bob0505/E32-TTL-100
 * 
 * modified by Linards Biezbardis
 */
#ifndef LORA_H_
#define LORA_H_

#include <SoftwareSerial.h>
#include <Arduino.h>

typedef enum {
  RET_SUCCESS = 0,
  RET_ERROR_UNKNOWN,	/* something shouldn't happened */
  RET_NOT_SUPPORT,
  RET_NOT_IMPLEMENT, // TODO: implement functionality
  RET_NOT_INITIAL,
  RET_INVALID_PARAM,
  RET_DATA_SIZE_NOT_MATCH,
  RET_BUF_TOO_SMALL,
  RET_TIMEOUT,
  RET_HW_ERROR,
  RET_BUF_EMPTY
} RET_STATUS;

enum MODE_TYPE
{
  MODE_0_NORMAL = 0,
  MODE_1_WAKE_UP,
  MODE_2_POWER_SAVIN,
  MODE_3_SLEEP,
  MODE_INIT = 0xFF
};

//SPED+
enum SLEEP_MODE_CMD_TYPE
{
  W_CFG_PWR_DWN_SAVE = 0xC0,
  R_CFG              = 0xC1,
  W_CFG_PWR_DWN_LOSE = 0xC2,
  R_MODULE_VERSION   = 0xC3,
  W_RESET_MODULE     = 0xC4
};

enum UART_FORMAT_TYPE
{
  UART_FORMAT_8N1 = 0x00,  /*no   parity bit one stop*/
  UART_FORMAT_8O1 = 0x01,  /*odd  parity bit one stop*/
  UART_FORMAT_8E1 = 0x02   /*even parity bitone stop*/
};

enum UART_BPS_TYPE
{
  UART_BPS_1200 = 0x00,
  UART_BPS_9600 = 0x03,
  UART_BPS_115200 = 0x07
};

enum AIR_BPS_TYPE
{
  AIR_BPS_300   = 0x00,
  AIR_BPS_2400  = 0x02,
  AIR_BPS_19200 = 0x05
};
//SPED-

//410~441MHz : 410M + CHAN*1M
//855~880.5MHz : 855 + channel * 0.1M
// eg. channel val (in decimal) = 441 - 410 * 1M || channel val (dec) = (880.5 - 855) * 10M
enum AIR_CHAN_TYPE
{
  AIR_CHAN_410M = 0x00,
  AIR_CHAN_433M = 0x17,
  AIR_CHAN_441M = 0x1F,
  AIR_CHAN_855M = 0x00,
  AIR_CHAN_868M = 0x82,
  AIR_CHAN_880M = 0xFF
};

//OPTION+
#define TRSM_TT_MODE		0x00	/*Transparent Transmission*/
#define TRSM_FP_MODE		0x01	/*Fixed-point transmission mode*/

#define OD_DRIVE_MODE		0x00
#define PP_DRIVE_MODE		0x01

enum WEAK_UP_TIME_TYPE
{
  WEAK_UP_TIME_250  = 0x00,
  WEAK_UP_TIME_1000 = 0x03,
  WEAK_UP_TIME_2000 = 0x07
};

#define DISABLE_FEC			0x00
#define ENABLE_FEC			0x01

//Transmit power
enum TSMT_PWR_TYPE
{
  TSMT_PWR_20DB = 0x00,
  TSMT_PWR_17DB = 0x01,
  TSMT_PWR_14DB = 0x02,
  TSMT_PWR_10DB = 0x03
};
//OPTION-

#pragma pack(push, 1)
struct SPEDstruct {
  uint8_t air_bps : 3; //bit 0-2
  uint8_t uart_bps: 3; //bit 3-5
  uint8_t uart_fmt: 2; //bit 6-7
};

struct OPTIONstruct {
  uint8_t tsmt_pwr    : 2; //bit 0-1
  uint8_t enFWC       : 1; //bit 2
  uint8_t wakeup_time : 3; //bit 3-5
  uint8_t drive_mode  : 1; //bit 6
  uint8_t trsm_mode   : 1; //bit 7
};

struct CFGstruct {
  uint8_t HEAD;
  uint8_t ADDH;
  uint8_t ADDL;
  struct SPEDstruct   SPED_bits;
  uint8_t CHAN;
  struct OPTIONstruct OPTION_bits;
};

struct MVerstruct {
  uint8_t HEAD;
  uint8_t Model;
  uint8_t Version;
  uint8_t features;
};
#pragma pack(pop) // tell compiler to create struct that matches UART data

/**
 * T_PING is used to check if there is someone to receive message
 * T_DATA tells receiver that data will be sent
 * 
 * MSG_DELIMITER is used to tell receiver that the number that follows is to be added to number before MSG_DELIMITER
 * MSG_TERMINATOR terminates the message
 */
enum MessageParameters {
    T_PING = 0x00,
    T_HASH_CHECK,
    T_DATA,
    MSG_PAIR_START = 0xFD,
    MSG_PAIR_END = 0xFE,
    MSG_TERMINATOR = 0xFF
};

#define TIME_OUT_CNT	100
#define MAX_TX_SIZE		58

#endif

class Lora {
    public:

        static RET_STATUS startLoraModule();
        static RET_STATUS pingModule();

        static RET_STATUS Lora::createAndSendBuffer(const std::vector<std::vector<unsigned>> &buffer);

        static RET_STATUS SendMsg(const uint8_t *SendBuf, const uint8_t msgLen);
        static RET_STATUS ReceiveMsg(uint8_t *pdatabuf, uint8_t *data_len);

        static RET_STATUS writeSettings(struct CFGstruct *pCFG);
        static RET_STATUS SleepModeCmd(uint8_t CMD, void* pBuff);
        static RET_STATUS WaitAUX_H(); // wait for module to initialize
        
        static void SwitchMode(MODE_TYPE mode);

    private:

        static void resetModule();
        static void triple_cmd(SLEEP_MODE_CMD_TYPE Tcmd);
        static void cleanUARTBuf();

        static RET_STATUS Read_module_version(struct MVerstruct* MVer);
        static RET_STATUS Read_CFG(struct CFGstruct* pCFG);
        static RET_STATUS ReadModuleInfo(uint8_t* pReadbuf, uint8_t buf_len);
        static RET_STATUS Write_CFG_PDS(struct CFGstruct* pCFG);

        static bool chkModeSame(MODE_TYPE mode);
        static bool ReadAUX();
};
