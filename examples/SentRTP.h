#include "rtpsession.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#ifndef WIN32
	#include <netinet/in.h>
	#include <arpa/inet.h>
#else
	#include <winsock2.h>
#endif // WIN32
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
using namespace jrtplib;
class SentRTP
{
	RTPSession sess;
	uint16_t portbase,destport;
	uint32_t destip;
	std::string ipstr;
	int status,i,num;
	uint8_t payload;
	uint32_t timestamp;
public:
	SentRTP(const std::string &_destip,uint16_t _localPort,uint16_t _destPort,uint8_t payload,uint32_t timestampinc);
	int send(const char *data,int len);
};

