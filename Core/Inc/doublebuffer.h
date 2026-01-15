/*
 * doublebuffer.h
 *
 *  Created on: Jan 15, 2026
 *      Author: 1
 */

#ifndef INC_DOUBLEBUFFER_H_
#define INC_DOUBLEBUFFER_H_

#include "main.h"

#define BUFFER_SIZE 1024

typedef enum{
	EMPTY = 0,
	READY
} bufferStatus_t;

typedef struct{
	uint8_t data[BUFFER_SIZE];
	volatile uint16_t length;
	volatile uint16_t index;
	volatile bufferStatus_t status;
} buffer_t;

buffer_t* GetEmptyBuffer();
void BufferAppend(uint8_t data, buffer_t *buffer);
void BufferCommit(buffer_t *buffer);

buffer_t* GetReadyBuffer();
uint8_t BufferTake(buffer_t *buffer);



#endif /* INC_DOUBLEBUFFER_H_ */
