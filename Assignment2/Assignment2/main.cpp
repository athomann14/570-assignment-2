/*	main.cpp
	CPS 470 Sample Code
*/

#include "headers.h"
#include "cpu.h"
#include "dns.h"

// this class is passed to all threads, acts as shared memory
class Parameters{
public:
	HANDLE mutex;
	HANDLE finished;
	HANDLE eventQuit;
};

// this function is where the thread starts
UINT thread(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	// wait for mutex, then print and sleep inside the critical section
	WaitForSingleObject(p->mutex, INFINITE);					// lock mutex
	printf ("Thread %d started\n", GetCurrentThreadId ());		// always print inside critical section to avoid screen garbage
	Sleep (1000);
	ReleaseMutex(p->mutex);										// release critical section

	// signal that this thread has exitted
	ReleaseSemaphore(p->finished, 1, NULL);

	return 0;
}

int main(void)
{
	WSADATA wsaData;

	//Initialize WinSock 
	WORD wVersionRequested = MAKEWORD(2,2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		printf("WSAStartup error %d\n", WSAGetLastError ());
		WSACleanup();	
		return -1;
	}


	printf ("-----------------\n");

	// print our primary/secondary DNS IPs
	DNS mydns;
	mydns.printDNSServer();

	printf ("-----------------\n");

	CPU cpu;
	// average CPU utilization over 500 ms; must sleep *after* the constructor of class CPU and between calls to GetCpuUtilization
	Sleep (500);
	// now print
	double util = cpu.GetCpuUtilization (NULL);
	printf ("current CPU utilization %f%%\n", util);

	printf ("-----------------\n");

	// thread handles are stored here; they can be used to check status of threads, or kill them
	HANDLE *ptrs = new HANDLE [2];
	Parameters p;
	
	// create a mutex for accessing critical sections (including printf)
	p.mutex = CreateMutex (NULL, 0, NULL);
	
	// create a semaphore that counts the number of active threads
	p.finished = CreateSemaphore(NULL, 0, 2, NULL);
	p.eventQuit = CreateEvent (NULL, true, false, NULL);

	// get current time
	DWORD t = timeGetTime();

	// structure p is the shared space between the threads
	ptrs[0] = CreateThread(NULL, 4096, (LPTHREAD_START_ROUTINE)thread, &p, 0, NULL);
	ptrs[1] = CreateThread(NULL, 4096, (LPTHREAD_START_ROUTINE)thread, &p, 0, NULL);

	// make sure this thread hangs here until the other two quit; otherwise, the program will terminate prematurely
	WaitForSingleObject(p.finished, INFINITE);
	WaitForSingleObject(p.finished, INFINITE);

	printf ("Terminating main(), completion time %d ms\n", timeGetTime()-t);

	WSACleanup();	

	return 0; 
}
