#include "app_function.h"
#include "amcom_packets.h"
#include "app.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <openthread/udp.h>
#include <openthread/cli.h>
#include <openthread/thread.h>

otInstance *otGetInstance(void);
int8_t get_rssi(void);
void UDPsend (uint8_t buf[TEST_MAX_PACKET_SIZE], size_t bytesToSend, const char* send_addr);
uint32_t getJoinTime(void);
uint32_t getSysTick_time(void);
size_t TEST_Serialize(uint8_t packetType, const void* payload, size_t payloadSize, uint8_t* destinationBuffer);
static TEST_IdentifyRequestPayload id_request;
static TEST_IdentifyResponsePayload id_response;

static TEST_RSSI_ResponsePayload rssi_response;

//static AMCOM_PDR_StartPayload pdr_start;
static TEST_PDR_ResponsePayload pdr_response;

//static AMCOM_THROUGHPUT_StartPayload thr_start;
static TEST_THROUGHPUT_ResponsePayload thr_response;

static  TEST_TON_ResponsePayload ton_response;

bool  identified = false;
uint32_t   timestamp = 0;



// Handling AMCOM Packets
void udpPacketHandler(const void* packet, size_t bytesReceived){
    static uint8_t amcomBuf[TEST_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;

    switch (*((const uint8_t*)packet)) {
      case TEST_IDENTIFY_REQUEST:  // Brak
        break;
      case TEST_IDENTIFY_RESPONSE:

        // Application name
        for(int i = 0; i< AMCOM_MAX_DEVICE_NAME_LEN; i++){
            id_response.deviceName [i] = *((const char*)packet+1+i);
        }

        otCliOutputFormat("BR name: %s\n", id_response.deviceName);
        // Application address
        for(int i = AMCOM_MAX_DEVICE_NAME_LEN; i< (AMCOM_MAX_DEVICE_NAME_LEN+AMCOM_MAX_ADDRESS_LEN); i++){
            id_response.deviceAddr [i-AMCOM_MAX_DEVICE_NAME_LEN] = *((const char*)packet+1+i);
        }
        otCliOutputFormat("BR addr: %s\n", id_response.deviceAddr);

        // Identification FLAG
        identified = true;

        break;
      case TEST_RTT_REQUEST:
              otCliOutputFormat("Otrzymalem rtt req\n");
              TEST_RTT_ResponsePayload rtt_response;
              //sprintf(rtt_response.deviceName,"0x%04x", otThreadGetRloc16(otGetInstance()));
              sprintf(rtt_response.deviceName,"%s", id_request.deviceName);
              rtt_response.packet_nunmber = *((const uint8_t*)packet+1)|*((const uint8_t*)packet+1+1)<<8;
              bytesToSend = TEST_Serialize(TEST_RTT_RESPONSE, &rtt_response, sizeof(rtt_response), amcomBuf);

              // Sending RTT Response
              if (bytesToSend > 0) {
                  UDPsend(amcomBuf, bytesToSend, id_response.deviceAddr);
                  otCliOutputFormat("Wysylam rtt response\n");
              }

              break;
      case TEST_RTT_RESPONSE:
              break;
      case TEST_PDR_STOP:
            if(*((const uint8_t*)packet+1) != pdr_response.test_uid){
                otCliOutputFormat("PDR STOP RECV uid: %d THR uid: %d\n",*((const uint8_t*)packet+1),pdr_response.test_uid);
                pdr_response.recv_packets = 0;
            }
            sprintf(pdr_response.deviceName,"%s", id_request.deviceName);
            bytesToSend = TEST_Serialize(TEST_PDR_RESPONSE, &pdr_response, sizeof(pdr_response), amcomBuf);

            otCliOutputFormat("UID: %d Otrzymalem %d pakietow z %d",pdr_response.test_uid, pdr_response.recv_packets, pdr_response.total_packets_number );

            // Sending PDR Response
            if (bytesToSend > 0) {
                UDPsend(amcomBuf, bytesToSend, id_response.deviceAddr);
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
      case TEST_PDR_RESPONSE:
              break;
      case TEST_RSSI_REQUEST:
              sprintf(rssi_response.deviceName,"%s", id_request.deviceName);
              rssi_response.packet_number = *((const uint8_t*)packet+1)|*((const uint8_t*)packet+1+1)<<8;
              rssi_response.rssi = get_rssi();
              bytesToSend = TEST_Serialize(TEST_RSSI_RESPONSE, &rssi_response, sizeof(rssi_response), amcomBuf);

              // Sending RSSI Response
              if (bytesToSend > 0) {
                    UDPsend(amcomBuf, bytesToSend, id_response.deviceAddr);
                    otCliOutputFormat("Wysylam rssi response");
               }

              break;
      case TEST_RSSI_RESPONSE:
              break;
      case TEST_TON_REQUEST:
              sprintf(ton_response.deviceName,"%s", id_request.deviceName);
              ton_response.time_on = getJoinTime();
              bytesToSend = TEST_Serialize(TEST_TON_RESPONSE, &ton_response, sizeof(ton_response), amcomBuf);

              // Sending Ton Response
              if (bytesToSend > 0) {
                  UDPsend(amcomBuf, bytesToSend, id_response.deviceAddr);
                  otCliOutputFormat("Wysylam ton response %d vs %d",ton_response.time_on, getJoinTime());
              }

              break;
      case TEST_TON_RESPONSE:
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
      case TEST_THROUGHPUT_RESPONSE:
              break;
      case TEST_THROUGHPUT_STOP:
              if(*((const uint8_t*)packet+1) != thr_response.test_uid){
                  thr_response.recv_packets = 0;
              }
              sprintf(thr_response.deviceName,"%s", id_request.deviceName);
              thr_response.packet_size = *((const uint8_t*)packet+2);
              bytesToSend = TEST_Serialize(TEST_THROUGHPUT_RESPONSE, &thr_response, sizeof(thr_response), amcomBuf);

              if (bytesToSend > 0) {
                  UDPsend(amcomBuf, bytesToSend, id_response.deviceAddr);
                  otCliOutputFormat("Wysylam thr response %d recv packet size: %d\n",thr_response.recv_packets, thr_response.packet_size );
             }
              break;
      default:
        break;
    }
}

/// Function that determines the state of the device
char get_dev_state(void){
  otDeviceRole dev_role = otThreadGetDeviceRole(otGetInstance());
  if(dev_role == OT_DEVICE_ROLE_CHILD){
      return 'C';
  }
  else if (dev_role == OT_DEVICE_ROLE_ROUTER){
      return 'R';
  }
  else if (dev_role == OT_DEVICE_ROLE_LEADER){
      return 'L';
  }
  else{
      return 'D';   // DISABLED or DETACHED
  }
}

/// Function that provide identification on main application.
void identify_request(void){
  static uint8_t amcomBuf[TEST_MAX_PACKET_SIZE];
  size_t bytesToSend = 0;

  sprintf(id_request.deviceName,"0x%04x", otThreadGetRloc16(otGetInstance()));

  const otNetifAddress *address = otIp6GetUnicastAddresses(otGetInstance());
  while(address->mNext !=NULL){
      if (address->mAddressOrigin == OT_ADDRESS_ORIGIN_SLAAC){
          otIp6AddressToString(&(address->mAddress), id_request.deviceAddr, AMCOM_MAX_ADDRESS_LEN);
          break;
      }
      address= address->mNext;
  }

  id_request.deviceState = get_dev_state();

  bytesToSend = TEST_Serialize(TEST_IDENTIFY_REQUEST, &id_request, sizeof(id_request), amcomBuf);

  otCliOutputFormat("identify%d\n",bytesToSend);
  if (bytesToSend > 0) {
      UDPsend(amcomBuf, bytesToSend, APPLICATION_ADDR);
    }
}

/// Function that checks the differences between the sent and current identification data.
bool send_dev_info_correct(void){
  char deviceName[AMCOM_MAX_DEVICE_NAME_LEN];
  char deviceAddr[AMCOM_MAX_ADDRESS_LEN];
  sprintf(deviceName,"0x%04x", otThreadGetRloc16(otGetInstance()) );

  const otNetifAddress *address = otIp6GetUnicastAddresses(otGetInstance());
  while(address->mNext !=NULL){
      if (address->mAddressOrigin == OT_ADDRESS_ORIGIN_SLAAC){
          otIp6AddressToString(&(address->mAddress), deviceAddr, AMCOM_MAX_ADDRESS_LEN);
          break;
      }
      address= address->mNext;
  }
  if(id_request.deviceState == get_dev_state() && strcmp(id_request.deviceName,deviceName) == 0 && strcmp(id_request.deviceAddr, deviceAddr)==0)
    return true;

  return false;
}

size_t TEST_Serialize(uint8_t packetType, const void* payload, size_t payloadSize, uint8_t* destinationBuffer) {
  size_t i =0;
  destinationBuffer[i] = packetType;
  for (i = 1; i < (payloadSize + 1); i++) {
       destinationBuffer[i] = *((const char*)payload + i - 1);
  }
  return i;
}

