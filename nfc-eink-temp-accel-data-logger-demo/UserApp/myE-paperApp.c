/******************************************************************************
 * @file	myE-paperApp.c
 *
 * @brief	Demo of customized E-paper update
 *
 * @details Use the E-paper driver in common/e-paper.h to determine how to
 * 			update the display line by line.
 *
 * @note 	More details about driver reference can be found
 * 			http://www.pervasivedisplays.com/products/27
 *
 * @author	Eve (Yi Zhao) - Sensor Systems Lab (UW)
 * @date	09/29/2014
 *****************************************************************************/
#include "../common/e-paper.h"
#include "tempSense.h"
#include "../common/globals.h"
#include "../common/accel.h"
#include "myE-paperApp.h"
#include "../common/fram_memory.h"

uint8_t dataAddress= 0x00007; // First three bytes for address; two bytes for current number of packagee; in next one byte,
								//first 4 bits are for number of bytes per package
								// next 3 bits are for time interval and the last bit is for overflow flag

struct FRAM_Status{
	uint8_t curentAddress;
	uint8_t lengthPerPac;
	unsigned char interval;
	unsigned char overflow;
}mystruct;


/**
*	@brief	Update EPD display from image templete and display real time sensor data
*	@param	imgPtr - the pointer pointed to the entry of image templete
*	@time 	EPD powerOn&Off takes around 325ms
*/


uint8_t updateDisplay1(unsigned char* imagePtr){

	volatile resultData newData;
	uint8_t dataPack[7];
	//Sample temperature
	//newData = tempSense(&allData);
	tempSense(&dataPack);
	//Sample Accelerometer
	 uint8_t value,indicator;
	 //uint8_t result[4];

	 //ACCEL_Status(&st);
	 //allData volatile d;
	 //allData.currentAddress = dataAddress;
	 ACCEL_singleSample(&dataPack);// read status of accel and 3 axis acceleration
	 if(((dataPack[6]&0xF0)==0x40)){
		 value=0x00;
		 indicator =4; //update motion indicator
	 }else {
		 value = 0xFF;
		 if(indicator>0)indicator--;//reduce counter in order to stop indicator after refresh
	 }
	 FRAM_Status mystruct;
	 mystruct.curentAddress = 0x09;
	 // store the x,y, temperature data and x,y,z acceleration, motion data to one allData
	 //uint8_t allData[7];
	 /*uint8_t j;
	 allData[0] = newData.x;
	 allData[1] = newData.y;
	 allData[2] = newData.ADC_data;
	 for (j = 0; j < 3; j ++) {
		 allData[j + 3] = result[j];
	 }*/
//	uint8_t address[3];
//	address[0];
	// SPI_FRAM_Status_Initialize(const uint8_t *addrData, const uint8_t *bufferData) {
	dataPack[0] =103;
	uint8_t a = 0x08;
	SPI_FRAM_Write(&(dataPack[0]), &mystruct);
//	SPI_FRAM_Status_Update(uint8_t *addrData, uint8_t *bufferData);
	uint8_t previousData = SPI_FRAM_Read(a);
	//Update E-ink based on data
	 if(EPD_power_init()==RES_OK){

		 //Update E-ink frame
		 if(imageUpdateState== IMG_UPDATE){
			 EPD_frame_newImg(imagePtr,0,cog.lines_per_display);
			 imageUpdateState = SENSE_UPDATE;
			 senseState=0;
		 }

		//Update motion indicator
		if(indicator)EPD_frame_singleDot(18,24,28,value);

		//Update temerature result in the plot
		EPD_frame_singleDot(newData.y,newData.y+1,newData.x,0xC3);
		return EPD_power_off();
	 }
	return FAIL;
}

/**
*	@brief	Update EPD display from imgBuffer
*	@param	imgPtr - the pointer pointed to the start of updated content
*	@time 	EPD powerOn&Off takes around 325ms
*	@note 	imgBuffer is filled from NFC I-block reading in myNFC_protocol.c,
*			the updateDisplay2 is only be called when receive whole image
*	@note	NFC-WISP Reader App 1.0 will sending dummy data for power
*			after finishing image transfer.
*			If you build *BUILD_E_INK_2_0_BATT_FREE*
*			we add more lowPowerSleep during E-ink updating to reive more energy from cell-phone
*/

uint8_t updateDisplay2(unsigned char* imagePtr){
	 //Update E-ink based on data
	 if(EPD_power_init()==RES_OK){

		 //Update E-ink frame
		 if(imageUpdateState){
			 EPD_frame_newImg(imagePtr,0,cog.lines_per_display);
		 }

		return EPD_power_off();
	 }
	return FAIL;

	//TODO:indicate end update
	//LED_2_BIT_auto_pulse();
}





