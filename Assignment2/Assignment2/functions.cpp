
#pragma once
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

void queryType(string & queryString, string & typeOfQuery)
{
	//unsigned long address = 0;
	//determine if input is an IP address
	 typeOfQuery = "PTR";
	bool nextStage = true;
	for (int i = 0; i < queryString.size(); i++) {
		if (!(isdigit(queryString[i]) | queryString[i] == '.')) {
			nextStage = false;
			break;
		}
	}

	//input is an IP so query type is PTR
	if (nextStage) {
		if (inet_addr(queryString.c_str()) == INADDR_NONE) {
			printf("Not a legitimate IP Address!\n");
			typeOfQuery = "ERROR";
		}
	}
	else {
		typeOfQuery = "A";
	}


	

}

