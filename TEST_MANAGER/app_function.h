 
#ifndef APP_FUNCTION_H_
#define APP_FUNCTION_H_

#include <stddef.h>
#include <stdint.h>
 
 
/**
 * @brief UDP packet handler handling receive packets.
 *
 *
 * @param packet UDP packet to handle
 * @param bytesReceived Length received packet
 * @param address Source IPv6 address 
 *
 * */
void udpPacketHandler(const void* packet, size_t bytesReceived, const char* address);
 
/**
 * @brief Function to identify devices in network
 *
 * */
void identify(void);

/**
 * @brief Function to trigger PDR test
 *
 *
 * @param packet_interval Delay between each packet
 * @param n_packets Number of packets
 * @param dev_iterator Device number on the list of connected device
 *
 * */
void pdr_test(const uint32_t packet_interval, const uint16_t n_packets,const int dev_iterator);
 
/**
 * @brief Function to trigger RTT test
 *
 *
 * @param packet_interval Delay between each packet
 * @param n_packets Number of packets
 * @param dev_iterator Device number on the list of connected device
 *
 * */
void rtt_test(const uint32_t packet_interval, const uint16_t n_packets,const int dev_iterator);
 
/**
 * @brief Function to trigger RSSI test
 *
 *
 * @param packet_interval Delay between each packet
 * @param n_packets Number of packets
 * @param dev_iterator Device number on the list of connected device
 *
 * */
void rssi_test(const uint32_t packet_interval, const uint16_t n_packets,const int dev_iterator);
 
/**
 * @brief Function to trigger THROUGHPUT test
 *
 *
 * @param packet_size Size of packet [B]
 * @param dev_iterator Device number on the list of connected device
 *
 * */
void thr_test(const uint8_t packet_size, const int dev_iterator);
 
 /**
 * @brief Function to trigger Ton test
 *
 *
 * @param dev_iterator Device number on the list of connected device
 *
 * */
void ton_test(const int dev_iterator);

/**
 * @brief Print Thread device connected to application
 *
 * */
void dev_list(void); 
 
 /**
 * @brief Function to check if device is listed
 *
 *
 * @param name Device name
 *
 * @return Position on connected devices list
 * */
int find_dev (const char* name);


 
#endif /* APP_FUNCTION_H_ */