#include "app_function.h"
#include "test_packets.h"
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

// Delay before STOP packets
#define WAIT_FOR_STOP 15
// Number of retransmit packets 
#define RETRANSMIT 3


static TEST_IdentifyResponsePayload id_response[TEST_MAX_NEIGHBOR];
static RTT_PACKET_TIME rtt_packTime [TEST_MAX_RTT_PACKET];
static TEST_THROUGHPUT_ResponsePayload thr_response;
static TEST_PDR_ResponsePayload pdr_response;

/// @brief Function to write results in file
/// @param filename File name
/// @param text Results
void file_writer(const char* filename, const char* text);

/// @brief Function to generate a specific UID for testing
uint8_t generate_uid(void);

/// @brief Function to assembly payload of packet
/// @param packetType  Packet type
/// @param payload     Payload
/// @param payloadSize  Size of payload
/// @param destinationBuffer  Buffer to send  
/// @return Bytes to send
size_t TEST_Serialize(uint8_t packetType, const void* payload, size_t payloadSize, uint8_t* destinationBuffer);

/// AMCOM packet handler handling receive packets 
void udpPacketHandler(const void* packet, size_t bytesReceived, const char* address){
    static uint8_t testBuf[TEST_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;
    char results[TEST_MAX_PACKET_SIZE-1];
 
    switch (*((const uint8_t*)packet)) {     
        case TEST_IDENTIFY_RESPONSE:
        size_t n=0;
            char deviceName_temp [TEST_MAX_DEVICE_NAME_LEN];
            char deviceAddr_temp [INET6_ADDRSTRLEN];
            
            for(int i = 0; i< TEST_MAX_DEVICE_NAME_LEN; i++){
                deviceName_temp [i] = *((const char*)packet+i+1);
            }
   
            for(n = 0; n<TEST_MAX_NEIGHBOR ; n++){
                if(false == id_response[n].active){
                    sprintf(id_response[n].deviceName, "%s", deviceName_temp);
                    printf("CLI name: %s\n", id_response[n].deviceName);
                    strcpy(id_response[n].deviceAddr, address);
                    printf("CLI addr: %s\n", id_response[n].deviceAddr);
                    id_response[n].active = true;
                    break;
                }
                if(strcmp(id_response[n].deviceAddr, address) == 0 && strcmp(id_response[n].deviceName, deviceName_temp) == 0 )
                    break;
            }          
            break;
        case TEST_RTT_RESPONSE:
            static TEST_RTT_ResponsePayload rtt_response;
            
            rtt_response.packet_number = *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN) | *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN+1)<<8;
            
            gettimeofday(&rtt_packTime[rtt_response.packet_number].stop, NULL);
            for(int i = 0; i < TEST_MAX_DEVICE_NAME_LEN; i++){
                rtt_response.deviceName [i] = *((const char*)packet+1+i);
            }
            
            long long int rtt_temp = (rtt_packTime[rtt_response.packet_number].stop.tv_sec - rtt_packTime[rtt_response.packet_number].start.tv_sec)*1000000
            +(rtt_packTime[rtt_response.packet_number].stop.tv_usec-rtt_packTime[rtt_response.packet_number].start.tv_usec);
            
            printf(" Czas rtt : %lld us\n", rtt_temp);
            sprintf(results, "RTT,%d,%lld",rtt_response.packet_number,rtt_temp);
            file_writer(rtt_response.deviceName, results); 
            break;
        case TEST_PDR_RESPONSE:
            if (pdr_response.test_uid == *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN))
                break;

            size_t m =TEST_MAX_DEVICE_NAME_LEN;
            for(int i = 0; i < TEST_MAX_DEVICE_NAME_LEN; i++){
                pdr_response.deviceName [i] = *((const char*)packet+1+i);
            }
            pdr_response.test_uid = *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN);
            pdr_response.recv_packets = *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN+1)| *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN+2)<<8;
            pdr_response.total_packets_number = *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN+3)| *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN+4)<<8;
            printf("Test: %d Recv packets: %d All packets: %d \n", pdr_response.test_uid, pdr_response.recv_packets, pdr_response.total_packets_number); 
            sprintf(results, "PDR,%d,%d,%d",pdr_response.test_uid, pdr_response.recv_packets,pdr_response.total_packets_number);
            file_writer(pdr_response.deviceName, results);
            break;
        case TEST_RSSI_RESPONSE:               
            static TEST_RSSI_ResponsePayload rssi_response;
            for(int i = 0; i < TEST_MAX_DEVICE_NAME_LEN; i++){
                rssi_response.deviceName [i] = *((const char*)packet+1+i);
            }
            rssi_response.packet_number = *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN)|*((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN+1)<<8;
            rssi_response.rssi = *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN+2);
            printf("Rssi dla pakiet: %d RSSI = %d [dBm]\n",rssi_response.packet_number, rssi_response.rssi);
            sprintf(results, "Rssi,%d,%d",rssi_response.packet_number, rssi_response.rssi);
            file_writer(rssi_response.deviceName,results);
            break;
        case TEST_TON_RESPONSE:
            static TEST_TON_ResponsePayload ton_response;
            for(int i = 0; i < TEST_MAX_DEVICE_NAME_LEN; i++){
                ton_response.deviceName [i] = *((const char*)packet+1+i);
            }
            ton_response.time_on = *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN)|*((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN+1)<<8|
                                    *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN+2)<<16|*((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN+3)<<24;
            printf("Ton = %d [ms]\n",ton_response.time_on);
            sprintf(results, "Ton,%d",ton_response.time_on);
            file_writer(ton_response.deviceName, results);
            break;
        case TEST_THROUGHPUT_RESPONSE:
            if(thr_response.test_uid == *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN))
                break;

            for(int i = 0; i < TEST_MAX_DEVICE_NAME_LEN; i++){
                thr_response.deviceName [i] = *((const char*)packet+1+i);
            } 
            thr_response.test_uid = *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN);
            thr_response.recv_packets =  *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN+1) | *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN+2)<<8;
            thr_response.packet_size = *((const uint8_t*)packet+1+TEST_MAX_DEVICE_NAME_LEN+3);
            printf("Recv pack %d, Size: %d B \n", thr_response.recv_packets, thr_response.packet_size);
            sprintf(results, "THR,%d,%d,%d",thr_response.test_uid,thr_response.packet_size,thr_response.recv_packets);
            file_writer(thr_response.deviceName, results);
            break;
        default:
            break;
    }
}

// Function to identify devices in network
void identify(void){
    uint8_t testBuf[TEST_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;
    bytesToSend = TEST_Serialize(TEST_IDENTIFY_REQUEST, NULL, 0, testBuf);
    
    for(int i = 0; i < TEST_MAX_NEIGHBOR ; i++){
        id_response[i].active = false;
    }

    for(int i=0; i<RETRANSMIT;i++){
        if (bytesToSend > 0) {
        UDPsend(testBuf, bytesToSend, MULTICAST_ADDRESS);
        printf("Identification rquest nr. %d\n", i+1);
        }
        sleep(1);
    }
    
}

/// Function to trigger PDR test
void pdr_test(const uint32_t packet_interval, const uint16_t n_packets,const int dev_iterator){      
    static TEST_PDR_RequestPayload pdr_request;
    static TEST_PDR_StopPayload pdr_stop; 
    uint8_t testBuf[TEST_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;
    
    pdr_request.test_uid = generate_uid();
    pdr_request.total_packets_number = n_packets;
    for (int i=0; i < n_packets; i++){
        usleep(packet_interval*1000);
        pdr_request.packet_number = i;
        bytesToSend = TEST_Serialize(TEST_PDR_REQUEST, &pdr_request, sizeof(pdr_request), testBuf);
        if (bytesToSend > 0) {
            UDPsend(testBuf, bytesToSend, id_response[dev_iterator].deviceAddr);
            printf("Sent packet: %d/%d\r",i+1, n_packets);
            fflush(stdout);
        }
                
    }
    printf("\n");
    
    sleep(WAIT_FOR_STOP);
    pdr_stop.test_uid = pdr_request.test_uid;
    printf("UID STOP PDR: %d\n",pdr_stop.test_uid );
    bytesToSend = TEST_Serialize(TEST_PDR_STOP, &pdr_stop, sizeof(pdr_stop), testBuf);
    
    for(int i = 0; i<RETRANSMIT; i++){
        if (bytesToSend > 0) {
            UDPsend(testBuf, bytesToSend, id_response[dev_iterator].deviceAddr);
            printf("PDR stop nr. %d\n", i+1);
        }
        usleep(100000); // 100ms delay
    }
}

/// Function to trigger RTT test
void rtt_test(const uint32_t packet_interval, const uint16_t n_packets,const int dev_iterator){
    static TEST_RTT_RequestPayload rtt_request;
    uint8_t testBuf[TEST_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;
        
    
    for (int n=0; n<n_packets;n++){           
        usleep(packet_interval*1000);
        rtt_request.packet_number=n;
        bytesToSend = TEST_Serialize(TEST_RTT_REQUEST, &rtt_request, sizeof(rtt_request), testBuf);
        if (bytesToSend > 0) {                  
            UDPsend(testBuf, bytesToSend, id_response[dev_iterator].deviceAddr);
            gettimeofday(&rtt_packTime[n].start, NULL); 
            printf("Sent packet: %d/%d\r", n+1, n_packets);
            fflush(stdout);
        }          
    }
    printf("\n");
         
}
/// Function to trigger RSSI test
void rssi_test(const uint32_t packet_interval, const uint16_t n_packets,const int dev_iterator){
    static TEST_RSSI_RequestPayload rssi_request;
    uint8_t testBuf[TEST_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;       
    
    for (int n=0; n<n_packets;n++){           
        usleep(packet_interval*1000);       
        rssi_request.packet_number=n;
        bytesToSend = TEST_Serialize(TEST_RSSI_REQUEST, &rssi_request, sizeof(rssi_request), testBuf);
        if (bytesToSend > 0) { 
            UDPsend(testBuf, bytesToSend, id_response[dev_iterator].deviceAddr);
            printf("Sent packet: %d/%d\r",n+1, n_packets);
            fflush(stdout);
        }   
    }
    printf("\n");   
}

void thr_test(const uint8_t packet_size, const int dev_iterator){
    static TEST_THROUGHPUT_StopPayload thr_stop; 
    uint8_t testBuf[TEST_MAX_PACKET_SIZE];
    uint8_t thr_request[THROUGHPUT_PAYLOAD];
    size_t bytesToSend = 0;
    
    thr_request[0] = generate_uid();   
    for(uint8_t i = 1; i >packet_size-2; i--){  // (-2B for TYPE, UID)
            thr_request[i] = i;
        }
    for (int i=0; i < 1000; i++){
        usleep(10000); // At least 10s 
        
        bytesToSend = TEST_Serialize(TEST_THROUGHPUT_REQUEST, thr_request, packet_size-1, testBuf);
        if (bytesToSend > 0) {
            UDPsend(testBuf, bytesToSend, id_response[dev_iterator].deviceAddr);
            printf("Sent packet: %d\r",i+1);
            fflush(stdout);           
        }           
    }
    printf("\n");
    sleep(WAIT_FOR_STOP);

    thr_stop.test_uid = thr_request[0];
    thr_stop.packet_size = packet_size;
    bytesToSend = TEST_Serialize(TEST_THROUGHPUT_STOP, &thr_stop, sizeof(thr_stop), testBuf);
    for(int i = 0; i<RETRANSMIT; i++){
        if (bytesToSend > 0) {
        UDPsend(testBuf, bytesToSend, id_response[dev_iterator].deviceAddr);
        printf("THR stop nr. %d\n", i+1);
        }
        usleep(100000); // 100ms delay
    }
    
}

/// Function to trigger Ton test
void ton_test(const int dev_iterator){
    uint8_t testBuf[TEST_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;
    bytesToSend = TEST_Serialize(TEST_TON_REQUEST, NULL, 0, testBuf);
    if (bytesToSend > 0) {
        UDPsend(testBuf, bytesToSend, id_response[dev_iterator].deviceAddr);
        printf("Ton sent \n");
    }

}

/// Function finding device number in list of connected device
int find_dev (const char* name){
    for(int i = 0; i < TEST_MAX_NEIGHBOR; i++){
        if(strcmp(id_response[i].deviceName, name) == 0)
            return i;
    }
    return -1;
}

/// Print Thread device connected to application
void dev_list(void){
    printf("Connected Device List: \n");
    for(int i = 0; i < TEST_MAX_NEIGHBOR; i++){
        if(id_response[i].active == true)
            printf("Device name: %s Device Address: %s\n", id_response[i].deviceName, id_response[i].deviceAddr);
    }
}

void file_writer(const char* filename, const char* text){
    char file_name[TEST_MAX_DEVICE_NAME_LEN+4];
    FILE *file;
    sprintf(file_name,"%s.csv",filename);
    file = fopen(file_name, "a");
    fseek(file, 0, SEEK_END);
    fprintf(file, "%s\n", text );
    fclose(file);
}

uint8_t generate_uid(void){
    return rand()%256;
}

size_t TEST_Serialize(uint8_t packetType, const void* payload, size_t payloadSize, uint8_t* destinationBuffer){
    size_t i = 0;
    destinationBuffer[i] = packetType;
    if(payload == NULL || payloadSize == 0){
        return 1;
    }
    for (i = 1; i < (payloadSize + 1); i++) {  
        destinationBuffer[i] = *((const char*)payload + i - 1);
    }
    return i;
}

