/******************************************************************************
 * fram_memory.h
 *
 * @date Jun 15, 2013
 * @author Artem Dementyev, Aaron Parks
 *****************************************************************************/
#include "globals.h"
#include "spi.h"
#include "../UserApp/myE-paperApp.h"

//void SPI_FRAM_Write_Enable_Latch(unsigned char portj_cs_pin);
void initFRAM(void);
void SPI_FRAM_Write_Enable_Latch(void);
unsigned char SPI_FRAM_Read_Status_Register(void);
void SPI_FRAM_Write_Memory(const uint8_t *bufferAddress,const uint8_t *bufferData, uint16_t lengthData);
uint8_t * SPI_FRAM_Read_Memory(const uint8_t *bufferAddress,uint16_t lengthData);
void SPI_FRAM_Enter_Sleep(void);
void SPI_FRAM_Wake_Up(void);
void SPI_FRAM_Read_Image(const uint8_t *bufferAddress,uint16_t lengthData);
uint8_t SPI_FRAM_Read(const uint8_t addressData);
void SPI_FRAM_Write(const uint8_t *bufferData, FRAM_Status* allData);
void SPI_FRAM_Status_Initialize(const uint8_t *addrData, const uint8_t *bufferData);
