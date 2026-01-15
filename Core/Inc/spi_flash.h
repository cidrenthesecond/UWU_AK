/*
 * spi_flash.h
 *
 *  Created on: Oct 17, 2025
 *      Author: 1
 */

#ifndef INC_SPI_FLASH_H_
#define INC_SPI_FLASH_H_

#include "main.h"


#endif /* INC_SPI_FLASH_H_ */

//REGISTERS define
#define WREN 0x06 //Write enable
#define WRDI 0x04 //Write disable
#define RDID 0x9F //Read identificaiton
#define RDSR 0x05 //Read status registert
#define RDCR 0x15 //Read configuration register
#define WRSR 0x01 //Write status register
#define READ 0x03 //READ data bytes
#define FAST_READ 0x0B // FAST READ
#define BURST 0xC0 // BURST read
#define SE 0x20 // Sector erase
#define BE32K 0x52 // block erase
#define BE 0xD8 // block erase
#define CE 0x60 //Chip erase
#define PP 0x02 //Page program
#define DP 0xB9 //deep power-down
#define RSTEN 0x66 //Reset enable
#define RST 0x99 //Reset


struct memory_info
{
	uint8_t manufacturer_ID;
	uint8_t memory_type;
	uint8_t capacity;
};

/*** Low-level SPI layer ***/
void SPI_Transmit_Data8(SPI_TypeDef *SPIx, uint8_t *data, uint16_t size);
void SPI_TransmitReceive_Data8(SPI_TypeDef *SPIx, uint8_t *tx, uint8_t *rx, uint16_t size);
void SPI_Receive_Data8(SPI_TypeDef *SPIx, uint8_t *rx, uint16_t size);

/*** Flash Command Abstraction Layer ***/
void Read_Identification(struct memory_info *info);
void CheckIC(void);
void Write_Enable(void);
void Write_Disable(void);
uint8_t Read_Status(void);
void Wait_While_Busy(void);
void Reset(void);

/*** Memory Operations ***/
void Read(uint32_t StartPage, uint8_t Offset, uint32_t size, uint8_t *Received_Data);
void Fast_Read(uint32_t StartPage, uint8_t Offset, uint32_t size, uint8_t *Received_Data);
void Sector_Erase(uint16_t numSector);
void Write_Clean(uint32_t page, uint16_t offset, uint32_t size, uint8_t *data);
void Write(uint32_t page, uint16_t offset, uint32_t size, uint8_t *data);
uint8_t Read_Byte(uint32_t Addr);
void Write_Byte(uint32_t Addr, uint8_t data);
void Chip_Erase(void);

/*** Data Conversion Helpers ***/
void Uint16ToBytes(uint8_t *bytes_temp, uint16_t value);
uint16_t BytesToUint16(uint8_t *bytes_temp);
void Write_Uint16t(uint32_t page, uint16_t offset, uint16_t data);
uint16_t Read_Uint16t(uint32_t page, uint16_t offset);

/*** Utility Functions ***/
uint32_t BytesToWrite(uint32_t size, uint16_t offset);
uint32_t BytesToModify(uint32_t size, uint16_t offset);

//interrupt test
void spi_it_transmit_callback(void);
void spi_it_receive_callback();
void FAST_READ_IT(uint32_t address, uint8_t *data, uint16_t size);
