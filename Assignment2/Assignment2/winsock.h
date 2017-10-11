/*	winsock.h 
	CPS 470 Sample Code

	Define a class that handles socket APIs
*/


#pragma once 
#include <winsock2.h>
#include <stdio.h>
#include <time.h>

class Winsock {

public:
	SOCKET	OpenSocket (void); 

};