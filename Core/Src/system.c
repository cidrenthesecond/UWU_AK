/*
 * system.c
 *
 *  Created on: Jan 8, 2026
 *      Author: 1
 */

#include "main.h"
#include "system.h"
#include "spi_project.h"
#include "doublebuffer.h"

//variables holding information on current file played
volatile uint32_t currentAddr;
volatile uint32_t currentFileAddr;
volatile uint32_t currentFileLength;

//structs holding files info
static struct fileData BOOTFile = {BOOTAddr,BOOTLength};
static struct fileData CLICKFile = {CLICKAddr,CLICKLength};
static struct fileData SHOOTFile = {SHOOTAddr,SHOOTLength};

//
volatile enum file filePlayed = NONE;

void SystemBoot()
{
	LL_GPIO_ResetOutputPin(LED_GPIO_Port, LED_Pin); //Turn off LED
	LL_GPIO_ResetOutputPin(NWP_GPIO_Port, NWP_Pin); //FLASH WriteProtect ON
	LL_GPIO_SetOutputPin(NSD_GPIO_Port,NSD_Pin); //Turn off amplifier
	LL_GPIO_SetOutputPin(NCS_GPIO_Port,NCS_Pin); //unselect FLASH

	//PWM configuration
	TIM1->CCR1 = PWM_STARTUP_VAL; //LSB
	TIM1->CCR4 = PWM_STARTUP_VAL; //MSB
	LL_TIM_ClearFlag_UPDATE(TIM1);
	LL_TIM_EnableAllOutputs(TIM1);
	LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH4);
	LL_TIM_EnableIT_UPDATE(TIM1);


	//SPI ON
	LL_SPI_Enable(SPI1);
	(void)LL_SPI_ReceiveData8(SPI1);

	/* GOES TO GPIO.c
	//Turn off NVIC, BOOT cannot be interrupted
	LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_8);
	LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_11);
	*/

	selectFileToPlay(BOOT);
}

//dodac obsluge jako obiekt to bedzie w miare uniwersalny kod
void BlinkLED(uint8_t blinks)
{
	for(uint8_t timesBlinked = 0; timesBlinked < blinks; timesBlinked++)
	{
		LL_GPIO_SetOutputPin(LED_GPIO_Port, LED_Pin);
		for(uint32_t indx = 0; indx <= 900000; indx++);
		LL_GPIO_ResetOutputPin(LED_GPIO_Port, LED_Pin);
		for(uint32_t indx = 0; indx <= 900000; indx++);
	}
}

void PlayFile(enum file fileToPlay)
{
	switch(fileToPlay)
	{
		case BOOT:
			currentAddr = BOOTFile.startAddr;
			currentFileAddr = BOOTFile.startAddr;
			currentFileLength = BOOTFile.length;
			filePlayed = BOOT;
			break;
		case CLICK:
			currentAddr = CLICKFile.startAddr;
			currentFileAddr = CLICKFile.startAddr;
			currentFileLength = CLICKFile.length;
			filePlayed = CLICK;
			break;
		case SHOOT:
			currentAddr = SHOOTFile.startAddr;
			currentFileAddr = SHOOTFile.startAddr;
			currentFileLength = SHOOTFile.length;
			filePlayed = SHOOT;
			break;
		default:
			while(1) LL_GPIO_SetOutputPin(LED_GPIO_Port, LED_Pin); //error break
			break;
	}

	GetEmptyBuffer();

	LL_SPI_Enable(SPI1);
	FAST_READ_IT(currentAddr,BUFFER_SIZE);

	while(!IsAnyBufferReady());

	GetReadyBuffer();
	LL_GPIO_SetOutputPin(NSD_GPIO_Port, NSD_Pin); // wlacz wzmacniacz
	LL_TIM_EnableIT_UPDATE(TIM1);
	LL_TIM_EnableCounter(TIM1); //wlacz timer
}


volatile uint8_t PWM_repetition = 0;
volatile uint8_t dataToPlay = 0;
volatile uint8_t dataQueue = 0;;


void TimerRoutine()
{
	TIM1 -> CCR4 = dataToPlay;
	PWM_repetition++;

	if(PWM_repetition >= REPETITIONS)
	{
		dataToPlay = BufferTake();
		PWM_repetition = 0;
	}
}

void selectFileToPlay(enum file fileToPlay)
{
	switch(fileToPlay)
	{
		case SHOOT:
			if(filePlayed == SHOOT) break;
			PlayFile(SHOOT);
			break;
		case CLICK:
			if(filePlayed == SHOOT || filePlayed == CLICK) break;
			PlayFile(CLICK);
			break;
		case BOOT:
			PlayFile(BOOT);
			break;
		default:
			//BLAD nie powinno tak byc
			while(1) LL_GPIO_SetOutputPin(LED_GPIO_Port, LED_Pin);
			break;
	}
}

void goIdle()
{
	LL_TIM_DisableIT_UPDATE(TIM1);
	TIM1 -> CCR4 = 0x55;
	for(uint32_t delay = 0; delay < 120000; delay++);
	LL_TIM_DisableCounter(TIM1);
	LL_GPIO_ResetOutputPin(NSD_GPIO_Port, NSD_Pin);
	currentAddr = 0;
	filePlayed = NONE;
	//ClearBuffers();
}

void NewMain()
{
	if(currentAddr >= BOOTAddr + BOOTLength)
	{
		goIdle();

		LL_EXTI_ClearFallingFlag_0_31(LL_EXTI_LINE_8);
		LL_EXTI_ClearFallingFlag_0_31(LL_EXTI_LINE_11);

		LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_8);
		LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_11);

		BlinkLED(3);
	}
	if((currentAddr - currentFileAddr) >= currentFileLength)
	{
		goIdle();
		if(!LL_GPIO_IsInputPinSet(Trigger_GPIO_Port, Trigger_Pin)) selectFileToPlay(SHOOT);
	}

	if(filePlayed != NONE)
	{
	uint16_t size;

	if(IsAnyBufferEmpty())
		{
			GetEmptyBuffer();

			LL_SPI_Enable(SPI1);
			if(currentFileLength - (currentAddr - currentFileAddr) >= BUFFER_SIZE) size = BUFFER_SIZE;
			else size = currentFileLength + currentFileAddr - currentAddr;
			FAST_READ_IT(currentAddr,size);
		}
	}
}




