#ifndef UDP_CONFIG_H_
#define UDP_CONFIG_H_

#include "amcom.h"
#include "app_function.h"


/**
 * @brief Initialization UDP socket to sending messages.
 *
 * */
void initUdp(void);
 
 /**
 * @brief Function to sending UDP message
 *
 *
 * @param buf AMCOM packet to send
 * @param send_addr Destination IPv6 address
 *
 * */
void UDPsend (uint8_t* buf[AMCOM_MAX_PACKET_SIZE], const char* send_addr);
 
 
#endif /* UDP_CONFIG_H_ */