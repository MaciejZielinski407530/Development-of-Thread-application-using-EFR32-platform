#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <openthread/udp.h>
#include <openthread/cli.h>
#include <openthread/thread.h>

#include "app_function.h"
#include "app.h"
#include "test_packets.h"

otInstance *otGetInstance(void);
int8_t get_rssi(void);
void UDPsend (uint8_t buf[TEST_MAX_PACKET_SIZE], size_t bytesToSend, const char* send_addr);
uint32_t getJoinTime(void);
uint32_t getSysTick_time(void);
size_t TEST_Serialize(uint8_t packetType, const void* payload, size_t payloadSize, uint8_t* destinationBuffer);


static TEST_IdentifyResponsePayload id_response;

static TEST_RTT_ResponsePayload rtt_response;

static TEST_PDR_ResponsePayload pdr_response;

static TEST_RSSI_ResponsePayload rssi_response;

static TEST_TON_ResponsePayload ton_response;

static TEST_THROUGHPUT_ResponsePayload thr_response;


bool  identified = false;
uint32_t   timestamp = 0;


// Handling AMCOM Packets
void udpPacketHandler(const void* packet, size_t bytesReceived, const char* address){
    static uint8_t amcomBuf[TEST_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;

    switch (*((const uint8_t*)packet)) {
      case TEST_IDENTIFY_REQUEST:
        sprintf(id_response.deviceName,"0x%04x", otThreadGetRloc16(otGetInstance()));
        bytesToSend = TEST_Serialize(TEST_IDENTIFY_RESPONSE, &id_response, sizeof(id_response), amcomBuf);

        int backoff = rand()%10;
        timestamp = getSysTick_time();
        while(1){
            if(timestamp+(backoff*100)<getSysTick_time()){
                break;
            }
        }
          otCliOutputFormat("identify%d\n",bytesToSend);
          if (bytesToSend > 0) {
              UDPsend(amcomBuf, bytesToSend, address);
              identified = true;     // Identification FLAG
            }

        break;
      case TEST_RTT_REQUEST:
              otCliOutputFormat("Otrzymalem rtt req\n");

              sprintf(rtt_response.deviceName,"%s", id_response.deviceName);
              rtt_response.packet_nunmber = *((const uint8_t*)packet+1)|*((const uint8_t*)packet+1+1)<<8;
              bytesToSend = TEST_Serialize(TEST_RTT_RESPONSE, &rtt_response, sizeof(rtt_response), amcomBuf);

              // Sending RTT Response
              if (bytesToSend > 0) {
                  UDPsend(amcomBuf, bytesToSend, address);
                  otCliOutputFormat("Wysylam rtt response\n");
              }

              break;
      case TEST_PDR_STOP:
            if(*((const uint8_t*)packet+1) != pdr_response.test_uid){
                otCliOutputFormat("PDR STOP RECV uid: %d THR uid: %d\n",*((const uint8_t*)packet+1),pdr_response.test_uid);
                pdr_response.recv_packets = 0;
            }
            sprintf(pdr_response.deviceName,"%s", id_response.deviceName);
            bytesToSend = TEST_Serialize(TEST_PDR_RESPONSE, &pdr_response, sizeof(pdr_response), amcomBuf);

            otCliOutputFormat("UID: %d Otrzymalem %d pakietow z %d",pdr_response.test_uid, pdr_response.recv_packets, pdr_response.total_packets_number );

            // Sending PDR Response
            if (bytesToSend > 0) {
                UDPsend(amcomBuf, bytesToSend,  address);
                otCliOutputFormat("Wysylam pdr response");
            }
              break;
      case TEST_PDR_REQUEST:
              if(*((const uint8_t*)packet+1) != pdr_response.test_uid){
                  pdr_response.test_uid = *((const uint8_t*)packet+1);
                  otCliOutputFormat("PDR REQ Recv uid: %d THR uid: %d\n",*((const uint8_t*)packet+1),pdr_response.test_uid);
                  pdr_response.recv_packets = 0;
                  pdr_response.total_packets_number = *((const uint8_t*)packet+1+3)|*((const uint8_t*)packet+1+4)<<8;
              }

              pdr_response.recv_packets++;
              otCliOutputFormat("UID: %d %d %d %d \n",*((const uint8_t*)packet+1),pdr_response.test_uid,pdr_response.recv_packets, *((const uint8_t*)packet+1+1)|*((const uint8_t*)packet+1+2)<<8);
              break;
      case TEST_RSSI_REQUEST:
              sprintf(rssi_response.deviceName,"%s", id_response.deviceName);
              rssi_response.packet_number = *((const uint8_t*)packet+1)|*((const uint8_t*)packet+1+1)<<8;
              rssi_response.rssi = get_rssi();
              bytesToSend = TEST_Serialize(TEST_RSSI_RESPONSE, &rssi_response, sizeof(rssi_response), amcomBuf);

              // Sending RSSI Response
              if (bytesToSend > 0) {
                    UDPsend(amcomBuf, bytesToSend, address);
                    otCliOutputFormat("Wysylam rssi response");
               }

              break;
      case TEST_TON_REQUEST:
              sprintf(ton_response.deviceName,"%s", id_response.deviceName);
              ton_response.time_on = getJoinTime();
              bytesToSend = TEST_Serialize(TEST_TON_RESPONSE, &ton_response, sizeof(ton_response), amcomBuf);

              // Sending Ton Response
              if (bytesToSend > 0) {
                  UDPsend(amcomBuf, bytesToSend, address);
                  otCliOutputFormat("Wysylam ton response %d vs %d",ton_response.time_on, getJoinTime());
              }

              break;
      case TEST_THROUGHPUT_REQUEST:
               if(*((const uint8_t*)packet+1) != thr_response.test_uid){
                  thr_response.test_uid = *((const uint8_t*)packet+1);
                  thr_response.recv_packets = 0;
                  timestamp = getSysTick_time();
               }
               if(getSysTick_time()-timestamp <= 10000){
                  thr_response.recv_packets++;
                  otCliOutputFormat("Recv %d", thr_response.recv_packets);
               }
              break;
      case TEST_THROUGHPUT_STOP:
              if(*((const uint8_t*)packet+1) != thr_response.test_uid){
                  thr_response.recv_packets = 0;
              }
              sprintf(thr_response.deviceName,"%s", id_response.deviceName);
              thr_response.packet_size = *((const uint8_t*)packet+2);
              bytesToSend = TEST_Serialize(TEST_THROUGHPUT_RESPONSE, &thr_response, sizeof(thr_response), amcomBuf);

              if (bytesToSend > 0) {
                  UDPsend(amcomBuf, bytesToSend, address);
                  otCliOutputFormat("Wysylam thr response %d recv packet size: %d\n",thr_response.recv_packets, thr_response.packet_size );
             }
              break;
      default:
        break;
    }
}

size_t TEST_Serialize(uint8_t packetType, const void* payload, size_t payloadSize, uint8_t* destinationBuffer) {
  size_t i =0;
  destinationBuffer[i] = packetType;
  if(payload == NULL || payloadSize == 0)
    return 1;
  for (i = 1; i < (payloadSize + 1); i++) {
       destinationBuffer[i] = *((const char*)payload + i - 1);
  }
  return i;
}

