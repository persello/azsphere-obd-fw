/*
 *  spi_io.h
 *  Author: Nelson Lombardo (C) 2015
 *  e-mail: nelson.lombardo@gmail.com
 *  License at the end of file.
 */

#ifndef _SPI_IO_H_
#define _SPI_IO_H_

void SPI_MOSI_Init(void);
void SPI_MISO_Init(void);
void SPI_SCK_Init(void);
void SPI_CS_Init(void);

void SPI_MOSI_Set(unsigned char);
unsigned char SPI_MISO_Get(void);
void SPI_SCK_Set(unsigned char);
void SPI_CS_Set(unsigned char);

#endif

