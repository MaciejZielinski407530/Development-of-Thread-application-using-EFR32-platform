
#ifndef APP_FUNCTION_H_
#define APP_FUNCTION_H_

#include <stddef.h>

#define MULTICAST_ADDRESS "ff05::1"
/**
 * @brief UDP packet handler handling receive packets.
 *
 *
 * @param packet  to handle
 * @param bytesReceived Length received packet
 * @param address Source IPv6 address
 *
 * */
void udpPacketHandler(const void* packet, size_t bytesReceived, const char* address);


#endif /* APP_FUNCTION_H_ */
