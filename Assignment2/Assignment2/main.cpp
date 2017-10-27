/*	main.cpp
CPS 472 Sample Code
*/
#pragma once


//#include "winsock.h"  // must include winsock2.h at the very top of the file before include others
#include "headers.h"
#include "cpu.h"
#include "dns.h"
#include "functions.h"
#include "Parameters.h"




int main(int argc, const char* argv[])
{

	string inputString = "";
	if (argc != 2)
	{
		return -1;
	}
	inputString = argv[1];

	int numThreads = 0;
	string filename = "";
	char * ptr;
	//THIS IS HARDCODED NEED TO DEAL WITH CASES!!!!!!!!!!!!
	bool batchMode = false;
	long tempval = strtol(inputString.c_str(), &ptr, 10);
	if ((tempval == 0) || (tempval == LONG_MAX) || (tempval == LONG_MIN)) {
		batchMode = false;
	}


	WSADATA wsaData;
	//Initialize WinSock 
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		printf("WSAStartup error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	Parameters p;

	if (batchMode){
		numThreads = stoi(argv[1]);
		printf("Starting Batch Mode with %d threads... \n", numThreads);
		printf("Reading input file... ");
		if (!readInput(p.inputQueue) ){
			return 0;
		}

		// thread handles are stored here; they can be used to check status of threads, or kill them
		//HANDLE *ptrs = new HANDLE[numThreads];
		//Parameters p;

		// create a mutex for accessing critical sections (including printf)
		//p.mutex = CreateMutex(NULL, 0, NULL);

		// create a semaphore that counts the number of active threads
		printf("...\n");
		p.finished = CreateSemaphore(NULL, 0, numThreads, NULL);
		p.eventQuit = CreateEvent(NULL, true, false, NULL);

		//initializing counters
		p.noDNSrec = 0;
		p.noAuthDNSserver = 0;
		p.localDNStimeout = 0;

		// get current time
		DWORD t = timeGetTime();

		//Create a vector of threads and initialize them
		vector<thread> threads;
		for (int i = 0; i < numThreads; i++) {
			threads.push_back(thread(threadFun, &p));
		}

		//joining threads to main thread 
		for (auto& thread : threads) {
			thread.join();

		}

		// structure p is the shared space between the threads
		/*
		for (int i = 0; i < numThreads; i++) {
			ptrs[i] = CreateThread(NULL, 4096, (LPTHREAD_START_ROUTINE)thread, &p, 0, NULL);
		}
		
		// make sure this thread hangs here until the other two quit; otherwise, the program will terminate prematurely
		for (int i = 0; i < numThreads; i++) {
			WaitForSingleObject(p.finished, INFINITE);
		}
		*/
		printf("Completed %d queries\n",p.outputQueue.size());
		printf("\tSuccessful: ") ;

		size_t processed = p.outputQueue.size();
		if(processed > 0){
			cout << p.transmissionTimes.size() * 100 / p.outputQueue.size()  << "%" << endl;
			cout << "\tNo DNS record: " << p.noDNSrec*100/ p.outputQueue.size() << "%" << endl;
			cout << "\tNo auth DNS server: " << p.noAuthDNSserver * 100 / p.outputQueue.size() << "%" << endl;
		}
		string tempString = "";
		ofstream fout; //input filestream
		fout.open("dns-out.txt");
		while (p.outputQueue.size()!=0) {
			tempString = p.outputQueue.front();
			p.outputQueue.pop();
			fout << tempString << endl;
		}
		fout.close();
		 
		long tempval;
		long totalDelays = 0;
		ofstream fout2; //input filestream
		fout2.open("dnsTranstimes.txt");
		while (p.transmissionTimes.size() != 0) {
			tempval = p.transmissionTimes.front();
			totalDelays += tempval;
			p.transmissionTimes.pop();
			fout2 << tempval << endl;
		}
		fout2.close();

		if (processed > 0)
		cout << "\tAverage Delay: " << totalDelays / processed << " ms" << endl;
		int totalRetx = 0;
		int tempcount;
		ofstream fout3; //input filestream
		fout3.open("dnsTransnums.txt");

		size_t successTrans = p.transmissionNums.size();
		while (p.transmissionNums.size() != 0) {
			tempcount = p.transmissionNums.front();
			totalRetx += tempcount;
			p.transmissionNums.pop();
			fout3 << tempcount << endl;
		}

		if(successTrans > 0)
		cout << "\tAverage retx attempts: " << totalRetx / successTrans << endl;

		fout3.close();

		
	}



	/*
	printf("-----------------\n");

	// print our primary/secondary DNS IPs



	printf("-----------------\n");

	CPU cpu;
	// average CPU utilization over 500 ms; must sleep *after* the constructor of class CPU and between calls to GetCpuUtilization
	Sleep(500);
	// now print
	double util = cpu.GetCpuUtilization(NULL);
	printf("current CPU utilization %f%%\n", util);

	printf("-----------------\n");
	*/


	if (!batchMode) {
		DNS mydns;
		mydns.printDNSServer();
		mydns.setInputText(inputString);
		mydns.queryType();
		mydns.createPKT();
		mydns.sendPKT();
		printf("\n%s",mydns.printReply().c_str());
	}


	
	WSACleanup();

	return 0;
}
