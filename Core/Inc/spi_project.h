/*
 * spi_project.h
 *
 *  Created on: Jan 15, 2026
 *      Author: 1
 */

#ifndef INC_SPI_PROJECT_H_
#define INC_SPI_PROJECT_H_

void SPI_Read_Data_IT(uint32_t size);
void spi_it_transmit_callback(void);
void spi_it_receive_callback();
void FAST_READ_IT(uint32_t address,uint16_t size);


#endif /* INC_SPI_PROJECT_H_ */
