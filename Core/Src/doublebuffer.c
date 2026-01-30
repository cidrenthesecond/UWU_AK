/*
 * doublebuffer.c
 *
 *  Created on: Jan 15, 2026
 *      Author: 1
 */
#include "doublebuffer.h"
#include "main.h"
#include "string.h"

// buffer definitions
static buffer_t buffer1 = {0};
static buffer_t buffer2 = {0};


// variables holding pointer to used buffer
static buffer_t *currentWriteBuffer;
static buffer_t *currentReadBuffer;

//returns if any buffer is empty and ready to fill
uint8_t IsAnyBufferEmpty()
{
	if(buffer1.status == EMPTY || buffer2.status == EMPTY) return 0x01;
	else return 0x00;
}

//return pointer to buffer to fill
void GetEmptyBuffer()
{
	if(buffer1.status == EMPTY)
	{
		currentWriteBuffer = &buffer1;
		buffer1.status = USED;
	}
	else if(buffer2.status == EMPTY)
	{
		currentWriteBuffer = &buffer2;
		buffer2.status = USED;
	}
}

//load data into buffer
void BufferAppend(uint8_t data)
{
	//if(buffer -> status == FULL) return; //full buffer protection
	if(currentWriteBuffer -> length < BUFFER_SIZE)
		currentWriteBuffer ->data[currentWriteBuffer->length++] = data;
}

//commits buffer to read
void BufferCommit()
{
	if(currentWriteBuffer -> length > 0)
	{
		currentWriteBuffer -> index = 0;
		currentWriteBuffer -> status = READY;

	}
}

//checks if any buffer is ready to read
uint8_t IsAnyBufferReady()
{
	if(buffer1.status == READY || buffer2.status == READY) return 0x01;
	else return 0x00;
}

//sets pointer to ready buffer
void GetReadyBuffer()
{
	if(buffer1.status == READY)
	{
		currentReadBuffer = &buffer1;
		buffer1.status = USED;
	}
	else if(buffer2.status == READY)
	{
		currentReadBuffer = &buffer2;
		buffer2.status = USED;
	}
}

//gives data from buffer and loops
uint8_t BufferTake()
{
	uint8_t val = currentReadBuffer -> data[currentReadBuffer-> index++];

	if((currentReadBuffer -> index) >= (currentReadBuffer -> length))
	{
		currentReadBuffer -> length = 0;
		currentReadBuffer -> status = EMPTY;
		if(IsAnyBufferReady())
		{
			GetReadyBuffer();
		}
		else
		{
			// No buffer ready - return silence or last value
			// This prevents reading from exhausted buffer
			currentReadBuffer -> index--; // Stay at last position
		}
	}

	return val;
}

//clears buffers
void ClearBuffers()
{
	buffer1.length = 0;
	buffer1.index  = 0;
	buffer1.status = EMPTY;
	buffer2.length = 0;
	buffer2.index = 0;
	buffer2.status = EMPTY;

	currentWriteBuffer = NULL;
	currentReadBuffer  = NULL;
}

uint8_t AreAllBuffersEmpty()
{
	if(buffer1.status == EMPTY && buffer2.status == EMPTY) return 1;
	else return 0;
}
