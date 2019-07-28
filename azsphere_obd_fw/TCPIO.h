#pragma once

typedef struct {

	char command[4];

	char *args;

} AppMessage;

char inputBuffer[1024];
char outputBuffer[1024];

int ibFree