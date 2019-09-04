/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

#include <ctype.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "mt3620-baremetal.h"
#include "mt3620-intercore.h"
#include "mt3620-uart-poll.h"

#include "softwareserial.h"

extern uint32_t StackTop; // &StackTop == end of TCM0

static _Noreturn void DefaultExceptionHandler(void);

static _Noreturn void RTCoreMain(void);

// ARM DDI0403E.d SB1.5.2-3
// From SB1.5.3, "The Vector table must be naturally aligned to a power of two whose alignment
// value is greater than or equal to (Number of Exceptions supported x 4), with a minimum alignment
// of 128 bytes.". The array is aligned in linker.ld, using the dedicated section ".vector_table".

// The exception vector table contains a stack pointer, 15 exception handlers, and an entry for
// each interrupt.
#define INTERRUPT_COUNT 100 // from datasheet
#define EXCEPTION_COUNT (16 + INTERRUPT_COUNT)
#define INT_TO_EXC(i_) (16 + (i_))
const uintptr_t ExceptionVectorTable[EXCEPTION_COUNT] __attribute__((section(".vector_table")))
__attribute__((used)) = {
	[0] = (uintptr_t)& StackTop,                // Main Stack Pointer (MSP)
	[1] = (uintptr_t)RTCoreMain,               // Reset
	[2] = (uintptr_t)DefaultExceptionHandler,  // NMI
	[3] = (uintptr_t)DefaultExceptionHandler,  // HardFault
	[4] = (uintptr_t)DefaultExceptionHandler,  // MPU Fault
	[5] = (uintptr_t)DefaultExceptionHandler,  // Bus Fault
	[6] = (uintptr_t)DefaultExceptionHandler,  // Usage Fault
	[11] = (uintptr_t)DefaultExceptionHandler, // SVCall
	[12] = (uintptr_t)DefaultExceptionHandler, // Debug monitor
	[14] = (uintptr_t)DefaultExceptionHandler, // PendSV
	[15] = (uintptr_t)DefaultExceptionHandler, // SysTick

	[INT_TO_EXC(0)... INT_TO_EXC(INTERRUPT_COUNT - 1)] = (uintptr_t)DefaultExceptionHandler };

static _Noreturn void DefaultExceptionHandler(void)
{
	for (;;) {
		// empty.
	}
}

SoftwareSerial serial;

static _Noreturn void RTCoreMain(void)
{

	// SCB->VTOR = ExceptionVectorTable
	WriteReg32(SCB_BASE, 0x08, (uint32_t)ExceptionVectorTable);

	// Initialize software serial
	initializeSS(&serial, 43, 1);

	// Debug serial
	Uart_Init();
	Uart_WriteStringPoll("BOOT");

	BufferHeader* outbound, * inbound;
	uint32_t sharedBufSize = 0;
	if (GetIntercoreBuffers(&outbound, &inbound, &sharedBufSize) == -1) {
		for (;;) {
			// empty.
		}
	}

	for (;;) {

		// Reads a bit from the software serial when it's time to do it
		updateSS(&serial);

		// Fresh data
		if (serial.lastCharReadProcessed == 0) {

			// Read from buffer happened
			serial.lastCharReadProcessed = 1;

			// 16: Component ID, 4: reserved, 1: character.
			char data[16 + 4 + 1] = { /* Component ID: (4 bytes little endian)*/ 0xfc, 0xce, 0xf2, 0x4e, /* 2 bytes little endian*/ 0x95, 0x2c, /* 2 bytes little endian*/ 0x8a, 0x41, /* Normal */ 0x98, 0x7e, 0x3f, 0xda, 0x13, 0x3f, 0xd6, 0x63, /* Reserved: */ 0x00, 0x28, 0x00, 0x03, /* Data: */ 0x00 };
			data[20] = serial.lastCharRead;
			EnqueueData(inbound, outbound, sharedBufSize, data, 21);
		}
	}
}
