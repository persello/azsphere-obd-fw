/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

#include "mt3620-baremetal.h"
#include "mt3620-timer-poll.h"

static const uintptr_t GPT_BASE = 0x21030000;

int microseconds_wait = 0;

void Gpt3_StartUs(int microseconds)
{
	// Stop timer
	WriteReg32(GPT_BASE, 0x50, 0x0);

	microseconds_wait = microseconds;

	// GPT3_INIT = initial counter value
	WriteReg32(GPT_BASE, 0x54, 0x0);

	// GPT3_CTRL
	uint32_t ctrlOn = 0x0;
	ctrlOn |= (0x19) << 16; // OSC_CNT_1US (default value)
	ctrlOn |= 0x1;          // GPT3_EN = 1 -> GPT3 enabled
	WriteReg32(GPT_BASE, 0x50, ctrlOn);
}

int Gpt3_Check() {
	// GPT3_CNT
	if (ReadReg32(GPT_BASE, 0x58) < microseconds_wait) {
		// Not expired yet
		return 1;
	}

	// GPT_CTRL -> disable timer
	WriteReg32(GPT_BASE, 0x50, 0x0);
	return 0;

}
