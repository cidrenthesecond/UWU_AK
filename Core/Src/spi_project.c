/*
 * spi_project.c
 *
 *  Created on: Jan 15, 2026
 *      Author: 1
 */
#include "main.h"
#include "spi_flash.h"
#include "doublebuffer.h"

static uint32_t rxLeft;

void SPI_Read_Data_IT(uint32_t size)
{
	rxLeft = size;

	LL_SPI_EnableIT_TXE(SPI1);
	LL_SPI_EnableIT_RXNE(SPI1);
	LL_SPI_Enable(SPI1);
}

void spi_it_receive_callback()
{
	if(rxLeft > 0)
	{
		BufferAppend(LL_SPI_ReceiveData8(SPI1), GetEmptyBuffer() );
		rxLeft--;
	}

	if(rxLeft <= 0)
	{
		LL_SPI_DisableIT_RXNE(SPI1);
		LL_SPI_DisableIT_TXE(SPI1);

		while(LL_SPI_GetTxFIFOLevel(SPI1) != LL_SPI_TX_FIFO_EMPTY);

		while(LL_SPI_IsActiveFlag_BSY(SPI1));

		LL_SPI_Disable(SPI1);

		while(LL_SPI_GetRxFIFOLevel(SPI1) != LL_SPI_RX_FIFO_EMPTY)
		{ (void) LL_SPI_ReceiveData8(SPI1);}

		LL_SPI_ClearFlag_OVR(SPI1);

		CS_HIGH();
	}
}

void SPI1_IRQHANDLER(void)
{
	if(LL_SPI_IsActiveFlag_RXNE(SPI1) && LL_SPI_IsEnabledIT_RXNE(SPI1))
	{
		spi_it_receive_callback();
	}
}


void FAST_READ_IT(uint32_t address,uint16_t size)
{
	uint8_t tData[5] = {0};

	tData[0] = FAST_READ;
	tData[1] = (address>>16)&0xFF;
	tData[2] = (address>>8)&0xFF;
	tData[3] = (address)&0xFF;
	tData[4] = 0x00; //dummy byte

	CS_LOW();

	SPI_Transmit_Data8(SPI1,tData,5);
	LL_SPI_Disable(SPI1);
	SPI_Read_Data_IT(size);
}



