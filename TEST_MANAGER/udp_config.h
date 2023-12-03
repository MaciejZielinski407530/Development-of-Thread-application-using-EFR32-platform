#ifndef UDP_CONFIG_H_
#define UDP_CONFIG_H_

#include "app_function.h"
#include "test_packets.h"

/**
 * @brief UDP socket to sending messages initialization.
 *
 * */
void initUdp(void);
 
 /**
 * @brief Function to sending UDP message
 *
 *
 * @param buf Message to send
 * @param PayloadSize Message length
 * @param send_addr Destination IPv6 address
 *
 * */
void UDPsend (uint8_t* buf, size_t PayloadSize, const char* send_addr);
 
 
#endif /* UDP_CONFIG_H_ */