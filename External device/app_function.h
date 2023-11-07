/*
 * fun_main.h
 *
 *  Created on: 25 paź 2023
 *      Author: komputer
 */
 
#ifndef FUN_MAIN_H_
#define FUN_MAIN_H_
 
#include "amcom.h"
 
 
void amcomPacketHandler(const AMCOM_Packet* packet, void* userContext);

void dev_list(void); 
void identify_req (void);
void pdr_test(const int n_tests, const int n_packets, const int dev_iterator);
void rtt_test(const int n_tests, const int n_packets, const int dev_iterator);
int find_dev (const char* name);
 
 
#endif /* FUN_MAIN_H_ */