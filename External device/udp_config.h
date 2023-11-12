#ifndef UDP_CONFIG_H_
#define UDP_CONFIG_H_

#include "amcom.h"
#include "app_function.h"


void initUdp(void);
void UDPsend (uint8_t* buf[AMCOM_MAX_PACKET_SIZE], const char* send_addr);
 
 
#endif /* UDP_CONFIG_H_ */