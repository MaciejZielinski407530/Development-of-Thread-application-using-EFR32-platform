#include "fun_main.h"
#include "amcom_packets.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <openthread/udp.h>
#include <openthread/thread.h>

#define DEVICE_NAME "Stolica"

AMCOM_IdentifyRequestPayload id_request;
AMCOM_IdentifyResponsePayload id_response[AMCOM_MAX_NEIGHBOR];

AMCOM_RTT_RequestPayload rtt_request;
AMCOM_RTT_ResponsePayload rtt_response;
rtt_stat rtt;

AMCOM_PDR_StartPayload pdr_start;
AMCOM_PDR_StopPayload pdr_stop;
AMCOM_PDR_RequestPayload pdr_request;
AMCOM_PDR_ResponsePayload pdr_response;

AMCOM_RSSI_RequestPayload rssi;


void amcomPacketHandler(const AMCOM_Packet* packet, void* userContext){
    static uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;
    size_t n =0;

    switch (packet->header.type) {
      case AMCOM_IDENTIFY_REQUEST:
              break;
      case AMCOM_IDENTIFY_RESPONSE:
              for(int n = 0; n<AMCOM_MAX_NEIGHBOR ; n++){
                  if(false == id_response[n].active){
                    for(int i = 0; i< AMCOM_MAX_DEVICE_NAME_LEN; i++){
                        id_response[n].deviceName [i] = packet->payload[i];
                     }
                     otCliOutputFormat("CLI name: %s\n", id_response[n].deviceName);

                     for(int i = AMCOM_MAX_DEVICE_NAME_LEN; i< (AMCOM_MAX_DEVICE_NAME_LEN+AMCOM_MAX_ADDRESS_LEN); i++){
                         id_response[n].deviceAddr [i-AMCOM_MAX_DEVICE_NAME_LEN] = packet->payload[i];
                     }
                     otCliOutputFormat("CLI addr: %s\n", id_response[n].deviceAddr);

                     id_response[n].deviceState = packet->payload[AMCOM_MAX_DEVICE_NAME_LEN+AMCOM_MAX_ADDRESS_LEN];
                     otCliOutputFormat("CLI state: %c\n", id_response[n].deviceState);

                     id_response[n].active = true;
                     break;
                  }

                  }
              break;
      case AMCOM_RTT_REQUEST:
              break;
      case AMCOM_RTT_RESPONSE:
              if(rtt_request.packet_number==packet->payload[0]){
                  otNetworkTimeGet(otGetInstance(),&rtt.stop_time);

                  rtt.half_time = packet->payload[1] | packet->payload[2] << 8 | packet->payload[3] << 16 | packet->payload[4] << 24
                      | packet->payload[5] << 32 | packet->payload[6] << 40| packet->payload[7] << 48 | packet->payload[8] << 56;



              }
              break;
      case AMCOM_PDR_START:
              break;
      case AMCOM_PDR_STOP:
              break;
      case AMCOM_PDR_REQUEST:
              break;
      case AMCOM_PDR_RESPONSE:

              for(size_t i = 1; i <= AMCOM_MAX_PDR_TEST; i++){
                  pdr_response.recv_packets[i] = packet->payload[n]|packet->payload[n+1]<<8;
                  n+=2;
                  otCliOutputFormat("Test: %d Recv packets: %d All packets: %d \n", i, pdr_response.recv_packets[i], pdr_start.expect_packets);

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

void identify_req (void){
      static uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
      size_t bytesToSend = 0;
      otNetifAddress *address = otIp6GetUnicastAddresses(otGetInstance());
      otIp6AddressToString(&address->mAddress, id_request.deviceAddr, AMCOM_MAX_ADDRESS_LEN);

      sprintf(id_request.deviceName, DEVICE_NAME);
      bytesToSend = AMCOM_Serialize(AMCOM_IDENTIFY_REQUEST, &id_request, sizeof(id_request), amcomBuf);

      otCliOutputFormat("identify%d\n",bytesToSend);
      if (bytesToSend > 0) {
          UDPsend(amcomBuf, bytesToSend, MULTICAST_ADDR);
        }
}

void pdr_test(void){
        uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
        size_t bytesToSend = 0;
        pdr_start.expect_packets=25;
        pdr_start.expect_tests=1;
        bytesToSend = AMCOM_Serialize(AMCOM_PDR_START, &pdr_start, sizeof(pdr_start), amcomBuf);
        //otCliOutputFormat("identify%d\n",bytesToSend);
             if (bytesToSend > 0) {
                 UDPsend(amcomBuf, bytesToSend, id_response[0].deviceAddr);
                 otCliOutputFormat("I sent start\n");
               }
          pdr_request.test_number = 1;

         for (int i=0; i < pdr_start.expect_packets; i++){
             pdr_request.packet_number = i;
             bytesToSend = AMCOM_Serialize(AMCOM_PDR_REQUEST, &pdr_request, sizeof(pdr_request), amcomBuf);
             if (bytesToSend > 0) {
                 UDPsend(amcomBuf, bytesToSend, id_response[0].deviceAddr);
                 otCliOutputFormat("I sent %d \n",pdr_request.packet_number);
                 }
         }

         pdr_stop.perform_tests = pdr_start.expect_tests;
         bytesToSend = AMCOM_Serialize(AMCOM_PDR_STOP, &pdr_stop, sizeof(pdr_stop), amcomBuf);
         if (bytesToSend > 0) {
               UDPsend(amcomBuf, bytesToSend, id_response[0].deviceAddr);
               otCliOutputFormat("I sent stop\n");
               }
}
void rtt_test(void){
        uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
        size_t bytesToSend = 0;
        if(rtt_request.packet_number == NULL){
            rtt_request.packet_number=0;
        }
        else{
            rtt_request.packet_number++;
        }
        bytesToSend = AMCOM_Serialize(AMCOM_RTT_REQUEST, &rtt_request, sizeof(rtt_request), amcomBuf);
        if (bytesToSend > 0) {
            otNetworkTimeGet(otGetInstance(),&rtt.start_time);
            UDPsend(amcomBuf, bytesToSend, id_response[0].deviceAddr);
            otCliOutputFormat("I sent rtt request\n");
           }
}