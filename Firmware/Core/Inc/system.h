/*
 * system.h
 *
 *  Created on: Jan 8, 2026
 *      Author: 1
 */

#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

#define PWM_STARTUP_VAL 0x80
#define REPETITIONS 10


#define BOOTAddr 23098
#define BOOTLength 22518
#define CLICKAddr 11998
#define CLICKLength 11100
#define SHOOTAddr 0x0000
#define SHOOTLength 11998

struct fileData{
	uint32_t startAddr;
	uint32_t length;
};

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

void NewMain();

#endif /* INC_SYSTEM_H_ */
