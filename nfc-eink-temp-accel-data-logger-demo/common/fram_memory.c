/******************************************************************************
 * fram_memory.c
 * @date Jun 15, 2013
 * @author Artem Dementyev, Aaron Parks,Eve(Yi Zhao)
 *****************************************************************************/

#include "fram_memory.h"
#include "e-paper.h"
#include "../UserApp/myE-paperApp.h"

extern struct FRAM_Status;
uint8_t const FRAM_Sleep[] = {0xB9};
uint8_t const ReadStatus[] = {0x05,0x00};
uint8_t const ReadDeviceID[]={0x9F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t const WriteMemData[] = {0x02};
uint8_t const ReadMemData[] = {0x03};

void SPI_FRAM_Wake_Up(void) {
	// CS low
	SPI_CS_MEM_OUT &= ~SPI_CS_MEM_BIT;
	delay_us(T_10us);

	lowPowerSleep(LPM_500us);

	// CS high
	SPI_CS_MEM_OUT |= SPI_CS_MEM_BIT;
}

void SPI_FRAM_Write_Enable_Latch(void) {
	// CS low
	SPI_CS_MEM_OUT &= ~SPI_CS_MEM_BIT;

	//send the op code
	uint16_t i;
	for (i = 0; i < 1; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = 0x06; //op code for enabling write latch (WREN)
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	// CS high
	SPI_CS_MEM_OUT |= SPI_CS_MEM_BIT;
}

void SPI_FRAM_Enter_Sleep() {
	//Note: must make sure SPI clock is SMCLK =13MHz
	// Select MEMERY SPI
	SPI_transaction(gpRxBuf,(uint8_t*)&FRAM_Sleep, sizeof(FRAM_Sleep),FRAM_SPI);
	//Disable SPI
}

uint8_t SPI_FRAM_Read_Status_Register(void) {

	SPI_transaction(gpRxBuf,(uint8_t*)&ReadStatus, sizeof(ReadStatus),FRAM_SPI);
	return gpRxBuf[1];
}

void SPI_FRAM_Write(const uint8_t *bufferData, FRAM_Status* allData) {
	// CS low
	//SPI_CS_MEM_OUT &= ~SPI_CS_MEM_BIT;
	//op code: 0x02 for write data
	SPI_transaction(gpRxBuf,(uint8_t*)&WriteMemData, sizeof(WriteMemData),FRAM_SPI);
//	SPI_FRAM_Status_Write(0x00000, bufferData)
	SPI_transaction(gpRxBuf, (uint8_t*)&(allData->currentAddress), sizeof(allData->currentAddress), FRAM_SPI);
	SPI_transaction(gpRxBuf,(uint8_t*)&bufferData, sizeof(bufferData),FRAM_SPI);

	// CS high
	//SPI_CS_MEM_OUT |= SPI_CS_MEM_BIT;
/*	*d.currentAddress += 7;
	if (*d.currentAddress >= 262144) {
		d.overflow = 1;
		d.currentAddress = 0x00007;
	}
	if (*d.currentAddress == 0x00007) {
		d.overflow = 0;
	}
	d.countOfPack += 1;*/

}

uint8_t SPI_FRAM_Read(const uint8_t addressData) {
	// CS low
	SPI_CS_MEM_OUT &= ~SPI_CS_MEM_BIT;
	//op code: 0x02 for write data
	SPI_transaction(gpRxBuf,(uint8_t*)&ReadMemData, sizeof(ReadMemData),FRAM_SPI);
	SPI_transaction(gpRxBuf,(uint8_t*)&addressData, sizeof(addressData),FRAM_SPI);
	SPI_transaction(gpRxBuf,(uint8_t*)&addressData, sizeof(addressData),FRAM_SPI);
	// CS high
	SPI_CS_MEM_OUT |= SPI_CS_MEM_BIT;
	return gpRxBuf[0];
}

// Initializes the constant values: length,time interval
/*void SPI_FRAM_Status_Initialize(uint8_t *addrData, uint8_t *bufferData) {
	SPI_transaction(gpRxBuf,(uint8_t*)&WriteMemData, sizeof(WriteMemData),FRAM_SPI);
	SPI_transaction(gpRxBuf,(uint8_t*)&addrData, sizeof(1),FRAM_SPI);
	SPI_transaction(gpRxBuf,(uint8_t*)&bufferData, sizeof(bufferData),FRAM_SPI);
	SPI_transaction(gpRxBuf,(uint8_t*)&bufferData, sizeof(bufferData),FRAM_SPI);

}
*/
void SPI_FRAM_Status_Update(uint8_t *addrData, uint8_t *bufferData) {
	SPI_transaction(gpRxBuf,(uint8_t*)&WriteMemData, sizeof(WriteMemData),FRAM_SPI);
	SPI_transaction(gpRxBuf,(uint8_t*)&addrData, sizeof(addrData),FRAM_SPI);
	SPI_transaction(gpRxBuf,(uint8_t*)&bufferData, sizeof(bufferData),FRAM_SPI);
}

void SPI_FRAM_Write_Memory(const uint8_t *bufferAddress,
		const uint8_t *bufferData, uint16_t lengthData) {

	// CS low
	SPI_CS_MEM_OUT &= ~SPI_CS_MEM_BIT;

	// send the op code
	uint16_t i;
	for ( i = 0; i < 1; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = 0x02; //op code for writing memory (WRITE)
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	//send the 18-bit address of first data byte (we use 8 * 3 = 24 bits to represent address)

	for (i = 0; i < 3; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = *bufferAddress++;
	}

	// send the 8-byte data
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	for ( i = 0; i < lengthData; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = *bufferData++;
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	// CS high
	SPI_CS_MEM_OUT |= SPI_CS_MEM_BIT;

} //end SPI write memory

uint8_t * SPI_FRAM_Read_Memory(const uint8_t *bufferAddress,
		uint16_t lengthData) {

	// TODO We don't have space in RAM for the following array.
	//uint8_t testArray[5807];

	// CS low
	SPI_CS_MEM_OUT &= ~SPI_CS_MEM_BIT;

	// send the op code
	uint16_t i;
	for (i = 0; i < 1; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = 0x03; //op code for reading memory (READ)
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	//send the 18-bit address
	for ( i = 0; i < 3; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = *bufferAddress++;
	}

	// clock out the 8-byte data
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	for (i = 0; i < lengthData; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = 0xff;
		gpRxBuf[i] = UCB1RXBUF;
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	// CS high
	SPI_CS_MEM_OUT |= SPI_CS_MEM_BIT;

	// TODO This will fail in many circumstances: We're returning a pointer to a local variable.
	return gpRxBuf;

} //end SPI write memory

void SPI_FRAM_Read_Image(const uint8_t *bufferAddress,
		uint16_t lengthData) {
	// CS low

	//uint8_t testArray[5807];
	SPI_CS_MEM_OUT &= ~SPI_CS_MEM_BIT;

	// send the op code
	uint16_t i;
	for (i = 0; i < 1; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = 0x03; //op code for reading memory (READ)
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	//send the 18-bit address
	for (i= 0; i < 3; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = *bufferAddress++;
	}

	// clock out the 8-byte data
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	for (i = 0; i < lengthData; ++i) {
		while (0 == (UCB1IFG & UCTXIFG)) {
		}
		UCB1TXBUF = 0xff;
		imageBuffer[i] = UCB1RXBUF;
	}

	// wait for last byte to clear SPI
	while (0 != (UCB1STAT & UCBUSY)) {
	}

	// CS high
	SPI_CS_MEM_OUT |= SPI_CS_MEM_BIT;
} //end SPI write memory

/**
 * Initialize the FRAM module and put it to sleep
 *
 * @note must wait 1ms before SPI
 */
void initFRAM(void) {

	/*sleep 1ms to allow FRAM startup*/
	lowPowerSleep(LPM_1ms);

	/*Coomment it out to test if SPI works well,
	 *the status register should return 0x40*/
	//SPI_FRAM_Read_Status_Register();

	/* Put FRAM to sleep*/
	SPI_FRAM_Enter_Sleep();
}

