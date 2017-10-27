/*	dns.h
	CPS 470 Sample Code
*/
#pragma once
#include <Iphlpapi.h>
#include "lookup.h"



class DNS {
	enum replyType { answer, authAns, addAns };
public:
	//FixedDNSheader * DNS::CreateDNSHeader(void);
	void printDNSServer (void);
	void createPKT(void);
	void sendPKT(void);
	void setInputText(string & input);
	void queryType(void);
	void responseParser(void);
	void findName(RR * rrArray);
	void Readbuffer(string & parsedInput, char * buffer, int * timesRun);
	string printReply();
	int printTnum();
	bool lookUpSuccess();
	long retrieveTransTime();
	void setCounters();
	int retrieveNoDNScount();
	int retrieveNoAuthDNScount();



private:
	replyType type;
	SOCKET sock;
	char recv_buf[512];
	string input_text;
	string PKTtype;
	string localDNSIP;
	int pkt_size;
	char * pkt;
	FixedDNSheader * dHDR;
	QueryHeader *qHDR;
	int nameFieldSize;
	string replyMessage;
	int count;
	bool IPlookupSuccess;
	long lookUpTimer;
	int internalNoDNScount;
	int internalNoauthDNScount;

};