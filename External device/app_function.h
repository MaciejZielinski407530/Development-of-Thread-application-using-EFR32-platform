/*
 * fun_main.h
 *
 *  Created on: 25 paź 2023
 *      Author: komputer
 */
 
#ifndef FUN_MAIN_H_
#define FUN_MAIN_H_
 
#include "amcom.h"
 
 
typedef struct{
  uint64_t start_time;
  uint64_t half_time;
  uint64_t stop_time;
}rtt_stat;
 
 
void amcomPacketHandler(const AMCOM_Packet* packet, void* userContext);
 
void identify_req (void);
void pdr_test(void);
void rtt_test(void);
 
 
#endif /* FUN_MAIN_H_ */