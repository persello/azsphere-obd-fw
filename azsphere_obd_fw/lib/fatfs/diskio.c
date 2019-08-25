/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "../ulibSD/ulibsd/sd_io.h"

SD_DEV dev[1];

/* Definitions of physical drive number for each drive */
#define DEV_MMC		0	/* Example: Map MMC/SD card to physical drive 0 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_MMC:
		result = SD_Status(dev);

		switch (result) {
		case SD_OK:
			stat = RES_OK;
			break;
		case SD_BUSY:
			stat = RES_NOTRDY;
			break;
		case SD_ERROR:
			stat = RES_ERROR;
			break;
		case SD_NOINIT:
			stat = RES_ERROR;
			break;
		case SD_NORESPONSE:
			stat = RES_ERROR;
			break;
		case SD_PARERR:
			stat = RES_PARERR;
			break;
		case SD_REJECT:
			stat = RES_WRPRT;
			break;
		}

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_MMC:
		result = SD_Init(dev);

		switch (result) {
		case SD_OK:
			stat = RES_OK;
			break;
		case SD_BUSY:
			stat = RES_NOTRDY;
			break;
		case SD_ERROR:
			stat = RES_ERROR;
			break;
		case SD_NOINIT:
			stat = RES_ERROR;
			break;
		case SD_NORESPONSE:
			stat = RES_ERROR;
			break;
		case SD_PARERR:
			stat = RES_PARERR;
			break;
		case SD_REJECT:
			stat = RES_WRPRT;
			break;
		}

		return stat;
	}

	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE* buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_MMC:
		// translate the arguments here

		// Reads a block at a time (assuming sector size is 512)
		for (int i = 0; i < count; i++) {
			result = SD_Read(dev, (void*)buff + (SD_BLK_SIZE * i), sector + i, 0, SD_BLK_SIZE);

			if (result != SD_OK) {
				break;
			}
		}

		switch (result) {
		case SD_OK:
			res = RES_OK;
			break;
		case SD_BUSY:
			res = RES_NOTRDY;
			break;
		case SD_ERROR:
			res = RES_ERROR;
			break;
		case SD_NOINIT:
			res = RES_ERROR;
			break;
		case SD_NORESPONSE:
			res = RES_ERROR;
			break;
		case SD_PARERR:
			res = RES_PARERR;
			break;
		case SD_REJECT:
			res = RES_WRPRT;
			break;
		}

		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE* buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_MMC:
		// translate the arguments here

		for (int i = 0; i < count; i++) {
			result = SD_Write(dev, buff + (SD_BLK_SIZE * i), sector + i);

			if (result != SD_OK) {
				break;
			}
		}

		switch (result) {
		case SD_OK:
			res = RES_OK;
			break;
		case SD_BUSY:
			res = RES_NOTRDY;
			break;
		case SD_ERROR:
			res = RES_ERROR;
			break;
		case SD_NOINIT:
			res = RES_ERROR;
			break;
		case SD_NORESPONSE:
			res = RES_ERROR;
			break;
		case SD_PARERR:
			res = RES_PARERR;
			break;
		case SD_REJECT:
			res = RES_WRPRT;
			break;
		}

		return res;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void* buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_MMC:

		switch (cmd) {
		case GET_BLOCK_SIZE:
			buff = malloc(sizeof(SD_BLK_SIZE));
			*(int*)buff = SD_BLK_SIZE;
			break;
		case GET_SECTOR_COUNT:
			buff = malloc(sizeof(dev->last_sector));
			*(DWORD*)buff = (dev->last_sector) + 1;
			break;
		}

		return res;
	}

	return RES_PARERR;
}

