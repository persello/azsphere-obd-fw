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

	/// <summary> The first free byte of the buffer. </summary>
	size_t head;

	/// <summary> The last used byte of the buffer. </summary>
	size_t tail;

	/// <summary> The size of the circular buffer. </summary>
	size_t size;

	/// <summary> Full, partial or empty. </summary>
	bufferStatus status;

} buffer_t;

/// <summary> Adds a character to the buffer.
/// <param name="_buffer"> The buffer in which to add the character. </param>
/// <param name="_data"> The character to be added to the buffer. </param>
/// <returns> 0 if successful, -1 if full </returns>
/// </summary>
int putCharBuffer(buffer_t* _buffer, char _data);

/// <summary> Gets a character from the buffer.
/// <param name="_buffer"> The buffer from which to read the character. </param>
/// <param name="_data"> The variable in which the output is written. </param>
/// <returns> 0 if successful, -1 if empty </returns>
/// </summary>
int getCharBuffer(buffer_t* _buffer, char* _data);
