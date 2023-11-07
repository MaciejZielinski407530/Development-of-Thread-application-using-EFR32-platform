#include "app_function.h"
#include "amcom_packets.h"
 
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <sys/time.h>

 
#define DEVICE_NAME "Stolica"
#define RT rtt_information[packet->payload[0]].rtt_info_time[packet->payload[1]|packet->payload[2]<<8]
 
static AMCOM_IdentifyRequestPayload id_request[AMCOM_MAX_NEIGHBOR];
static AMCOM_IdentifyResponsePayload id_response;
 
static AMCOM_RTT_RequestPayload rtt_request;
static AMCOM_RTT_ResponsePayload rtt_response;

 
static AMCOM_PDR_StartPayload pdr_start;
static AMCOM_PDR_StopPayload pdr_stop;
static AMCOM_PDR_RequestPayload pdr_request;
static AMCOM_PDR_ResponsePayload pdr_response;
 
static AMCOM_RSSI_RequestPayload rssi;

static RTT_INFO rtt_information [AMCOM_MAX_RTT_TEST];

static struct timeval rtt_start, rtt_stop;
 
void search_addr(void);

 
void amcomPacketHandler(const AMCOM_Packet* packet, void* userContext){
    static uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;
    
 
    switch (packet->header.type) {
      case AMCOM_IDENTIFY_REQUEST:
            size_t n=0;
            for(n = 0; n<AMCOM_MAX_NEIGHBOR ; n++){
                  if(false == id_request[n].active){
                    for(int i = 0; i< AMCOM_MAX_DEVICE_NAME_LEN; i++){
                        id_request[n].deviceName [i] = packet->payload[i];
                     }
                     printf("CLI name: %s\n", id_request[n].deviceName);
 
                     for(int i = AMCOM_MAX_DEVICE_NAME_LEN; i< (AMCOM_MAX_DEVICE_NAME_LEN+AMCOM_MAX_ADDRESS_LEN); i++){
                         id_request[n].deviceAddr [i-AMCOM_MAX_DEVICE_NAME_LEN] = packet->payload[i];
                     }
                     printf("CLI addr: %s\n", id_request[n].deviceAddr);
 
                     id_request[n].deviceState = packet->payload[AMCOM_MAX_DEVICE_NAME_LEN+AMCOM_MAX_ADDRESS_LEN];
                     printf("CLI state: %c\n", id_request[n].deviceState);
 
                     id_request[n].active = true;
                     break;
                  }
            }
            //AMCOM_IdentifyResponsePayload id_response;
            search_addr();
            //id_response.deviceAddr = 
            sprintf(id_response.deviceName, DEVICE_NAME);
            //sprintf(id_response.deviceAddr,"fe80::943b:1475:2b7b:da1");
            bytesToSend = AMCOM_Serialize(AMCOM_IDENTIFY_RESPONSE, &id_response, sizeof(id_response), amcomBuf);
                
            printf("Adres: %s\n", (const char*) id_response.deviceAddr);
            printf("identify%d,  %s\n",bytesToSend, (const char*) amcomBuf);
            if (bytesToSend > 0) {
                UDPsend(amcomBuf, id_request[n].deviceAddr);
            }


              break;
      case AMCOM_IDENTIFY_RESPONSE:
      /*
              for(int n = 0; n<AMCOM_MAX_NEIGHBOR ; n++){
                  if(false == id_response[n].active){
                    for(int i = 0; i< AMCOM_MAX_DEVICE_NAME_LEN; i++){
                        id_response[n].deviceName [i] = packet->payload[i];
                     }
                     printf("CLI name: %s\n", id_response[n].deviceName);
 
                     for(int i = AMCOM_MAX_DEVICE_NAME_LEN; i< (AMCOM_MAX_DEVICE_NAME_LEN+AMCOM_MAX_ADDRESS_LEN); i++){
                         id_response[n].deviceAddr [i-AMCOM_MAX_DEVICE_NAME_LEN] = packet->payload[i];
                     }
                     printf("CLI addr: %s\n", id_response[n].deviceAddr);
 
                     id_response[n].deviceState = packet->payload[AMCOM_MAX_DEVICE_NAME_LEN+AMCOM_MAX_ADDRESS_LEN];
                     printf("CLI state: %c\n", id_response[n].deviceState);
 
                     id_response[n].active = true;
                     break;
                  }
 
                  }
                  */
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
              break;
      default:
        break;
    }
}
 /*
void identify_req (void){
      static uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
      size_t bytesToSend = 0;

      search_addr();
 
      sprintf(id_request.deviceName, DEVICE_NAME);
      bytesToSend = AMCOM_Serialize(AMCOM_IDENTIFY_REQUEST, &id_request, sizeof(id_request), amcomBuf);
 
      printf("identify%d\n",bytesToSend);
      if (bytesToSend > 0) {
          UDPsend(amcomBuf, MULTICAST_ADDR);
        }
}
*/
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
 
void pdr_test(const int n_tests, const int n_packets,const int dev_iterator){
        uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
        size_t bytesToSend = 0;
        pdr_start.expect_packets = n_packets;
        pdr_start.expect_tests = n_tests;
        bytesToSend = AMCOM_Serialize(AMCOM_PDR_START, &pdr_start, sizeof(pdr_start), amcomBuf);

             if (bytesToSend > 0) {
                 UDPsend(amcomBuf, id_request[0].deviceAddr);
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
               UDPsend(amcomBuf, id_request[0].deviceAddr);
               printf("I sent stop\n");
               }
}

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

void dev_list(void){
    printf("Connected Device List: \n");
    for(int i = 0; i < AMCOM_MAX_NEIGHBOR; i++){
        if(strlen(id_request[i].deviceName) > 0)
            printf("Device name: %s Device Role: %c Device Address: %s\n", id_request[i].deviceName, id_request[i].deviceState, id_request[i].deviceAddr);
    }
}

int find_dev (const char* name){
    for(int i = 0; i < AMCOM_MAX_NEIGHBOR; i++){
        if(strcmp(id_request[i].deviceName, name) == 0)
            return i;
    }
    return -1;
}