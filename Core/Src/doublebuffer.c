/*
 * doublebuffer.c
 *
 *  Created on: Jan 15, 2026
 *      Author: 1
 */
#include "doublebuffer.h"
#include "main.h"

static buffer_t buffer1 = {0};
static buffer_t buffer2 = {0};

buffer_t* GetEmptyBuffer()
{
	if(buffer1.status == EMPTY) return &buffer1;
	if(buffer2.status == EMPTY) return &buffer2;

	return 0x00;//blad
}

void BufferAppend(uint8_t data, buffer_t *buffer)
{
	//if(buffer -> status == FULL) return; //full buffer protection
	if(buffer -> length < BUFFER_SIZE)
		buffer->data[buffer->length++] = data;
}

void BufferCommit(buffer_t *buffer)
{
	if(buffer -> length > 0)
	{
		buffer -> index = 0;
		buffer -> status = READY;
	}
}

buffer_t* GetReadyBuffer()
{
	if(buffer1.status == READY) return &buffer1;
	if(buffer2.status == READY) return &buffer2;
	return 0x00; //blad narazie aby kompilator zamknal morde!!!
}

uint8_t BufferTake(buffer_t *buffer)
{
	//if(buffer -> status == EMPTY) return 0; //blad
	uint8_t val = buffer -> data[buffer-> index++];

	if((buffer -> index) >= (buffer -> length))
	{
		buffer -> length = 0;
		buffer -> status = EMPTY;
	}

	return val;
}
