#include "app_function.h"
#include "amcom_packets.h"
#include "udp_config.h"
 
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <sys/time.h>

// Define device name
#define DEVICE_NAME "0x0000"

#define RT rtt_information[packet->payload[0]].rtt_info_time[packet->payload[1]|packet->payload[2]<<8]
 
static AMCOM_IdentifyRequestPayload id_request[AMCOM_MAX_NEIGHBOR];
static AMCOM_IdentifyResponsePayload id_response;
 
static AMCOM_RTT_RequestPayload rtt_request;
static AMCOM_RTT_ResponsePayload rtt_response;

 
static AMCOM_PDR_StartPayload pdr_start;
static AMCOM_PDR_StopPayload pdr_stop;
static AMCOM_PDR_RequestPayload pdr_request;
static AMCOM_PDR_ResponsePayload pdr_response;
 
static AMCOM_RSSI_ResponsePayload rssi_response;

static RTT_INFO rtt_information [AMCOM_MAX_RTT_TEST];

static struct timeval rtt_start, rtt_stop;

//static AMCOM_THROUGHPUT_StartPayload thr_start;
static AMCOM_THROUGHPUT_ResponsePayload thr_response;


// Function to find application IPv6 address
void search_addr(void);

/// AMCOM packet handler handling receive packets 
void amcomPacketHandler(const AMCOM_Packet* packet, void* userContext){
    static uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;
    
 
    switch (packet->header.type) {
      case AMCOM_IDENTIFY_REQUEST:
            size_t n=0;
            char deviceName_temp [AMCOM_MAX_DEVICE_NAME_LEN];
            char deviceAddr_temp [AMCOM_MAX_ADDRESS_LEN];
            char deviceState_temp;
            for(int i = 0; i< AMCOM_MAX_DEVICE_NAME_LEN; i++){
                deviceName_temp [i] = packet->payload[i];
            }
            for(int i = AMCOM_MAX_DEVICE_NAME_LEN; i< (AMCOM_MAX_DEVICE_NAME_LEN+AMCOM_MAX_ADDRESS_LEN); i++){
                deviceAddr_temp[i-AMCOM_MAX_DEVICE_NAME_LEN] = packet->payload[i];
            }
            deviceState_temp = packet->payload[AMCOM_MAX_DEVICE_NAME_LEN+AMCOM_MAX_ADDRESS_LEN];
            
            for(n = 0; n<AMCOM_MAX_NEIGHBOR ; n++){
                if(false == id_request[n].active){
                    sprintf(id_request[n].deviceName, "%s", deviceName_temp);
                    printf("CLI name: %s\n", id_request[n].deviceName);
                    sprintf(id_request[n].deviceAddr, "%s", deviceAddr_temp);
                    printf("CLI addr: %s\n", id_request[n].deviceAddr);
                    id_request[n].deviceState = deviceState_temp;
                    printf("CLI state: %c\n", id_request[n].deviceState);
                    id_request[n].active = true;
                    break;
                }
                if(strcmp(id_request[n].deviceAddr, deviceAddr_temp) == 0 || strcmp(id_request[n].deviceName, deviceName_temp) == 0 ){
                    sprintf(id_request[n].deviceName, "%s", deviceName_temp);
                    printf("CLI name update: %s\n", id_request[n].deviceName);
                    sprintf(id_request[n].deviceAddr, "%s", deviceAddr_temp);
                    printf("CLI addr update: %s\n", id_request[n].deviceAddr);
                    id_request[n].deviceState = deviceState_temp;
                    printf("CLI state update: %c\n", id_request[n].deviceState);
                    id_request[n].active = true;
                    break;
                }
                
            }

            search_addr();
            sprintf(id_response.deviceName, DEVICE_NAME);
            bytesToSend = AMCOM_Serialize(AMCOM_IDENTIFY_RESPONSE, &id_response, sizeof(id_response), amcomBuf);

            if (bytesToSend > 0) {
                UDPsend(amcomBuf, id_request[n].deviceAddr);
            }


              break;
      case AMCOM_IDENTIFY_RESPONSE:
              break;
      case AMCOM_RTT_REQUEST:
              break;
      case AMCOM_RTT_RESPONSE:              
            gettimeofday(&RT.stop, NULL);
            
            printf("STOP Test: %d, Pakiet: %d, Czas: %lld ",packet->payload[0],packet->payload[1]|packet->payload[2]<<8, (long long int)(rtt_information[packet->payload[0]].rtt_info_time[packet->payload[1]|packet->payload[2]<<8].stop.tv_sec * 1000000 + rtt_information[packet->payload[0]].rtt_info_time[packet->payload[1]|packet->payload[2]<<8].stop.tv_usec));
            printf(" Czas rtt : %lld us\n", (long long int) (RT.stop.tv_sec - RT.start.tv_sec)*1000000+(RT.stop.tv_usec-RT.start.tv_usec));

              break;
      case AMCOM_PDR_START:
              break;
      case AMCOM_PDR_STOP:
              break;
      case AMCOM_PDR_REQUEST:
              break;
      case AMCOM_PDR_RESPONSE:
            printf("Odebrane pdr response\n");
            size_t m =0;
            for(size_t i = 0; i <= pdr_stop.perform_tests; i++){
                  pdr_response.recv_packets[i] = packet->payload[m]| packet->payload[m+1]<<8;
                  m+=2;
                  printf("Test: %d Recv packets: %d All packets: %d \n", i+1, pdr_response.recv_packets[i], pdr_start.expect_packets);
 
              }
 
              break;
      case AMCOM_RSSI_REQUEST:
              break;
      case AMCOM_RSSI_RESPONSE:
              printf("Rssi dla test %d pakiet: %d RSSI = %d\n",packet->payload[0], packet->payload[1]|packet->payload[2]<<8, packet->payload[3]);
              break;
      case AMCOM_TON_REQUEST:
              break;
      case AMCOM_TON_RESPONSE:
              printf("Ton = %d [ms]\n",packet->payload[0]|packet->payload[1]<<8);

              break;
      case AMCOM_THROUGHPUT_START:
              break;        
      case AMCOM_THROUGHPUT_REQUEST:
              break;
      case AMCOM_THROUGHPUT_RESPONSE:
              thr_response.recv_packets =  packet->payload[0] | packet->payload[1]<<8;
              printf("Recv pack %d", thr_response.recv_packets);
              break;
      default:
        break;
    }
}

// Function to find application IPv6 address
void search_addr(void){
    struct ifaddrs *ifaddr, *ifa;
 
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(1);
    }
 
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET6 && strcmp(ifa->ifa_name,"enp0s8")==0) {           
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)ifa->ifa_addr;

            if(!(ipv6->sin6_scope_id ==0)){
                continue;
            }

            if (inet_ntop(AF_INET6, &(ipv6->sin6_addr), id_response.deviceAddr, sizeof(id_response.deviceAddr)) != NULL) {
                printf("Adres IPv6 na interfejsie %s: %s\n", ifa->ifa_name, id_response.deviceAddr);
                break;
            }
        }
    }
 
    freeifaddrs(ifaddr);
}
/// Function to trigger PDR test
void pdr_test(const int n_tests, const int n_packets,const int dev_iterator){
        uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
        size_t bytesToSend = 0;
        pdr_start.expect_packets = n_packets;
        pdr_start.expect_tests = n_tests;
        bytesToSend = AMCOM_Serialize(AMCOM_PDR_START, &pdr_start, sizeof(pdr_start), amcomBuf);

             if (bytesToSend > 0) {
                 UDPsend(amcomBuf, id_request[dev_iterator].deviceAddr);
                 printf("I sent start\n");
               }
          
        for(int n = 0; n < n_tests; n++){
            pdr_request.test_number = n;
 
            for (int i=0; i < pdr_start.expect_packets; i++){
                usleep(100000);
                pdr_request.packet_number = i;
                bytesToSend = AMCOM_Serialize(AMCOM_PDR_REQUEST, &pdr_request, sizeof(pdr_request), amcomBuf);
                if (bytesToSend > 0) {
                    UDPsend(amcomBuf, id_request[dev_iterator].deviceAddr);
                    printf("I sent %d \n",pdr_request.packet_number);
                    }
                
                }
        }
        sleep(15);
         pdr_stop.perform_tests = pdr_start.expect_tests;
         bytesToSend = AMCOM_Serialize(AMCOM_PDR_STOP, &pdr_stop, sizeof(pdr_stop), amcomBuf);
         if (bytesToSend > 0) {
               UDPsend(amcomBuf, id_request[dev_iterator].deviceAddr);
               printf("I sent stop\n");
               }
}

/// Function to trigger RTT test
void rtt_test(const int n_tests, const int n_packets,const int dev_iterator){
        uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
        size_t bytesToSend = 0;
        

        for(int i=0; i<n_tests; i++){
            for (int n=0; n<n_packets;n++){           
            usleep(100000);
            rtt_request.test_number=i;
            rtt_request.packet_number=n;
            bytesToSend = AMCOM_Serialize(AMCOM_RTT_REQUEST, &rtt_request, sizeof(rtt_request), amcomBuf);
            if (bytesToSend > 0) { 
                gettimeofday(&rtt_information[i].rtt_info_time[n].start, NULL);  
                printf("START Test: %d, Pakiet: %d, Czas: %lld",i,n, (long long int)(rtt_information[i].rtt_info_time[n].start.tv_sec * 1000000 + rtt_information[i].rtt_info_time[n].start.tv_usec));

                UDPsend(amcomBuf, id_request[dev_iterator].deviceAddr);
                printf("I sent rtt request\n");
            }
           
            }

        }
        
        
}
/// Function to trigger RSSI test
void rssi_test(const int n_tests, const int n_packets,const int dev_iterator){
        AMCOM_RSSI_RequestPayload rssi_request;
        uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
        size_t bytesToSend = 0;
        
        for(int i=0; i<n_tests; i++){
            for (int n=0; n<n_packets;n++){           
            usleep(100000);
            rssi_request.test_number=i;
            rssi_request.packet_number=n;
            bytesToSend = AMCOM_Serialize(AMCOM_RSSI_REQUEST, &rssi_request, sizeof(rssi_request), amcomBuf);
            if (bytesToSend > 0) { 
                UDPsend(amcomBuf, id_request[dev_iterator].deviceAddr);
                printf("I sent rssi request\n");
            }
           
            }

        }
}

/// Function to trigger Throughput test
void thr_test(const int packet_size, const int dev_iterator){
        uint16_t number_of_packets_64 [] = {181, 362, 543, 725, 906, 1087, 1268, 1449, 1630, 1812, 1993};
        uint16_t number_of_packets_128 [] = {94, 188, 282, 376, 470, 464, 658, 752, 846, 940, 1034};
        uint16_t number_of_packets_200 [] = {61, 122, 183, 244, 305, 366, 427, 488, 549, 610, 671};
        size_t num_size = sizeof(number_of_packets_64)/sizeof(number_of_packets_64[0]);

        AMCOM_THROUGHPUT_StartPayload thr_start;

        AMCOM_THROUGHPUT_RequestMinPayload thr_request_min;
        AMCOM_THROUGHPUT_RequestMidPayload thr_request_mid;
        AMCOM_THROUGHPUT_RequestMaxPayload thr_request_max;

        if (packet_size == THROUGHPUT_MIN_PAYLOAD){
            thr_start.expect_packet_size = packet_size;
            for(uint8_t i = 0; i < THROUGHPUT_MIN_PAYLOAD; i++){
                thr_request_min.payload[i] = i;
            }
            send_thr(number_of_packets_64, num_size, &thr_start, &thr_request_min, dev_iterator);
        }
        if (packet_size == THROUGHPUT_MID_PAYLOAD){
            thr_start.expect_packet_size = packet_size;
            for(uint8_t i = 0; i < THROUGHPUT_MID_PAYLOAD; i++){
                thr_request_mid.payload[i] = i;
            }
            send_thr(number_of_packets_128, num_size, &thr_start, &thr_request_mid, dev_iterator);
        }
        if (packet_size == THROUGHPUT_MAX_PAYLOAD){
            thr_start.expect_packet_size = packet_size;
            for(uint8_t i = 0; i < THROUGHPUT_MAX_PAYLOAD; i++){
                thr_request_max.payload[i] = i;
            }
            send_thr(number_of_packets_200, num_size, &thr_start, &thr_request_max, dev_iterator);
        }

}
/// Function to sending throughput packets from thr_test()
void send_thr(uint16_t number_of_packets[], size_t num_size, const void *thr_start, const void *thr_req, const int dev_iterator){
    uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;
    printf("Rozmiar %d\n", sizeof(number_of_packets));

    for(int n = 0; n < num_size; n++){
        
            ((AMCOM_THROUGHPUT_StartPayload*) thr_start)->expect_packets = number_of_packets[n];
            bytesToSend = AMCOM_Serialize(AMCOM_THROUGHPUT_START, thr_start, sizeof(thr_start), amcomBuf);
            
            if (bytesToSend > 0) {
                UDPsend(amcomBuf, id_request[dev_iterator].deviceAddr);
                printf("I sent start\n");
            } 
          
            for (int i=0; i < number_of_packets[n]; i++){
                usleep(1000000*10/number_of_packets[n]);
                
                bytesToSend = AMCOM_Serialize(AMCOM_THROUGHPUT_REQUEST, thr_req, sizeof(thr_req), amcomBuf);
                if (bytesToSend > 0) {
                    UDPsend(amcomBuf, id_request[dev_iterator].deviceAddr);
                    printf("I sent thr %d \n",i);
                    }
               
            }
            sleep(15);
        }
}

/// Function to trigger Ton test
void ton_test(const int dev_iterator){
    uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;
    bytesToSend = AMCOM_Serialize(AMCOM_TON_REQUEST, NULL, 0, amcomBuf);
    if (bytesToSend > 0) {
        UDPsend(amcomBuf, id_request[dev_iterator].deviceAddr);
        printf("I sent ton \n");
    }

}

/// Print Thread device connected to application
void dev_list(void){
    printf("Connected Device List: \n");
    for(int i = 0; i < AMCOM_MAX_NEIGHBOR; i++){
        if(strlen(id_request[i].deviceName) > 0)
            printf("Device name: %s Device Role: %c Device Address: %s\n", id_request[i].deviceName, id_request[i].deviceState, id_request[i].deviceAddr);
    }
}

/// Function finding device number in list of connected device
int find_dev (const char* name){
    for(int i = 0; i < AMCOM_MAX_NEIGHBOR; i++){
        if(strcmp(id_request[i].deviceName, name) == 0)
            return i;
    }
    return -1;
}