#ifndef __ICComm__
#define __ICComm__
#define ICComm_Mode_Master 0
#define ICComm_Mode_Slave 1
typedef struct __ICCommConfig__{
    char OperateMode;
    unsigned int PIN_RX;
    unsigned int PIN_SCK;
    unsigned int PIN_TX;
    unsigned int PIN_CSN;
    void* SPI_Inst;
}_ICCommConfig;
typedef _ICCommConfig* ICCommConfig;
void ICComm_Setup(ICCommConfig config);
void ICComm_WriteData(ICCommConfig config,void* ptr, int len);
void ICComm_ReadData(ICCommConfig config,void* ptr, int len);
#endif
