 /*
 * spi_flash.c
 *
 *  Created on: Oct 18, 2025
 *      Author: 1
 */
#include "spi_flash.h"
#include "main.h"
#include "system.h"

#define Sector_Size 4096

//CS Pin define
//#define CS_GPIO_Port  //Port on which CS is
//#define CS_Pin //Pin on which CS is

//CS macros
#define CS_LOW() LL_GPIO_ResetOutputPin(NCS_GPIO_Port,NCS_Pin)
#define CS_HIGH() LL_GPIO_SetOutputPin(NCS_GPIO_Port,NCS_Pin)

//
#define Macronix_ID 0xC2
#define Memory_Type 0x23
#define Memory_Capacity 0x14

//define SPI1 peripheral used
#define FLASH_SPI SPI1

//Warstwa sprzetowa
//SPI musi byc wlaczone

//metoda wysyla bajty nic nie zwraca
void SPI_Transmit_Data8(SPI_TypeDef *SPIx,uint8_t *data,uint16_t size)
{
	//wyslij dana ilosc bajtow
	for(uint16_t i = 0; i < size ; i++)
	{

	while(!LL_SPI_IsActiveFlag_TXE(SPIx));
	LL_SPI_TransmitData8(SPIx, data[i]);

	while(!LL_SPI_IsActiveFlag_RXNE(SPIx));
	(void)LL_SPI_ReceiveData8(SPIx);
	}

	//zaczekaj na koniec pracy
	while(LL_SPI_IsActiveFlag_BSY(SPIx));
}

//Metoda wysyla i odbiera bajty
void SPI_TransmitReceive_Data8(SPI_TypeDef *SPIx,uint8_t *tx, uint8_t *rx, uint16_t size)
{
	//wyslij dobierz dane
	for(uint16_t i = 0; i < size; i++)
	{
	while(!LL_SPI_IsActiveFlag_TXE(SPIx));
	LL_SPI_TransmitData8(SPIx,tx[i]);

	while(!LL_SPI_IsActiveFlag_RXNE(SPIx));
	rx[i] = LL_SPI_ReceiveData8(SPIx);
	}

	//zaczekaj na koniec pracy
	while(LL_SPI_IsActiveFlag_BSY(SPIx));
}

void SPI_Receive_Data8(SPI_TypeDef *SPIx,uint8_t *rx,uint16_t size)
{
	for(uint16_t i =0; i <size; i++)
	{
		while(!LL_SPI_IsActiveFlag_TXE(SPIx));
		LL_SPI_TransmitData8(SPIx,0x00); //dummy bajt

		while(!LL_SPI_IsActiveFlag_RXNE(SPIx));
		rx[i] = LL_SPI_ReceiveData8(SPIx);
	}
}


//Warstwa abstrakcji

//Metoda przyjmuje i zwraca strukture ID układu
void Read_Identification(struct memory_info *info)
{
	uint8_t DataToSend = RDID;
	uint8_t ReceiveData[3] = {0,0,0};

	CS_LOW();

	SPI_Transmit_Data8(FLASH_SPI,&DataToSend,1);
	SPI_Receive_Data8(FLASH_SPI,ReceiveData,3);

	info -> manufacturer_ID = ReceiveData[0];
	info -> memory_type = ReceiveData[1];
	info -> capacity = ReceiveData[2];

	while(LL_SPI_IsActiveFlag_BSY(FLASH_SPI));

	CS_HIGH();
}

//Metoda sprawdza zgodnosc ID i opcjonalnie wykonuje rutyne bledu sukcesu
void CheckIC()
{
	struct memory_info MX25V_Info;

	Read_Identification(&MX25V_Info);

	if(!(Macronix_ID == MX25V_Info.manufacturer_ID &&
		 Memory_Type == MX25V_Info.memory_type &&
		 Memory_Capacity == MX25V_Info.capacity))
	{
		//error handler
	}
	//do nothing or confirm/return1
}


void Write_Enable()
{
	uint8_t DataToSend = WREN;

	CS_LOW();

	SPI_Transmit_Data8(FLASH_SPI, &DataToSend,1);

	while(LL_SPI_IsActiveFlag_BSY(FLASH_SPI));
	CS_HIGH();

	while(!(Read_Status() & 0x02));

}


void Write_Disable()
{
	uint8_t DataToSend = WRDI;
	CS_LOW();

	SPI_Transmit_Data8(FLASH_SPI, &DataToSend,1);

	while(LL_SPI_IsActiveFlag_BSY(FLASH_SPI));
	CS_HIGH();

	while(Read_Status() & 0x02);
}

//Metoda zwraca zawartosc rejestru statusu
uint8_t Read_Status()
{
	uint8_t DataToSend = RDSR;
	uint8_t Status = 0;

	CS_LOW();

	SPI_Transmit_Data8(FLASH_SPI,&DataToSend,1);
	SPI_Receive_Data8(FLASH_SPI,&Status,1);

	while(LL_SPI_IsActiveFlag_BSY(FLASH_SPI));
	CS_HIGH();

	return Status;
}

//Metoda czeka az bit WIP (Write in Progress) się wyzeruje i pozwoli na wpisanie kolejnych danych
void Wait_While_Busy(void)
{
    uint8_t DataToSend = RDSR;
    uint8_t status = 0;

    CS_LOW();

    SPI_Transmit_Data8(FLASH_SPI,&DataToSend,1);

    do {
    	SPI_Receive_Data8(FLASH_SPI,&status,1);
    } while(status & 0x01);

    while (LL_SPI_IsActiveFlag_BSY(FLASH_SPI));
    CS_HIGH();
}

void Reset()
{
	uint8_t Data1 = RSTEN;
	uint8_t Data2 = RST;

	CS_LOW();
	SPI_Transmit_Data8(FLASH_SPI,&Data1,1);

	while(LL_SPI_IsActiveFlag_BSY(FLASH_SPI));
	CS_HIGH();

	CS_LOW();
	SPI_Transmit_Data8(FLASH_SPI,&Data2,1);

	while(LL_SPI_IsActiveFlag_BSY(FLASH_SPI));
	CS_HIGH();
}

void Read(uint32_t StartPage, uint8_t Offset, uint32_t size, uint8_t *Received_Data)
{
	uint8_t tData[4];
	uint32_t Memory_Address = (StartPage*256) + Offset;

	tData[0] = READ;
	tData[1] = (Memory_Address>>16)&0xFF;
	tData[2] = (Memory_Address>>8)&0xFF;
	tData[3] = (Memory_Address)&0xFF;

	CS_LOW();

	SPI_Transmit_Data8(FLASH_SPI,tData,4);
	SPI_Receive_Data8(FLASH_SPI,Received_Data,size);

	while(LL_SPI_IsActiveFlag_BSY(FLASH_SPI));
	CS_HIGH();
}

void Fast_Read(uint32_t StartPage, uint8_t Offset, uint32_t size, uint8_t *Received_Data)
{
	uint8_t tData[5];
	uint32_t Memory_Address = (StartPage*256) + Offset;

	tData[0] = FAST_READ;
	tData[1] = (Memory_Address>>16)&0xFF;
	tData[2] = (Memory_Address>>8)&0xFF;
	tData[3] = (Memory_Address)&0xFF;
	tData[4] = 0x00; //dummy byte

	CS_LOW();

	SPI_Transmit_Data8(FLASH_SPI,tData,5);
	SPI_Receive_Data8(FLASH_SPI,Received_Data,size);

	while(LL_SPI_IsActiveFlag_BSY(FLASH_SPI));
	CS_HIGH();
}

void Sector_Erase(uint16_t numSector)
{
	uint8_t tData[4];
	uint32_t Memory_Address = numSector*16*256;

	tData[0] = SE;
	tData[1] = (Memory_Address >>16)&0xFF;
    tData[2] = (Memory_Address >>8)&0xFF;
	tData[3] = (Memory_Address)&0xFF;

	Write_Enable();

	CS_LOW();
	SPI_Transmit_Data8(FLASH_SPI,tData,4);

	while(LL_SPI_IsActiveFlag_BSY(FLASH_SPI));
	CS_HIGH();

	Wait_While_Busy();

	Write_Disable();

}

uint32_t BytesToWrite(uint32_t size,uint16_t offset)
{
	if ((size+offset)<256) return size;
	else return 256-offset;
}

uint32_t BytesToModify(uint32_t size,uint16_t offset)
{
	if ((size+offset)<4096) return size;
	else return 4096 - offset;
}

void Write_Clean(uint32_t page, uint16_t offset, uint32_t size,uint8_t *data)
{
	uint8_t tData[266];
	uint32_t startPage = page;
	uint32_t endPage = startPage + ((size+offset-1)/256);
	uint32_t numPages = endPage-startPage+1;

	uint16_t startSector = startPage/16;
	uint16_t endSector = endPage/16;
	uint16_t numSectors = endSector-startSector+1;
	for(uint16_t i = 0; i < numSectors; i++)
	{
		Sector_Erase(startSector++);
	}

	uint32_t dataPosition = 0;

	// write the data
	for (uint32_t i =0; i < numPages; i++)
	{
		uint32_t memAddress = (startPage*256) + offset;
		uint16_t bytesremaining = BytesToWrite(size,offset);
		uint32_t index = 0;

		Write_Enable();

		tData[0] = PP;
		tData[1] = (memAddress >> 16)&0xFF;
		tData[2] = (memAddress >> 8)&0xFF;
		tData[3] = (memAddress)&0xFF;
		index = 4;

		uint16_t bytestosend = bytesremaining + index;

		for (uint16_t i = 0; i< bytesremaining; i++)
		{
			tData[index++] = data[i+dataPosition];
		}

		if (bytestosend > 250)
		{
			CS_LOW();

			SPI_Transmit_Data8(FLASH_SPI,tData,100);
			SPI_Transmit_Data8(FLASH_SPI,tData+100,bytestosend-100);

			while(LL_SPI_IsActiveFlag_BSY(FLASH_SPI));
			CS_HIGH();
		}
		else
		{
			CS_LOW();

			SPI_Transmit_Data8(FLASH_SPI,tData,bytestosend);

			while(LL_SPI_IsActiveFlag_BSY(FLASH_SPI));
			CS_HIGH();
		}

		startPage++;
		offset= 0;
		size = size-bytesremaining;
		dataPosition = dataPosition+bytesremaining;

		Wait_While_Busy();

		Write_Disable();
	}
}

void Write(uint32_t page,uint16_t offset,uint32_t size,uint8_t *data)
{
	uint16_t startSector = page/16;
	uint16_t endSector = (page + ((size+offset-1)/256))/16;
	uint16_t numSectors = endSector-startSector+1;

	uint8_t previousData[4096];
	uint32_t sectorOffset = ((page%16)*256)+offset;
	uint32_t dataindex = 0;

	for (uint16_t i = 0; i<numSectors; i++)
	{
		uint32_t startPage = startSector*16;
		Fast_Read(startPage,0,4096,previousData);

		uint32_t bytesRemaining = BytesToModify(size,sectorOffset);

		for(uint16_t i =0; i < bytesRemaining; i++)
		{
			previousData[i+sectorOffset] = data[i +dataindex];
		}

		Write_Clean(startPage,0,4096,previousData);

		startSector++;
		sectorOffset = 0;
		dataindex = dataindex + bytesRemaining;
		size = size-bytesRemaining;
	}

}

uint8_t Read_Byte(uint32_t Addr)
{
	uint8_t tData[4];
	uint8_t rData;

	tData[0] = READ;
	tData[1] = (Addr>>16)&0xFF;
	tData[2] = (Addr>>8)&0xFF;
	tData[3] = (Addr)&0xFF;

	CS_LOW();

	SPI_Transmit_Data8(SPI1,tData,4);

	SPI_Receive_Data8(SPI1,&rData,1);

	while(LL_SPI_IsActiveFlag_BSY(SPI1));
	CS_HIGH();

	return rData;
}

void Write_Byte (uint32_t Addr, uint8_t data)
{
	uint8_t tData[5];
	uint8_t index;

	tData[0] = PP;
	tData[1] = (Addr>>16)&0xFF;
	tData[2] = (Addr>>8)&0xFF;
	tData[3] = (Addr)&0xFF;
	tData[4] = data;
	index = 5;

	if(Read_Byte(Addr) == 0xFF)
	{
		Write_Enable();

		CS_LOW();
		SPI_Transmit_Data8(FLASH_SPI,tData,index);
		while(LL_SPI_IsActiveFlag_BSY(FLASH_SPI));
		CS_HIGH();


		Wait_While_Busy();
		Write_Disable();

	}
}

//temp bajty do konwersji

uint8_t TempBytes[2];

//konwersja uint16 na dwa bajty
void Uint16ToBytes(uint8_t *bytes_temp, uint16_t value)
{
	//zapis w uni
	union {
		uint16_t val;
		uint8_t byte[2];
	} u;

	u.val = value;
	//zwrot bajtow
	bytes_temp[0] = u.byte[0];//LSB
	bytes_temp[1] = u.byte[1];//MSB
}


//konwersja dwoch bajtow na uint16_t
uint16_t BytesToUint16(uint8_t *bytes_temp)
{
	//zapis w unii
	union{
		uint16_t val;
		uint8_t bytes[2];
	}u;

	u.bytes[0] = bytes_temp[0];
	u.bytes[1] = bytes_temp[1];

	//zwrot wartosci
	return u.val;
}

void Write_Uint16t (uint32_t page,uint16_t offset, uint16_t data)
{
	Uint16ToBytes(TempBytes,data);

	//wpisz wartosc jako jeden nie czyszczac strony
	uint32_t Address = (page*256) +offset;

	Write_Byte(Address, TempBytes[0]);
	Write_Byte(Address+1,TempBytes[1]);
}

uint16_t Read_Uint16t (uint32_t page, uint16_t offset)
{
	uint8_t rData[2];
	Read(page,offset,2,rData);
	return BytesToUint16(rData);
}

void Chip_Erase()
{
	uint8_t Data1 = CE;

	Write_Enable();

	CS_LOW();
	SPI_Transmit_Data8(FLASH_SPI,&Data1,1);
	while(LL_SPI_IsActiveFlag_BSY(FLASH_SPI));
	CS_HIGH();

	Wait_While_Busy();
}

typedef struct {
	uint8_t* data_ptr;
	uint32_t count;
}buffer_t;

static buffer_t tx_buffer = {0};
static buffer_t rx_buffer = {0};

void SPI_Transmit_Data_IT(uint8_t *data, uint16_t size)
{
	tx_buffer.data_ptr = data;
	tx_buffer.count = size;

	LL_SPI_EnableIT_TXE(SPI1);
	LL_SPI_Enable(SPI1);
}

void SPI_Read_Data_IT(uint8_t *data, uint32_t size)
{
	tx_buffer.count = size;

	rx_buffer.data_ptr = data;
	rx_buffer.count = size;

	LL_SPI_EnableIT_TXE(SPI1);
	LL_SPI_EnableIT_RXNE(SPI1);
	LL_SPI_Enable(SPI1);
}


#define DUMMY_BYTE 0x00
void spi_it_transmit_callback(void)
{
	if (tx_buffer.count > 0)
	{
		LL_SPI_TransmitData8(SPI1, *tx_buffer.data_ptr);

		tx_buffer.data_ptr++;
		tx_buffer.count--;
	}
	else
	{
		LL_SPI_TransmitData8(SPI1, DUMMY_BYTE);
		tx_buffer.count--;
	}

	if(tx_buffer.count <= 0 && !LL_SPI_IsEnabledIT_RXNE(SPI1))
	{
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

extern circularBuffer buffer;

void spi_it_receive_callback()
{
	if(rx_buffer.count > 0)
	{
		//*rx_buffer.data_ptr = LL_SPI_ReceiveData8(SPI1);
		bufferAppend(&buffer, LL_SPI_ReceiveData8(SPI1));
		//rx_buffer.data_ptr++;
		rx_buffer.count--;
	}

	if(rx_buffer.count <= 0)
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
	if(LL_SPI_IsActiveFlag_TXE(SPI1) && LL_SPI_IsEnabledIT_TXE(SPI1))
	{
		spi_it_transmit_callback();
	}

	if(LL_SPI_IsActiveFlag_RXNE(SPI1) && LL_SPI_IsEnabledIT_RXNE(SPI1))
	{
		spi_it_receive_callback();
	}
}


void FAST_READ_IT(uint32_t address, uint8_t *data, uint16_t size)
{
	uint8_t tData[5] = {0};

	tData[0] = FAST_READ;
	tData[1] = (address>>16)&0xFF;
	tData[2] = (address>>8)&0xFF;
	tData[3] = (address)&0xFF;
	tData[4] = 0x00; //dummy byte

	CS_LOW();

	SPI_Transmit_Data8(FLASH_SPI,tData,5);
	LL_SPI_Disable(SPI1);
	SPI_Read_Data_IT(data, size);
}








