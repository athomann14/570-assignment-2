/*	dns.h
	CPS 470 Sample Code
*/
#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "headers.h"
#include "dns.h"
#include "winsock.h"



// NOTE: link with Iphlpapi.lib; prints primary/second DNS server info

void DNS::printDNSServer(void)
{
	// MSDN sample code
	FIXED_INFO *FixedInfo;
	ULONG    ulOutBufLen;
	DWORD    dwRetVal;
	IP_ADDR_STRING * pIPAddr;

	ulOutBufLen = sizeof(FIXED_INFO);
	FixedInfo = (FIXED_INFO *) GlobalAlloc( GPTR, sizeof( FIXED_INFO ) );
	ulOutBufLen = sizeof( FIXED_INFO );

	if(ERROR_BUFFER_OVERFLOW == GetNetworkParams(FixedInfo, &ulOutBufLen)) {
		GlobalFree( FixedInfo );
		FixedInfo = (FIXED_INFO *)GlobalAlloc( GPTR, ulOutBufLen );
	}

	if ( dwRetVal = GetNetworkParams( FixedInfo, &ulOutBufLen ) ) {
		printf( "Call to GetNetworkParams failed. Return Value: %08x\n", dwRetVal );
	}
	else {
		printf( "Host Name: %s\n", FixedInfo->HostName );
		printf( "Domain Name: %s\n", FixedInfo->DomainName );

		printf( "Local DNS Servers:\n" );
		//printf( "\t%s\n", FixedInfo->DnsServerList.IpAddress.String);
		DNS::localDNSIP = FixedInfo->DnsServerList.IpAddress.String;

		pIPAddr = FixedInfo->DnsServerList.Next;
		while ( pIPAddr ) {
			printf( "\t%s\n", pIPAddr ->IpAddress.String);
			pIPAddr = pIPAddr ->Next;
		}
	}

	GlobalFree (FixedInfo);
}

void DNS::createPKT(void)
{
	string host = "";
	assert(strcmp(DNS::PKTtype.c_str(),"ERROR")!=0);

	//Preparing string depending on query type
	if (strcmp(DNS::PKTtype.c_str(), "A") == 0) {
		host = DNS::input_text;
	}
	else {
	//ELSE PTR query type
		struct in_addr requestIP;
		requestIP.S_un.S_addr = htonl((inet_addr(DNS::input_text.c_str())));
		host = inet_ntoa(requestIP);
		host += ".in-addr.arpa";
	}

	//string 
	DNS::pkt_size = sizeof(FixedDNSheader) + sizeof(QueryHeader) + host.size() + 2;
	DNS::pkt = new char[pkt_size];
	DNS::dHDR = (FixedDNSheader *)pkt;
	DNS::qHDR = (QueryHeader*)(pkt + pkt_size - sizeof(QueryHeader));

	dHDR->ID = htons(102);
	dHDR->questions = htons(1);
	dHDR->addRRs = 0;
	dHDR->answers = 0;
	dHDR->authRRs = 0;
	dHDR->flags = htons(DNS_QUERY | DNS_RD | DNS_STDQUERY);

	//building Question
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


	//setting query type
	if (strcmp(DNS::PKTtype.c_str(), "A") == 0) {
		qHDR->type = htons(DNS_A); //for constructing type a Query
	}
	else {
		//ELSE PTR Request!
		qHDR->type = htons(DNS_PTR);
	}


}

void DNS::sendPKT(void)
{


	Winsock ws;
	DNS::sock = ws.OpenSocket();

								   // set up the address of where we're sending data
	struct sockaddr_in send_addr;
	send_addr.sin_family = AF_INET;
	send_addr.sin_addr.S_un.S_addr = inet_addr(DNS::localDNSIP.c_str()); // 208.67.222.222

	send_addr.sin_port = htons(53);

	int send_addrSize = sizeof(struct sockaddr_in);

	int sentbytes = sendto(sock, pkt, pkt_size, 0, (struct sockaddr*) &send_addr, send_addrSize);
	cout << "sentbytes=" << sentbytes << endl;

	for (int i = 0; i< pkt_size; i++)
	{
		printf("i=%d %c\n", i, pkt[i]);
	}
	cout << endl;

	

	int recvbytes = 0;
	if (sentbytes > 0)
		recvbytes = recvfrom(sock, DNS::recv_buf, 512, 0, (sockaddr *)&send_addr, &send_addrSize);

	//parser here

	DNS::responseParser();


	//end parse here

	closesocket(sock);

	delete[] DNS::pkt;

}

void DNS::setInputText(string & input)
{
	DNS::input_text = input;

}

void DNS::queryType(void)
{
	//unsigned long address = 0;

	//determine if input is an IP address
	string typeOfQuery = "PTR";
	bool nextStage = true;
	for (int i = 0; i < DNS::input_text.size(); i++) {
		if (!(isdigit(DNS::input_text[i]) | DNS::input_text[i] == '.')) {
			nextStage = false;
			break;
		}
	}

	//input is an IP so query type is PTR
	if (nextStage) {
		if (inet_addr(DNS::input_text.c_str()) == INADDR_NONE) {
			printf("Not a legitimate IP Address!\n");
			typeOfQuery = "ERROR";
		}
		
	}
	else {
		typeOfQuery = "A";
	}

	DNS::PKTtype = typeOfQuery;


}

void DNS::responseParser(void) {



	//cout << "recv_bytes=" << recvbytes << endl;

	FixedDNSheader * rDNS = (FixedDNSheader *)recv_buf;
	cout << "ID=" << ntohs(dHDR->ID) << "??" << ntohs(rDNS->ID) << endl;
	cout << "questions=" << ntohs(rDNS->questions) << endl;
	cout << "Answers=" << ntohs(rDNS->answers) << endl;
	cout << "authRRs=" << ntohs(rDNS->authRRs) << endl;
	cout << "addRRs=" << ntohs(rDNS->addRRs) << endl;

	printf("flag 0x=%x\n", ntohs(rDNS->flags));
	unsigned short rcode = 0x0F;
	rcode = rcode & ntohs(rDNS->flags);
	cout << "Rcode= " << rcode << endl;
	int numAns = ntohs(rDNS->answers);
	int numAuthAns = ntohs(rDNS->authRRs);
	int numAddAns = ntohs(rDNS->addRRs);
	
	
	switch (rcode) {

	case 0 :
	{
		printf("Success DNS LOOKUP!\n");	
		
		RR * rrArray = new RR[numAns];
		for (int i = 0; i < numAns; i++) {
			rrArray->rName = & recv_buf[DNS::pkt_size];
			DNS::findName(rrArray);

		}




		uint8_t hello = 192;
		uint8_t offsetHeader = recv_buf[DNS::pkt_size];
		bool isPointer = false;
		if (recv_buf[DNS::pkt_size] == '1' &&  recv_buf[DNS::pkt_size + 1] == '1') {
			isPointer = true;
		}

		if (isPointer) {

		}


		for (int i = 0; i < numAddAns; i++){


		}
		
		
		break;
	}
	case 1:
	{
		printf("FORMAT ERROR!\n");
		break;
	}
	case 2:
	{
		printf("SERVER FAILURE!\n");
		break;
	}
	case 3:
	{
		printf("NAME ERROR, DOMAIN NAME DOESN'T EXIST!\n");
		break;
	}
	case 4:
	{
		printf("NOT IMPLEMENTED!\n");
		break;
	}
	case 5:
	{
		printf("REFUSED!\n");
		break;
	}
	 default:
		printf("General Failure!\n");
		break;
	}
	
	recv_buf[DNS::pkt_size];

}

void DNS::findName(RR * rrArray) {
	// binary 1100 0000 anded with name to detect if entry is a pointer
	if ((*rrArray->rName & PTR_NAME) == PTR_NAME) {
		printf("THIS IS A POINTER WORKED!\n");
		rrArray->rName += 2;
		rrArray->rrFields = (FixedRR *)rrArray->rName; 
		cout << "type = " << ntohs(rrArray->rrFields->type) << endl;
		cout << "class = " << ntohs(rrArray->rrFields->RRclass) << endl;
		cout << "TTL = " << ntohs(rrArray->rrFields->ttl) << endl;
		cout << "RDLength = " << ntohs(rrArray->rrFields->len) << endl;
		rrArray->rData = rrArray->rName + 10; //plus the ten bytes of the middle RR record contents
		string newstring = "";
		Readbuffer(newstring, (rrArray->rData));
		printf("READ RESPONSE\n");
	}


}

void DNS::Readbuffer(string & parsedInput, char * buffer) {

	string tempstring = "";
	int hello = *(buffer);
	while (hello > 0) {
		for (int i = 0; i < hello; i++) {
			tempstring += *(buffer + 1 + i);
		}
		tempstring += '.';
		buffer += hello + 1;
		hello = *(buffer);
	}
	parsedInput = tempstring.substr(0, tempstring.length()-1);
	printf("READ RESPONSE\n");
}
