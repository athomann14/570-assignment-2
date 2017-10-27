/*	dns.h
	CPS 470 Sample Code
*/
#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "headers.h"
#include "dns.h"
#include "winsock.h"



// NOTE: link with Iphlpapi.lib; prints primary/second DNS server info
//Print DNS functionality provided by Dr. Yao
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
		//printf( "Host Name: %s\n", FixedInfo->HostName );
		//printf( "Domain Name: %s\n", FixedInfo->DomainName );

		//printf( "Local DNS Servers:\n" );
		//printf( "\t%s\n", FixedInfo->DnsServerList.IpAddress.String);
		DNS::localDNSIP = FixedInfo->DnsServerList.IpAddress.String;

		pIPAddr = FixedInfo->DnsServerList.Next;
		while ( pIPAddr ) {
			//printf( "\t%s\n", pIPAddr ->IpAddress.String);
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

	DNS::IPlookupSuccess = false;
	DNS::lookUpTimer = 0;
	Winsock ws;
	DNS::sock = ws.OpenSocket();

								   // set up the address of where we're sending data
	struct sockaddr_in send_addr;
	send_addr.sin_family = AF_INET;
	send_addr.sin_addr.S_un.S_addr = inet_addr(DNS::localDNSIP.c_str()); // 208.67.222.222

	send_addr.sin_port = htons(53);

	int send_addrSize = sizeof(struct sockaddr_in);

	int count = 1;
	struct timeval tp;
	tp.tv_sec = 30;
	tp.tv_usec = 0;
	int recvbytes = 0;
	
	clock_t timer;
	timer = clock();
	

	while (count < 4) {
		int sentbytes = sendto(sock, pkt, pkt_size, 0, (struct sockaddr*) &send_addr, send_addrSize);
		fd_set fd;
		FD_ZERO(&fd); // clear the set
		FD_SET(DNS::sock, &fd); // add your socket to the set
		//int available = select(0, &fd, NULL, NULL, &tp);
		//if (available > 0) {
		if (select(0, &fd, NULL, NULL, &tp)>0){
			recvbytes = recvfrom(sock, DNS::recv_buf, 512, 0, (sockaddr *)&send_addr, &send_addrSize);
			timer = clock() - timer;
			DNS::lookUpTimer = (timer);
			DNS::responseParser();
			DNS::count = count;
			break;
			// parse the response
			// break from the loop
		}
		count++;
		if (DNS::replyMessage.length() == 0){
			DNS::replyMessage += "Local DNS server timeout";

		}

	}

	/*
	int sentbytes = sendto(sock, pkt, pkt_size, 0, (struct sockaddr*) &send_addr, send_addrSize);
	//cout << "sentbytes=" << sentbytes << endl;

	
	for (int i = 0; i< pkt_size; i++)
	{
		printf("i=%d %c\n", i, pkt[i]);
	}
	cout << endl;
	
	int recvbytes = 0;
	if (sentbytes > 0)
		recvbytes = recvfrom(sock, DNS::recv_buf, 512, 0, (sockaddr *)&send_addr, &send_addrSize);
	*/
	//parser here

	


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

	FixedDNSheader * rDNS = (FixedDNSheader *)recv_buf;
	/*
	cout << "ID=" << ntohs(dHDR->ID) << "??" << ntohs(rDNS->ID) << endl;
	cout << "questions=" << ntohs(rDNS->questions) << endl;
	cout << "Answers=" << ntohs(rDNS->answers) << endl;
	cout << "authRRs=" << ntohs(rDNS->authRRs) << endl;
	cout << "addRRs=" << ntohs(rDNS->addRRs) << endl;
	printf("flag 0x=%x\n", ntohs(rDNS->flags));
	*/
	unsigned short rcode = 0x0F;
	rcode = rcode & ntohs(rDNS->flags);
	//cout << "Rcode= " << rcode << endl;
	int numAns = ntohs(rDNS->answers);
	int numAuthAns = ntohs(rDNS->authRRs);
	int numAddAns = ntohs(rDNS->addRRs);
	DNS::replyMessage = "";
	
	switch (rcode) {

	case 0 :
	{
		DNS::IPlookupSuccess = TRUE;
		
		//THIS IS IMPORTANT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		replyType type = answer;

		DNS::replyMessage +=("Answer(s): \n");	
		char * responseBuffPointer = &recv_buf[DNS::pkt_size];
		RR * rrArray = new RR[numAns+ numAuthAns+ numAddAns];

		for (int i = 0; i < numAns; i++) {
			rrArray->rName = responseBuffPointer;
			DNS::findName(rrArray);
			int lengthOfData = (int)ntohs(rrArray->rrFields->len);
			responseBuffPointer += DNS::nameFieldSize + lengthOfData +10 ;
		}
		/*
		if(numAuthAns >0){
			for (int i = 0; i < numAuthAns; i++) {
				rrArray->rName = responseBuffPointer;
				DNS::findName(rrArray);
				int lengthOfData = (int)ntohs(rrArray->rrFields->len);
				responseBuffPointer += DNS::nameFieldSize + lengthOfData + 10;
			}
		}
		
		if (numAddAns >0) {
			DNS::replyMessage += "Additional Answer(s): \n";
			for (int i = 0; i < numAddAns; i++) {
				rrArray->rName = responseBuffPointer;
				DNS::findName(rrArray);
				int lengthOfData = (int)ntohs(rrArray->rrFields->len);
				responseBuffPointer += DNS::nameFieldSize + lengthOfData + 10;
			}
		}
		*/

		


		delete  rrArray ;
		
		break;
	}
	case 1:
	{
		//printf("FORMAT ERROR!\n");
		DNS::replyMessage += "FORMAT ERROR!\n";
		break;
	}
	case 2:
	{
		//printf("Authoritative DNS Server not found.\n");
		DNS::replyMessage += "Authoritative DNS Server not found.\n";
		DNS::internalNoauthDNScount++;
		break;
	}
	case 3:
	{
		//printf("No DNS entry.\n");
		DNS::replyMessage += "No DNS entry.\n";
		DNS::internalNoDNScount++;
		break;
	}
	case 4:
	{

		//printf("NOT IMPLEMENTED!\n");
		DNS::replyMessage +="NOT IMPLEMENTED!\n" ;
		break;
	}
	case 5:
	{
		//printf("REFUSED!\n");
		DNS::replyMessage += "REFUSED!\n";
		break;
	}
	 default:
		//printf("General Failure!\n");
		DNS::replyMessage += "General Failure!\n";
		break;
	}
	
	//recv_buf[DNS::pkt_size];

}

void DNS::findName(RR * rrArray) {

	// binary 1100 0000 anded with name to detect if entry is a pointer
		struct in_addr aRR;
		string nameParsed = "";
		int timesRun = 0;
		Readbuffer(nameParsed, (rrArray->rName), &timesRun);
		//printf("Name: %s\n", nameParsed.c_str());
		u_short * hello2 = (u_short*)(rrArray->rName);
		//this line is used to increment pointer to correct location for fixed RR
		//rrArray->rName += 2;
		//rrArray->rrFields = (FixedRR *)rrArray->rName+2;
		rrArray->rrFields = (FixedRR *)(rrArray->rName+ DNS::nameFieldSize);
		//cout << "type = " << ntohs(rrArray->rrFields->type) << endl;
		//cout << "class = " << ntohs(rrArray->rrFields->RRclass) << endl;
		//cout << "TTL = " << ntohs(rrArray->rrFields->ttl) << endl;
		//cout << "RDLength = " << ntohs(rrArray->rrFields->len) << endl;
		rrArray->rData = rrArray->rName + 10 + DNS::nameFieldSize; //plus the ten bytes of the middle RR record contents
		string newstring = "";
		if (ntohs(rrArray->rrFields->type) == 1 && DNS::type != authAns) {
			//Type is an A record
			u_long * addressPointer = (u_long *)rrArray->rData;
			aRR.S_un.S_addr = (*addressPointer);
			newstring = inet_ntoa(aRR);
			DNS::replyMessage += nameParsed;
			DNS::replyMessage += " is ";
			DNS::replyMessage += newstring;
			DNS::replyMessage += "\n";
			//printf("%s is %s\n",nameParsed.c_str(),newstring.c_str());

		}
		else if (DNS::type != authAns) {
			//PTR record
			Readbuffer(newstring, (rrArray->rData), &timesRun);
			DNS::replyMessage += nameParsed;
			DNS::replyMessage += " is ";
			DNS::replyMessage += newstring;
			DNS::replyMessage += "\n";
			//DNS::replyMessage += sprintf("%s is %s\n", nameParsed.c_str(), newstring.c_str());
			//printf("%s is %s\n", nameParsed.c_str(),newstring.c_str());

		}


}

void DNS::Readbuffer(string & parsedInput, char * buffer, int * timesRun) {
	// binary 1100 0000 0000 0000 anded with name to detect if entry is a pointer
	if ((*buffer & PTR_NAME) == PTR_NAME) {
		//printf("THIS IS A POINTER VALUE!\n");
		//cast to unsigned short pointer
		u_short * offsetPTR = (u_short *)(buffer);
		//subtract off 1100 0000 0000 0000 to get proper offset from recv_buf
		u_short offset = ntohs(*offsetPTR) - PTR_NAME;
		buffer = recv_buf + offset;
		if (*timesRun == 0) {
			DNS::nameFieldSize = 2;
			*timesRun += 1;
		}
	}


	int length = 0;
	string tempstring = "";
	int numBytes = *(buffer);
	while (numBytes > 0) {
		for (int i = 0; i < numBytes; i++) {
			tempstring += *(buffer + 1 + i);
		}
		tempstring += '.';
		buffer += numBytes + 1;
		length += numBytes;
		numBytes = *(buffer);
	}

	length++;
	if (*timesRun == 0) {
		DNS::nameFieldSize = length;
		*timesRun += 1;
	}
	
	parsedInput = tempstring.substr(0, tempstring.length()-1);
	//printf("READ RESPONSE\n");
}

string DNS::printReply() {
	
	string tempString = DNS::replyMessage.c_str();
	return tempString;

}
int DNS::printTnum() {
	return DNS::count;
}

bool DNS::lookUpSuccess() {
	return DNS::IPlookupSuccess;
}

long DNS::retrieveTransTime() {
	return DNS::lookUpTimer;
}

void DNS::setCounters() {
	DNS::internalNoDNScount = 0;
	DNS::internalNoauthDNScount = 0;
}

int DNS::retrieveNoDNScount() {
	return DNS::internalNoDNScount;
}
int DNS::retrieveNoAuthDNScount() {
	return DNS::internalNoauthDNScount;
}
