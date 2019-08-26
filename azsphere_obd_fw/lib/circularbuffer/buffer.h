#pragma once
#include <stddef.h>

typedef enum {
	BUFFER_EMPTY,
	BUFFER_PARTIAL,
	BUFFER_FULL
} bufferStatus;

/// <summary> The main buffer struct. </summary>
typedef struct {
	/// <summary> The content of the circular buffer. </summary>
	char* content;

	/// <summary> The first free BYTE of the buffer. </summary>
	size_t head;

	/// <summary> The last used BYTE of the buffer. </summary>
	size_t tail;

	/// <summary> The size of the circular buffer. </summary>
	size_t size;

	/// <summary> Full, partial or empty. </summary>
	bufferStatus status;

} buffer_t;

void initCircBuffer(buffer_t* _buffer, size_t _size);

/// <summary> Adds a character to the buffer. </summary>
/// <param name="_buffer"> The buffer in which to add the character. </param>
/// <param name="_data"> The character to be added to the buffer. </param>
/// <returns> 0 if successful, -1 if full </returns>
int putCharBuffer(buffer_t* _buffer, char _data);

/// <summary> Gets a character from the buffer. </summary>
/// <param name="_buffer"> The buffer from which to read the character. </param>
/// <param name="_data"> The variable in which the output is written. </param>
/// <returns> 0 if successful, -1 if empty </returns>
int getCharBuffer(buffer_t* _buffer, char* _data);
