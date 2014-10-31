#include "SentRTP.h"

//
// This function checks if there was a RTP error. If so, it displays an error
// message and exists.
//

static void checkerror(int rtperr)
{
	if (rtperr < 0)
	{
		std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl;
		exit(-1);
	}
}

//
// The main routine
//

SentRTP::SentRTP(const std::string &_destip,uint16_t _localPort,uint16_t _destPort,uint8_t _payload,uint32_t _timestamp)
	:ipstr(_destip),portbase(_localPort),destport(_destPort),payload(_payload),timestamp(_timestamp)
{
	destip = inet_addr(ipstr.c_str());
	if (destip == INADDR_NONE)
	{
		std::cerr << "Bad IP address specified" << std::endl;
		return;
	}
	destip = ntohl(destip);

	RTPUDPv4TransmissionParams transparams;
	RTPSessionParams sessparams;
	
	// IMPORTANT: The local timestamp unit MUST be set, otherwise
	//            RTCP Sender Report info will be calculated wrong
	// In this case, we'll be sending 10 samples each second, so we'll
	// put the timestamp unit to (1.0/10.0)
	sessparams.SetOwnTimestampUnit(1.0/10.0);		
	
	sessparams.SetAcceptOwnPackets(true);
	transparams.SetPortbase(portbase);
	status = sess.Create(sessparams,&transparams);	
	checkerror(status);

	RTPIPv4Address addr(destip,destport);
	
	status = sess.AddDestination(addr);
	checkerror(status);
}
int SentRTP::send(const char * data, int len)
{
	return sess.SendPacket(data,len,payload,false,timestamp);
}

