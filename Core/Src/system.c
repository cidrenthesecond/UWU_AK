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


//===================================================================
//HARDWARE CONFIGURATION
//=======================================================

static void DisableButtonInterrupts()
{
	LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_8);
	LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_11);
}

static void EnableButtonInterrupts()
{
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_8);
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_11);

	LL_EXTI_ClearFallingFlag_0_31(LL_EXTI_LINE_8);
	LL_EXTI_ClearFallingFlag_0_31(LL_EXTI_LINE_11);
}

static void InitializeGPIO()
{
	LL_GPIO_ResetOutputPin(LED_GPIO_Port, LED_Pin); //Turn off LED
	LL_GPIO_ResetOutputPin(NWP_GPIO_Port, NWP_Pin); //FLASH WriteProtect ON
	LL_GPIO_SetOutputPin(NSD_GPIO_Port,NSD_Pin); //Turn off amplifier
	LL_GPIO_SetOutputPin(NCS_GPIO_Port,NCS_Pin); //unselect FLASH
}

static void InitializePWM()
{
	TIM1->CCR1 = PWM_STARTUP_VAL; //LSB
	TIM1->CCR4 = PWM_STARTUP_VAL; //MSB
	LL_TIM_ClearFlag_UPDATE(TIM1);
	LL_TIM_EnableAllOutputs(TIM1);
	LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH4);
	LL_TIM_EnableIT_UPDATE(TIM1);
}

static void InitializeSPI()
{
    LL_SPI_Enable(SPI1);
    (void)LL_SPI_ReceiveData8(SPI1); // Clear receive buffer
}

//===============================================================
//FILE LOADING FUNCTIONS
//===============================================================

static void LoadFileMetadata(enum file fileToPlay)
{
    switch (fileToPlay)
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
            // Error: invalid file type
            break;
    }
}

static void ReadDataToBuffer()
{
    if (!IsAnyBufferEmpty()) return;

    GetEmptyBuffer();
    LL_SPI_Enable(SPI1);

    uint32_t remainingBytes = (currentFileLength + currentFileAddr) - currentAddr;
    uint16_t bytesToRead = (remainingBytes >= BUFFER_SIZE) ? BUFFER_SIZE : (uint16_t)remainingBytes;

    FAST_READ_IT(currentAddr, bytesToRead);
    currentAddr += bytesToRead;
}

//============================================================
//PLAYBACK CONTROL
//============================================================
void selectFileToPlay(enum file fileToPlay)
{
	if(filePlayed != NONE) return;

	switch(fileToPlay)
		{
			case SHOOT:
			case CLICK:
			case BOOT:
				PlayFile(fileToPlay);
				break;
			default:
				//BLAD nie powinno tak byc
				break;
		}
}


void PlayFile(enum file fileToPlay)
{
	LoadFileMetadata(fileToPlay);

	DisableButtonInterrupts();

	GetEmptyBuffer();

	LL_SPI_Enable(SPI1);
	FAST_READ_IT(currentAddr,BUFFER_SIZE);
	currentAddr = currentAddr + BUFFER_SIZE;

	while(!IsAnyBufferReady());
	GetReadyBuffer();

	LL_GPIO_SetOutputPin(NSD_GPIO_Port, NSD_Pin); // wlacz wzmacniacz
	for(uint16_t time =0 ; time < 20000; time++);

	TIM1 -> CCR4 = 0x88;
	LL_TIM_EnableCounter(TIM1); //wlacz timer
	for(uint16_t time =0 ; time < 20000; time++);

	LL_TIM_EnableIT_UPDATE(TIM1);
}

void goIdle()
{
	LL_TIM_DisableIT_UPDATE(TIM1);

	TIM1 -> CCR4 = PWM_STARTUP_VAL;
	for(uint16_t delay = 0; delay < 20000; delay++);

	LL_TIM_DisableCounter(TIM1);
	LL_GPIO_ResetOutputPin(NSD_GPIO_Port, NSD_Pin);

	currentAddr = 0;
	filePlayed = NONE;

	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_8);
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_11);

	LL_EXTI_ClearFallingFlag_0_31(LL_EXTI_LINE_8);
	LL_EXTI_ClearFallingFlag_0_31(LL_EXTI_LINE_11);
}

//========================================================
//UTILITY
//=====================================================

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

// ==============================================================================
// SYSTEM INITIALIZATION
// ==============================================================================

void SystemBoot()
{
	DisableButtonInterrupts();

	InitializeGPIO();
	InitializePWM();
	InitializeSPI();	//SPI ON

	selectFileToPlay(BOOT);

    while ((currentAddr < BOOTAddr + BOOTLength) && !AreAllBuffersEmpty())
    {
        ReadDataToBuffer();
    }

	goIdle();

	DisableButtonInterrupts();
	BlinkLED(3);
	EnableButtonInterrupts();
}



// ==============================================================================
// INTERRUPT HANDLERS
// ==============================================================================

volatile uint8_t PWM_repetition = 0;
volatile uint8_t dataToPlay = 0;

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

// ==============================================================================
// MAIN LOOP
// ==============================================================================

void NewMain()
{
	if((currentAddr >= currentFileLength + currentFileAddr))
	{
		goIdle();
		if(!LL_GPIO_IsInputPinSet(Trigger_GPIO_Port, Trigger_Pin)) selectFileToPlay(SHOOT);
	}

	if(filePlayed != NONE)
	{
		ReadDataToBuffer();
	}
}




