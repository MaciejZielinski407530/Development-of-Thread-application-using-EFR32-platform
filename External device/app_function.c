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

//#define RT rtt_information[packet->payload[0]].rtt_info_time[packet->payload[1]|packet->payload[2]<<8]
 #define RT rtt_information[packet->payload[AMCOM_MAX_DEVICE_NAME_LEN]].rtt_info_time[packet->payload[AMCOM_MAX_DEVICE_NAME_LEN+1]|packet->payload[AMCOM_MAX_DEVICE_NAME_LEN+2]<<8]
void send_thr(uint16_t number_of_packets[], size_t num_size, const void *thr_start, const void *thr_req, const int dev_iterator);
void file_writer(const char* filename, const char* text);
static AMCOM_IdentifyRequestPayload id_request[AMCOM_MAX_NEIGHBOR];
static AMCOM_IdentifyResponsePayload id_response;
 
static AMCOM_PDR_StartPayload pdr_start;
static AMCOM_PDR_StopPayload pdr_stop;
static AMCOM_PDR_ResponsePayload pdr_response;

static RTT_INFO rtt_information [AMCOM_MAX_RTT_TEST];

static struct timeval rtt_start, rtt_stop;

// Function to find application IPv6 address
void search_addr(void);

/// AMCOM packet handler handling receive packets 
void amcomPacketHandler(const AMCOM_Packet* packet, void* userContext){
    static uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;
    char results[60];
 
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
                printf("Identification response sent\n");
            }


              break;
      case AMCOM_IDENTIFY_RESPONSE:
              break;
      case AMCOM_RTT_REQUEST:
              break;
      case AMCOM_RTT_RESPONSE:
            static AMCOM_RTT_ResponsePayload rtt_response;

            gettimeofday(&RT.stop, NULL);
            for(int i = 0; i < AMCOM_MAX_DEVICE_NAME_LEN; i++){
                rtt_response.deviceName [i] = packet->payload[i];
            }
            rtt_response.test_number = packet->payload[AMCOM_MAX_DEVICE_NAME_LEN];
            rtt_response.packet_number = packet->payload[AMCOM_MAX_DEVICE_NAME_LEN+1]|packet->payload[AMCOM_MAX_DEVICE_NAME_LEN+2]<<8;
            //printf("STOP Test: %d, Pakiet: %d, Czas: %lld ",packet->payload[0],packet->payload[1]|packet->payload[2]<<8, (long long int)(rtt_information[packet->payload[0]].rtt_info_time[packet->payload[1]|packet->payload[2]<<8].stop.tv_sec * 1000000 + rtt_information[packet->payload[0]].rtt_info_time[packet->payload[1]|packet->payload[2]<<8].stop.tv_usec));
            printf(" Czas rtt : %lld us\n", (long long int) (RT.stop.tv_sec - RT.start.tv_sec)*1000000+(RT.stop.tv_usec-RT.start.tv_usec));
            sprintf(results, "RTT,%d,%d,%lld",rtt_response.test_number,rtt_response.packet_number,(long long int) (RT.stop.tv_sec - RT.start.tv_sec)*1000000+(RT.stop.tv_usec-RT.start.tv_usec));
            file_writer(rtt_response.deviceName, results); 
              break;
      case AMCOM_PDR_START:
              break;
      case AMCOM_PDR_STOP:
              break;
      case AMCOM_PDR_REQUEST:
              break;
      case AMCOM_PDR_RESPONSE:
            size_t m =AMCOM_MAX_DEVICE_NAME_LEN;
            for(int i = 0; i < AMCOM_MAX_DEVICE_NAME_LEN; i++){
                pdr_response.deviceName [i] = packet->payload[i];
            }
            for(size_t i = AMCOM_MAX_DEVICE_NAME_LEN; i < (pdr_stop.perform_tests+AMCOM_MAX_DEVICE_NAME_LEN); i++){
                  pdr_response.recv_packets[i] = packet->payload[m]| packet->payload[m+1]<<8;
                  m+=2;
                  printf("Test: %ld Recv packets: %d All packets: %d \n", (i+1-AMCOM_MAX_DEVICE_NAME_LEN), pdr_response.recv_packets[i], pdr_start.expect_packets); 
                  sprintf(results, "PDR,%ld,%d,%d",(i+1-AMCOM_MAX_DEVICE_NAME_LEN), pdr_response.recv_packets[i], pdr_start.expect_packets);
                  file_writer(pdr_response.deviceName, results);    
            }
 
              break;
      case AMCOM_RSSI_REQUEST:
              break;
      case AMCOM_RSSI_RESPONSE:               
              static AMCOM_RSSI_ResponsePayload rssi_response;
              for(int i = 0; i < AMCOM_MAX_DEVICE_NAME_LEN; i++){
                rssi_response.deviceName [i] = packet->payload[i];
              }
              rssi_response.test_number = packet->payload[AMCOM_MAX_DEVICE_NAME_LEN];
              rssi_response.packet_number = packet->payload[AMCOM_MAX_DEVICE_NAME_LEN+1]|packet->payload[AMCOM_MAX_DEVICE_NAME_LEN+2]<<8;
              rssi_response.rssi = packet->payload[AMCOM_MAX_DEVICE_NAME_LEN+3];
              printf("Rssi dla test %d pakiet: %d RSSI = %d [dBm]\n",rssi_response.test_number, rssi_response.packet_number, rssi_response.rssi);
              sprintf(results, "Rssi,%d,%d,%d",rssi_response.test_number, rssi_response.packet_number, rssi_response.rssi);
              file_writer(rssi_response.deviceName,results);
              break;
      case AMCOM_TON_REQUEST:
              break;
      case AMCOM_TON_RESPONSE:
              static AMCOM_TON_ResponsePayload ton_response;
              for(int i = 0; i < AMCOM_MAX_DEVICE_NAME_LEN; i++){
                ton_response.deviceName [i] = packet->payload[i];
              }
              ton_response.time_on = packet->payload[AMCOM_MAX_DEVICE_NAME_LEN]|packet->payload[AMCOM_MAX_DEVICE_NAME_LEN+1]<<8;
              printf("Ton = %d [ms]\n",ton_response.time_on);
              sprintf(results, "Ton,%d",ton_response.time_on);
              file_writer(ton_response.deviceName, results);
              break;
      case AMCOM_THROUGHPUT_START:
              break;        
      case AMCOM_THROUGHPUT_REQUEST:
              break;
      case AMCOM_THROUGHPUT_RESPONSE:
            static AMCOM_THROUGHPUT_ResponsePayload thr_response;
            for(int i = 0; i < AMCOM_MAX_DEVICE_NAME_LEN; i++){
                thr_response.deviceName [i] = packet->payload[i];
            }
            thr_response.recv_packets =  packet->payload[AMCOM_MAX_DEVICE_NAME_LEN] | packet->payload[AMCOM_MAX_DEVICE_NAME_LEN+1]<<8;
            printf("Recv pack %d\n", thr_response.recv_packets);
            sprintf(results, "THR,%d",thr_response.recv_packets);
            file_writer(ton_response.deviceName, results);
            break;
      case AMCOM_THROUGHPUT_STOP:
            break;
      default:
        break;
    }
}

void file_writer(const char* filename, const char* text){
    char file_name[AMCOM_MAX_DEVICE_NAME_LEN+4];
    FILE *file;
    sprintf(file_name,"%s.csv",filename);
    file = fopen(file_name, "a");
    fseek(file, 0, SEEK_END);
    fprintf(file, "%s\n", text );
    fclose(file);
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
        
        static AMCOM_PDR_RequestPayload pdr_request;
        
        uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
        size_t bytesToSend = 0;
        pdr_start.expect_packets = n_packets;
        pdr_start.expect_tests = n_tests;
        bytesToSend = AMCOM_Serialize(AMCOM_PDR_START, &pdr_start, sizeof(pdr_start), amcomBuf);

             if (bytesToSend > 0) {
                 UDPsend(amcomBuf, id_request[dev_iterator].deviceAddr);
                 printf("PDR start\n");
               }
          
        for(int n = 0; n < n_tests; n++){
            pdr_request.test_number = n;
 
            for (int i=0; i < pdr_start.expect_packets; i++){
                usleep(100000);
                pdr_request.packet_number = i;
                bytesToSend = AMCOM_Serialize(AMCOM_PDR_REQUEST, &pdr_request, sizeof(pdr_request), amcomBuf);
                if (bytesToSend > 0) {
                    UDPsend(amcomBuf, id_request[dev_iterator].deviceAddr);
                    printf("Sent: Test: %d/%d, Packet: %d/%d\r", n+1, n_tests, i+1, n_packets);
                    fflush(stdout);
                    //printf("I sent %d \n",pdr_request.packet_number);
                    }
                
            }
            printf("\n");
        }
        sleep(15);
         pdr_stop.perform_tests = pdr_start.expect_tests;
         bytesToSend = AMCOM_Serialize(AMCOM_PDR_STOP, &pdr_stop, sizeof(pdr_stop), amcomBuf);
         if (bytesToSend > 0) {
               UDPsend(amcomBuf, id_request[dev_iterator].deviceAddr);
               printf("PDR stop\n");
               }
}

/// Function to trigger RTT test
void rtt_test(const int n_tests, const int n_packets,const int dev_iterator){
        static AMCOM_RTT_RequestPayload rtt_request;
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
                //printf("START Test: %d, Pakiet: %d, Czas: %lld",i,n, (long long int)(rtt_information[i].rtt_info_time[n].start.tv_sec * 1000000 + rtt_information[i].rtt_info_time[n].start.tv_usec));
                UDPsend(amcomBuf, id_request[dev_iterator].deviceAddr);
                printf("Sent: Test: %d/%d, Packet: %d/%d\r", i+1, n_tests, n+1, n_packets);
                fflush(stdout);
            }
           
            }
            printf("\n");
        }
        
        
}
/// Function to trigger RSSI test
void rssi_test(const int n_tests, const int n_packets,const int dev_iterator){
        static AMCOM_RSSI_RequestPayload rssi_request;
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
                printf("Sent: Test: %d/%d, Packet: %d/%d\r", i+1, n_tests, n+1, n_packets);
                fflush(stdout);
            }
           
            }
            printf("\n");
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

    for(int n = 0; n < num_size; n++){
        
            ((AMCOM_THROUGHPUT_StartPayload*) thr_start)->expect_packets = number_of_packets[n];
            bytesToSend = AMCOM_Serialize(AMCOM_THROUGHPUT_START, thr_start, sizeof(thr_start), amcomBuf);
            
            if (bytesToSend > 0) {
                UDPsend(amcomBuf, id_request[dev_iterator].deviceAddr);
                printf("Throughput start\n");
            } 
          
            for (int i=0; i < number_of_packets[n]; i++){
                usleep(1000000*10/number_of_packets[n]);
                
                bytesToSend = AMCOM_Serialize(AMCOM_THROUGHPUT_REQUEST, thr_req, sizeof(thr_req), amcomBuf);
                if (bytesToSend > 0) {
                    UDPsend(amcomBuf, id_request[dev_iterator].deviceAddr);
                    printf("Sent: Test: %d/%ld, Packet: %d/%d\r", n+1, num_size, i+1, number_of_packets[n]);
                    fflush(stdout);
                    }
               
            }
            sleep(15);
            printf("Po oczekiwaniu\n");
            bytesToSend = AMCOM_Serialize(AMCOM_THROUGHPUT_STOP, NULL, 0, amcomBuf);
            
            if (bytesToSend > 0) {
                UDPsend(amcomBuf, id_request[dev_iterator].deviceAddr);
                printf("Throughput stop\n");
            } 
        }
}

/// Function to trigger Ton test
void ton_test(const int dev_iterator){
    uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;
    bytesToSend = AMCOM_Serialize(AMCOM_TON_REQUEST, NULL, 0, amcomBuf);
    if (bytesToSend > 0) {
        UDPsend(amcomBuf, id_request[dev_iterator].deviceAddr);
        printf("Ton sent \n");
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