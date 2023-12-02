#ifndef UDP_CONFIG_H_
#define UDP_CONFIG_H_

#include "app_function.h"
#include "amcom_packets.h"

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
void UDPsend (uint8_t* buf, size_t PayloadSize, const char* send_addr);
 
 
#endif /* UDP_CONFIG_H_ */