
#ifndef APP_FUNCTION_H_
#define APP_FUNCTION_H_

#include <stddef.h>
#include <stdbool.h>


/**
 * @brief AMCOM packet handler handling receive packets.
 *
 *
 * @param packet AMCOM packet to handle
 * @param userContext user defined, general purpose context, that will be fed back to the callback function
 *
 * */
void udpPacketHandler(const void* packet, size_t bytesReceived);

/**
 * @brief Function that determines the state of the device.
 *
 * @return the state of the device
 *
 * */
char get_dev_state(void);

/**
 * @brief Function that provide identification on main application.
 *
 * Function send device RLOC16, device address, device state to main application
 *
 * */
void identify_request (void);

/**
 * @brief Function that checks the differences between the sent and current identification data.
 *
 * @return TRUE, if all is the same, FALSE otherwise
 *
 * */
bool send_dev_info_correct(void);

#endif /* APP_FUNCTION_H_ */
