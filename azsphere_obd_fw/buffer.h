#pragma once
#include <stddef.h>
#include <malloc.h>
#include <string.h>

enum bufferStatus {
	BUFFER_EMPTY,
	BUFFER_PARTIAL,
	BUFFER_FULL
};

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

/// <summary> Creates a new buffer.
/// <param name="_size"> The size in bytes of the new buffer. </param>
/// <returns> The newly created buffer. </returns>
/// </summary>
buffer_t newBuffer(int _size);

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
