#include "../../interface/ICComm.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

void ICComm_Setup(ICCommConfig config)
{

    spi_init(config->SPI_Inst, 1000 * 1000);
    switch (config->OperateMode)
    {
    case ICComm_Mode_Master:
        /* code */
        break;
    case ICComm_Mode_Slave:
        spi_set_slave(config->SPI_Inst, true);
        break;
    default:
        break;
    }
    gpio_set_function(config->PIN_RX, GPIO_FUNC_SPI);
    gpio_set_function(config->PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(config->PIN_TX, GPIO_FUNC_SPI);
    gpio_set_function(config->PIN_CSN, GPIO_FUNC_SPI);
}

void ICComm_WriteData(ICCommConfig config, void *ptr, int len)
{
    spi_write_blocking(config->SPI_Inst, ptr, len);
}
void ICComm_ReadData(ICCommConfig config, void *ptr, int len)
{
    spi_read_blocking(config->SPI_Inst, 0, ptr, len);
}