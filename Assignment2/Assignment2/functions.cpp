
#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "functions.h"
#include "headers.h"
#include "Parameters.h"
#include "dns.h"

UINT threadFun(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);
	DNS mydns;
	string inputString = "";
	int internalNoDNScount;
	int internalNoauthDNScount;
	// wait for mutex, then print and sleep inside the critical section
	//WaitForSingleObject(p->mutex, INFINITE);					// lock mutex
	p->dataLock.lock();
	//printf("Thread %d started\n", GetCurrentThreadId());		// always print inside critical section to avoid screen garbage
	mydns.printDNSServer();
	p->dataLock.unlock();
	//ReleaseMutex(p->mutex);

	while (true) {
		//WaitForSingleObject(p->mutex, INFINITE);
		p->dataLock.lock();
		if (p->inputQueue.size() == 0) {
			//ReleaseMutex(p->mutex);
			p->noDNSrec += mydns.retrieveNoDNScount();
			p->noAuthDNSserver += mydns.retrieveNoAuthDNScount();
			p->dataLock.unlock();
			break;
		}
		inputString = p->inputQueue.front();
		p->inputQueue.pop();
		//ReleaseMutex(p->mutex);
		p->dataLock.unlock();

		mydns.setInputText(inputString);
		mydns.queryType();
		mydns.createPKT();
		mydns.sendPKT();
		

		//WaitForSingleObject(p->mutex, INFINITE);
		p->dataLock.lock();
		p->outputQueue.push(mydns.printReply());
		//IP lookup was successful so push transmission number and times to the queues
		if (mydns.lookUpSuccess()) {
			p->transmissionNums.push(mydns.printTnum());
			p->transmissionTimes.push(mydns.retrieveTransTime());
		}
		
		//ReleaseMutex(p->mutex);
		p->dataLock.unlock();
	}


																// signal that this thread has exitted
	ReleaseSemaphore(p->finished, 1, NULL);

	return 0;
}

bool readInput(queue<string> & inputQueue) {
	//READING FROM FILE AND WRITING TO FILE FROM CPS 570 9-05-2017.pptx Dr. Zhongmei Yao
	string filename = "dns-in.txt";
	ifstream fin; //input filestream
	fin.open(filename); //opening file to read
	string url = "";

	//printf("size of queue is %d entries \n", urlQueue.size());

	if (fin.fail()) {
		printf("Error opening specified file\n");
		return false;
	}
	int i = 0;
	int j = 1;
	while (!fin.eof())
	{
		fin >> url;
		if(j%2!=0){
			inputQueue.push(url);
			//printf("%s\n", url.c_str());
			i++;
		}
		
		j++;
		
	}
	fin.close();
	//if file is empty, terminate program
	if (inputQueue.front() == "") {
		//printf("Invalid File!\n");
		return false;
	}
	printf("found %d entries \n",i);
	return true;

}



