/*
 * system.h
 *
 *  Created on: Jan 8, 2026
 *      Author: 1
 */

#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

#define PWM_STARTUP_VAL 0x80
#define REPETITIONS 4

#define BOOTAddr 0xB20C
#define BOOTLength 48094
#define CLICKAddr 0x6732
#define CLICKLength 19162
#define SHOOTAddr 0x0000
#define SHOOTLength 26418

struct fileData{
	uint32_t startAddr;
	uint32_t length;
};

typedef struct
{
	uint8_t data[4000];
	volatile uint16_t head;
	volatile uint16_t tail;
} circularBuffer;

void bufferAppend(circularBuffer* buffer, uint8_t data);


enum file{
	NONE = 0,
	BOOT,
	CLICK,
	SHOOT
};

void SystemBoot();
void BlinkLED(uint8_t blinks);
void PlayFile(enum file fileToPlay);
void TimerRoutine();
void selectFileToPlay(enum file fileToPlay);
void mainSM();
void goIdle();

void buffercheck();




#endif /* INC_SYSTEM_H_ */
