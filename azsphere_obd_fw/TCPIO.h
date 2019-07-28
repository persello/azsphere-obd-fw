#pragma once
#include "buffer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <applibs/log.h>
#include <unistd.h>
#include <arpa/inet.h>

buffer_t inputBuffer;
buffer_t outputBuffer;

/*size_t bufferSize, unsigned int port, void(*processData)(buffer_t*)*/
int initSocket();

int endSocket();

int sendData(char* data);