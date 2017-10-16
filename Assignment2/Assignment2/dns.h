/*	dns.h
	CPS 470 Sample Code
*/
#pragma once
#include <Iphlpapi.h>
#include "lookup.h"



class DNS {
public:
	//FixedDNSheader * DNS::CreateDNSHeader(void);
	void printDNSServer (void);
	void createPKT(void);
	void sendPKT(void);
	void setInputText(string & input);
	void queryType(void);
	void responseParser(void);
	void findName(RR * rrArray);
	void Readbuffer(string & parsedInput, char * buffer);


private:
	SOCKET sock;
	char recv_buf[512];
	string input_text;
	string PKTtype;
	string localDNSIP;
	int pkt_size;
	char * pkt;
	FixedDNSheader * dHDR;
	QueryHeader *qHDR;

};