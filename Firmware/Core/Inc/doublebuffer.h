/*
 * doublebuffer.h
 *
 *  Created on: Jan 15, 2026
 *      Author: 1
 */

#ifndef INC_DOUBLEBUFFER_H_
#define INC_DOUBLEBUFFER_H_

#include "main.h"

#define BUFFER_SIZE 1500 //1024

typedef enum{
	EMPTY = 0,
	USED,
	READY
} bufferStatus_t;

typedef struct{
	uint8_t data[BUFFER_SIZE];
	volatile uint16_t length;
	volatile uint16_t index;
	volatile bufferStatus_t status;
} buffer_t;

uint8_t IsAnyBufferEmpty();

void GetEmptyBuffer();
void BufferAppend(uint8_t data);
void BufferCommit();

uint8_t IsAnyBufferReady();
void GetReadyBuffer();
uint8_t BufferTake();
void ClearBuffers();

uint8_t AreAllBuffersEmpty();


#endif /* INC_DOUBLEBUFFER_H_ */
