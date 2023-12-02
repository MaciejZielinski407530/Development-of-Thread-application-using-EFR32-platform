
#ifndef APP_FUNCTION_H_
#define APP_FUNCTION_H_

#include <stddef.h>

#define MULTICAST_ADDRESS "ff05::1"
/**
 * @brief AMCOM packet handler handling receive packets.
 *
 *
 * @param packet AMCOM packet to handle
 * @param userContext user defined, general purpose context, that will be fed back to the callback function
 *
 * */
void udpPacketHandler(const void* packet, size_t bytesReceived, const char* address);


#endif /* APP_FUNCTION_H_ */
