/*
 * fun_main.h
 *
 *  Created on: 25 paź 2023
 *      Author: komputer
 */

#ifndef APP_FUNCTION_H_
#define APP_FUNCTION_H_

#include "amcom.h"


void amcomPacketHandler(const AMCOM_Packet* packet, void* userContext);

char get_dev_state(void);
void identify_request (void);

#endif /* APP_FUNCTION_H_ */
