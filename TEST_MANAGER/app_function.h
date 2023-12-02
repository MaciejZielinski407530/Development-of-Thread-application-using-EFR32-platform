 
#ifndef FUN_MAIN_H_
#define FUN_MAIN_H_

#include <stddef.h>
#include <stdint.h>
 
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
 * @brief Print Thread device connected to application
 *
 * */
void dev_list(void); 
 
/**
 * @brief Function to trigger RSSI test
 *
 *
 * @param n_tests Number of tests to execute
 * @param n_packets Number of packets per test
 * @param dev_iterator Device number in list of connected device
 *
 * */
void rssi_test(const uint32_t packet_interval, const uint16_t n_packets,const int dev_iterator);
 
/**
 * @brief Function to trigger PDR test
 *
 *
 * @param n_tests Number of tests to execute
 * @param n_packets Number of packets per test
 * @param dev_iterator Device number in list of connected device
 *
 * */
void pdr_test(const uint32_t packet_interval, const uint16_t n_packets,const int dev_iterator);
 
/**
 * @brief Function to trigger RTT test
 *
 *
 * @param n_tests Number of tests to execute
 * @param n_packets Number of packets per test
 * @param dev_iterator Device number in list of connected device
 *
 * */
void rtt_test(const uint32_t packet_interval, const uint16_t n_packets,const int dev_iterator);
 
/**
 * @brief Function to trigger THROUGHPUT test
 *
 *
 * @param packet_size Size of packet [B]
 * @param dev_iterator Device number in list of connected device
 *
 * */
void thr_test(const uint8_t packet_size, const int dev_iterator);
 
 /**
 * @brief Function to trigger Ton test
 *
 *
 * @param dev_iterator Device number in list of connected device
 *
 * */
void ton_test(const int dev_iterator);

/**
 * @brief Function finding device number in list of connected device
 *
 *
 * @param name Device name
 * 
 * @return Device number in list of connected device
 *
 * */
int find_dev (const char* name);


 
#endif /* FUN_MAIN_H_ */