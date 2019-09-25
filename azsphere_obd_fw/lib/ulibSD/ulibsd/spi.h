#pragma once

void SPI_CS_Set(unsigned char s);

void SPI_SCK_Slow(void);

void SPI_SCK_Fast(void);

void SPI_Send(const char* buf, unsigned int bytes);

void SPI_Receive(const char** buf, unsigned int bytes);

void SPI_Init(void);