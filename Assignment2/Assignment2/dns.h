/*	dns.h
	CPS 470 Sample Code
*/
#pragma once
#include <Iphlpapi.h>


class DNS {
public:
	FixedDNSheader * DNS::CreateDNSHeader(void);
	void printDNSServer (void);
	u_short ID;
	u_short flags;
	u_short questions;
private:

};