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
	if (argc !=2 )
	{
		return -1;
	}
	inputString = argv[1];
	
	//WqueryConstructor(inputString, typeOfQuery);

	WSADATA wsaData;

	//Initialize WinSock 
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		printf("WSAStartup error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}


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

	// thread handles are stored here; they can be used to check status of threads, or kill them
	HANDLE *ptrs = new HANDLE[2];
	Parameters p;

	// create a mutex for accessing critical sections (including printf)
	p.mutex = CreateMutex(NULL, 0, NULL);

	// create a semaphore that counts the number of active threads
	p.finished = CreateSemaphore(NULL, 0, 2, NULL);
	p.eventQuit = CreateEvent(NULL, true, false, NULL);

	// get current time
	DWORD t = timeGetTime();

	// structure p is the shared space between the threads
	ptrs[0] = CreateThread(NULL, 4096, (LPTHREAD_START_ROUTINE)thread, &p, 0, NULL);
	ptrs[1] = CreateThread(NULL, 4096, (LPTHREAD_START_ROUTINE)thread, &p, 0, NULL);

	// make sure this thread hangs here until the other two quit; otherwise, the program will terminate prematurely
	WaitForSingleObject(p.finished, INFINITE);
	WaitForSingleObject(p.finished, INFINITE);


	// testing the DNS query

	//string host = "www.yahoo.com"; //for constructing type A query
	//string host = "224.111.229.213.in-addr.arpa";  //for constructing type PTR Query
	/*
	int pkt_size = sizeof(FixedDNSheader) + sizeof(QueryHeader) + host.size() + 2;

	char* pkt = new char[pkt_size];

	FixedDNSheader * dHDR = (FixedDNSheader *)pkt;
	QueryHeader *qHDR = (QueryHeader*)(pkt + pkt_size - sizeof(QueryHeader));

	dHDR->ID = htons(102);
	dHDR->questions = htons(1);
	dHDR->addRRs = 0;
	dHDR->answers = 0;
	dHDR->authRRs = 0;
	dHDR->flags = htons(DNS_QUERY | DNS_RD | DNS_STDQUERY);
	
	//	dHDR->flags = htons( 0x0100 );  

	int position = host.find(".");

	string sub_str;

	int i = 0, sub_size = 0, hdr_size = sizeof(FixedDNSheader);

	host += ".";
	while (position != -1)
	{
		sub_size = position - i;
		sub_str = host.substr(i, position);

		pkt[hdr_size + i] = sub_size;  // specify the size of the chunk (subdomain)
		i++;
		memcpy(pkt + hdr_size + i, sub_str.c_str(), sub_size); // specify the actual subdomain

		i += sub_size;
		position = host.find(".", i);
	}
	pkt[hdr_size + i] = 0;

	qHDR->qclass = htons(DNS_INET);
	//	qHDR->qclass = htons( 0x0001); 

	qHDR->type = htons(DNS_A); //for constructing type a Query
	//qHDR->type = htons(DNS_PTR);
	//	qHDR->type = htons( DNS_PTR ); 
	*/

	/*
	
	Winsock ws;

	SOCKET sock = ws.OpenSocket(); // defined in winsock.h
	
								   // set up the address of where we're sending data
	struct sockaddr_in send_addr;
	send_addr.sin_family = AF_INET;
	dnsIP = "4.2.2.1";
	//send_addr.sin_addr.S_un.S_addr = inet_addr(dnsIP.c_str()); // 208.67.222.222
	send_addr.sin_addr.S_un.S_addr = inet_addr(dnsIP.c_str()); // 208.67.222.222
	
	send_addr.sin_port = htons(53);

	int send_addrSize = sizeof(struct sockaddr_in);

	int sentbytes = sendto(sock, pkt, pkt_size, 0, (struct sockaddr*) &send_addr, send_addrSize);
	cout << "sentbytes=" << sentbytes << endl;

	for (int i = 0; i< pkt_size; i++)
	{
		printf("i=%d %c\n", i, pkt[i]);
	}
	cout << endl;

	char recv_buf[512];

	int recvbytes = 0;
	if (sentbytes > 0)
		recvbytes = recvfrom(sock, recv_buf, 512, 0, (sockaddr *)&send_addr, &send_addrSize);

	cout << "recv_bytes=" << recvbytes << endl;

	FixedDNSheader * rDNS = (FixedDNSheader *)recv_buf;
	cout << "ID=" << ntohs(dHDR->ID) << "??" << ntohs(rDNS->ID) << endl;
	cout << "questions=" << ntohs(rDNS->questions) << endl;
	cout << "Answers=" << ntohs(rDNS->answers) << endl;
	cout << "authRRs=" << ntohs(rDNS->authRRs) << endl;
	cout << "addRRs=" << ntohs(rDNS->addRRs) << endl;

	printf("flag 0x=%x\n", ntohs(rDNS->flags));
	unsigned short rcode = 0x0F;
	rcode = rcode & ntohs(rDNS->flags);
	cout << "Rcode= " << rcode << endl; ;

	// for debugging:
	//	for ( int i = 0; i< recvbytes; i++)
	//	{
	//		printf("%x\t", i, recv_buf[i] ); 
	//	}
	//	cout<<endl; 
	

	closesocket(sock);
	
	delete[] pkt;
	

	printf("Terminating main(), completion time %d ms\n", timeGetTime() - t);
	*/

	DNS mydns;
	mydns.printDNSServer();
	mydns.setInputText(inputString);
	mydns.queryType();
	mydns.createPKT();
	mydns.sendPKT();

	WSACleanup();

	return 0;
}
