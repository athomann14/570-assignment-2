
#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "functions.h"
#include "headers.h"
#include "Parameters.h"
UINT thread(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	// wait for mutex, then print and sleep inside the critical section
	WaitForSingleObject(p->mutex, INFINITE);					// lock mutex
	printf("Thread %d started\n", GetCurrentThreadId());		// always print inside critical section to avoid screen garbage
	Sleep(1000);
	ReleaseMutex(p->mutex);										// release critical section

																// signal that this thread has exitted
	ReleaseSemaphore(p->finished, 1, NULL);

	return 0;
}



