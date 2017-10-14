/*	dns.h
	CPS 470 Sample Code
*/

#include "headers.h"
#include "dns.h"
#include "lookup.h"

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
		printf( "\t%s\n", FixedInfo->DnsServerList.IpAddress.String);

		pIPAddr = FixedInfo->DnsServerList.Next;
		while ( pIPAddr ) {
			printf( "\t%s\n", pIPAddr ->IpAddress.String);
			pIPAddr = pIPAddr ->Next;
		}
	}

	GlobalFree (FixedInfo);
}

FixedDNSheader * DNS::CreateDNSHeader(char & pkt) {


	string host = "www.yahoo.com";
	//	string host = "193.73.238.131.in-addr.arpa"; 
	//	string host = "7.74.238.131.in-addr.arpa"; 
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

	return dHDR;
}
