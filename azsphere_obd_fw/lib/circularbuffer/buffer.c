#include "buffer.h"

#include <malloc.h>
// #include "../memorylogger/Inc/Public/memorylogger.h"
#include <string.h>

/* Due to some compilation order restrictions of the current environment, this
	file has been included at the end of its header. Being this not a good practice,
	remember not to do this in other files and to revert back to the standard including
	order when a better solution is available. */

void initCircBuffer(buffer_t* _buffer, size_t _size) {
	_buffer->head = 0;
	_buffer->tail = 0;
	_buffer->size = _size;
	_buffer->content = (char*)malloc(_size * sizeof(char));
	_buffer->status = BUFFER_EMPTY;
}

void headForward(buffer_t* _buffer) {

	// Moves the head making it go back to 0 when size - 1 is reached
	_buffer->head = (_buffer->head + 1) % (_buffer->size - 1);

	// Sets the status to full when head = tail, otherwise it is surely partial (not empty)
	if (_buffer->head == _buffer->tail) {
		_buffer->status = BUFFER_FULL;
	}
	else {
		_buffer->status = BUFFER_PARTIAL;
	}
}

void tailForward(buffer_t* _buffer) {

	// Moves the tail making it go back to 0 when size - 1 is reached
	_buffer->tail = (_buffer->tail + 1) % (_buffer->size - 1);

	// Sets the status to empty when head = tail, otherwise it is surely partial (not full)
	if (_buffer->head == _buffer->tail) {
		_buffer->status = BUFFER_EMPTY;
	}
	else {
		_buffer->status = BUFFER_PARTIAL;
	}
}

int putCharBuffer(buffer_t* _buffer, char _data) {
	if (_buffer->status != BUFFER_FULL) {
		_buffer->content[_buffer->head] = _data;
		headForward(_buffer);
	}
	else {
		return -1;
	}
	return 0;
}

int getCharBuffer(buffer_t* _buffer, char* _data) {
	if (_buffer->status != BUFFER_EMPTY) {
		*_data = _buffer->content[_buffer->tail];
		tailForward(_buffer);
	}
	else {
		return -1;
	}
	return 0;
}