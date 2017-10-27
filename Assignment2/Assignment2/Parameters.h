#pragma once
#include "headers.h"

// this class is passed to all threads, acts as shared memory
class Parameters {
public:
	//HANDLE mutex;
	mutex dataLock;
	HANDLE finished;
	HANDLE eventQuit;
	queue<string> inputQueue;
	queue<int> transmissionNums;
	queue<long> transmissionTimes;
	queue<string> outputQueue;
	int noDNSrec;
	int noAuthDNSserver;
	int localDNStimeout;


};
