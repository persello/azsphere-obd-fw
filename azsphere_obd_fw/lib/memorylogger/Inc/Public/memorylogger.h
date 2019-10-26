#include <stddef.h>
#include <malloc.h>
#include <applibs/log.h>

/////////////////////////////////////////////

void* my_malloc(size_t size, const char* file, int line)
{

	void* p = malloc(size);
	Log_Debug("MEMORY: Allocated = %s, %i, %p[%li]\n", file, line, p, size);

	return p;
}

#define malloc(X) my_malloc( X, __FILE__, __LINE__)

////////////////////////////////////////////////

void my_free(void* f, const char* file, int line)
{

	Log_Debug("MEMORY: Deallocated = %s, %i, %p[%li]\n", file, line, f, sizeof(&f));
	free(f);
}

#define free(X) my_free( X, __FILE__, __LINE__)
