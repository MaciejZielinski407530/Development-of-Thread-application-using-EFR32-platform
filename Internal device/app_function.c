#include "app_function.h"
#include "amcom_packets.h"
#include "app.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <openthread/udp.h>

#include <openthread/thread.h>

otInstance *otGetInstance(void);
int8_t get_rssi(void);
void UDPsend (uint8_t* buf[AMCOM_MAX_PACKET_SIZE], size_t* bytesToSend, const char* send_addr);

static AMCOM_IdentifyRequestPayload id_request;
static AMCOM_IdentifyResponsePayload id_response;

static AMCOM_RSSI_ResponsePayload rssi_response;

static AMCOM_PDR_StartPayload pdr_start;
static AMCOM_PDR_ResponsePayload pdr_response;

static AMCOM_THROUGHPUT_StartPayload thr_start;
static AMCOM_THROUGHPUT_ResponsePayload thr_response;

static  AMCOM_TON_ResponsePayload ton_response;

bool  identified = false;
int   timestamp = 0;
bool  send_thr = false;


// Handling AMCOM Packets
void amcomPacketHandler(const AMCOM_Packet* packet, void* userContext){
    static uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;

    switch (packet->header.type) {
      case AMCOM_IDENTIFY_REQUEST:  // Brak
        break;
      case AMCOM_IDENTIFY_RESPONSE:

        // Application name
        for(int i = 0; i< AMCOM_MAX_DEVICE_NAME_LEN; i++){
            id_response.deviceName [i] = packet->payload[i];
        }

        otCliOutputFormat("BR name: %s\n", id_response.deviceName);
        // Application address
        for(int i = AMCOM_MAX_DEVICE_NAME_LEN; i< (AMCOM_MAX_DEVICE_NAME_LEN+AMCOM_MAX_ADDRESS_LEN); i++){
            id_response.deviceAddr [i-AMCOM_MAX_DEVICE_NAME_LEN] = packet->payload[i];
        }
        otCliOutputFormat("BR addr: %s\n", id_response.deviceAddr);

        // Identification FLAG
        identified = true;

        break;
      case AMCOM_RTT_REQUEST:
              otCliOutputFormat("Otrzymalem rtt req\n");
              AMCOM_RTT_ResponsePayload rtt_response;

              rtt_response.test_number = packet->payload[0];
              rtt_response.packet_nunmber = packet->payload[1]|packet->payload[2]<<8;
              bytesToSend = AMCOM_Serialize(AMCOM_RTT_RESPONSE, &rtt_response, sizeof(rtt_response), amcomBuf);

              // Sending RTT Response
              if (bytesToSend > 0) {
                  UDPsend(amcomBuf, bytesToSend, id_response.deviceAddr);
                  otCliOutputFormat("Wysylam rtt response\n");
              }

              break;
      case AMCOM_RTT_RESPONSE:
              break;
      case AMCOM_PDR_START:
            pdr_start.expect_tests = packet->payload[0];
            pdr_start.expect_packets = packet->payload[1] | (packet->payload[2]<<8);
            otCliOutputFormat("Expect tests: %d Expect packets: %d", pdr_start.expect_tests, pdr_start.expect_packets);

            for (int i = 0; i<AMCOM_MAX_PDR_TEST; i++){   // Wyczyszczenie pdr_response z poprzednich testow
                pdr_response.recv_packets[i]=0;
            }

              break;
      case AMCOM_PDR_STOP:
            bytesToSend = AMCOM_Serialize(AMCOM_PDR_RESPONSE, &pdr_response, sizeof(pdr_response), amcomBuf);

            for(int i = 0; i<pdr_start.expect_tests;i++){
                otCliOutputFormat("Otrzymalem %d pakietow dla testu %d",pdr_response.recv_packets[i],i );
            }

            // Sending PDR Response
            if (bytesToSend > 0) {
                UDPsend(amcomBuf, bytesToSend, id_response.deviceAddr);
                otCliOutputFormat("Wysylam pdr response");
            }
              break;
      case AMCOM_PDR_REQUEST:
              pdr_response.recv_packets[packet->payload[0]]++;
              otCliOutputFormat("%d \n",packet->payload[1]|packet->payload[2]<<8);
              break;
      case AMCOM_PDR_RESPONSE:
              break;
      case AMCOM_RSSI_REQUEST:

              rssi_response.test_number = packet->payload[0];
              rssi_response.packet_number = packet->payload[1]|packet->payload[2]<<8;
              rssi_response.rssi = get_rssi();
              bytesToSend = AMCOM_Serialize(AMCOM_RSSI_RESPONSE, &rssi_response, sizeof(rssi_response), amcomBuf);

              // Sending RSSI Response
              if (bytesToSend > 0) {
                    UDPsend(amcomBuf, bytesToSend, id_response.deviceAddr);
                    otCliOutputFormat("Wysylam rssi response");
               }

              break;
      case AMCOM_RSSI_RESPONSE:
              break;
      case AMCOM_TON_REQUEST:
              ton_response.time_on = getJoinTime();
              bytesToSend = AMCOM_Serialize(AMCOM_TON_RESPONSE, &ton_response, sizeof(ton_response), amcomBuf);

              // Sending Ton Response
              if (bytesToSend > 0) {
                  UDPsend(amcomBuf, bytesToSend, id_response.deviceAddr);
                  otCliOutputFormat("Wysylam rssi response");
              }

              break;
      case AMCOM_TON_RESPONSE:
              break;
      case AMCOM_THROUGHPUT_START:
              thr_start.expect_packets = packet->payload[2] | packet->payload[3]<<8;
              thr_response.recv_packets = 0;
              timestamp=0;
              send_thr = false;
              break;
      case AMCOM_THROUGHPUT_REQUEST:
              if(timestamp==0)
                timestamp = getSysTick_time();
              thr_response.recv_packets++;
              if(getSysTick_time()-timestamp >= 10000 && send_thr == false){
                    bytesToSend = AMCOM_Serialize(AMCOM_THROUGHPUT_RESPONSE, &thr_response, sizeof(thr_response), amcomBuf);
                    send_thr = true;
                    if (bytesToSend > 0) {
                       UDPsend(amcomBuf, bytesToSend, id_response.deviceAddr);
                       otCliOutputFormat("Wysylam thr response %d recv\n",thr_response.recv_packets );
                    }
              }

              break;
      case AMCOM_THROUGHPUT_RESPONSE:
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
  static uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
  size_t bytesToSend = 0;

  sprintf(id_request.deviceName,"0x%04x", otThreadGetRloc16(otGetInstance()) );

  otNetifAddress *address = otIp6GetUnicastAddresses(otGetInstance());
  while(address->mNext !=NULL){
      if (address->mAddressOrigin == OT_ADDRESS_ORIGIN_SLAAC){
          otIp6AddressToString(&(address->mAddress), id_request.deviceAddr, AMCOM_MAX_ADDRESS_LEN);
          break;
      }
      address= address->mNext;
  }

  id_request.deviceState = get_dev_state();

  bytesToSend = AMCOM_Serialize(AMCOM_IDENTIFY_REQUEST, &id_request, sizeof(id_request), amcomBuf);

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

  otNetifAddress *address = otIp6GetUnicastAddresses(otGetInstance());
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
