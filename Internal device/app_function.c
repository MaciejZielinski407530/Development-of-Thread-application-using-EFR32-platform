#include "app_function.h"
#include "amcom_packets.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <openthread/udp.h>

#include <openthread/thread.h>




static AMCOM_IdentifyRequestPayload id_request;
static AMCOM_IdentifyResponsePayload id_response;

//static AMCOM_RTT_RequestPayload rtt;

static AMCOM_PDR_StartPayload pdr_start;
static AMCOM_PDR_StopPayload pdr_stop;
static AMCOM_PDR_RequestPayload pdr_request;
static AMCOM_PDR_ResponsePayload pdr_response;

static AMCOM_RSSI_RequestPayload rssi;

bool identified = false;

void amcomPacketHandler(const AMCOM_Packet* packet, void* userContext){
    static uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;

    switch (packet->header.type) {
      case AMCOM_IDENTIFY_REQUEST:  // Brak
        break;
      case AMCOM_IDENTIFY_RESPONSE:

        for(int i = 0; i< AMCOM_MAX_DEVICE_NAME_LEN; i++){
            id_response.deviceName [i] = packet->payload[i];
        }
        otCliOutputFormat("BR name: %s\n", id_response.deviceName);

        for(int i = AMCOM_MAX_DEVICE_NAME_LEN; i< (AMCOM_MAX_DEVICE_NAME_LEN+AMCOM_MAX_ADDRESS_LEN); i++){
            id_response.deviceAddr [i-AMCOM_MAX_DEVICE_NAME_LEN] = packet->payload[i];
        }
        otCliOutputFormat("BR addr: %s\n", id_response.deviceAddr);
        identified = true;

              break;
      case AMCOM_RTT_REQUEST:
              otCliOutputFormat("Otrzymalem rtt req\n");
              AMCOM_RTT_ResponsePayload rtt_response;

              rtt_response.test_number = packet->payload[0];
              rtt_response.packet_nunmber = packet->payload[1]|packet->payload[2]<<8;
              bytesToSend = AMCOM_Serialize(AMCOM_RTT_RESPONSE, &rtt_response, sizeof(rtt_response), amcomBuf);

              if (bytesToSend > 0) {
                  UDPsend(amcomBuf, bytesToSend, id_response.deviceAddr);
                  otCliOutputFormat("Wysylam rtt response");
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


                if (bytesToSend > 0) {
                     UDPsend(amcomBuf, bytesToSend, id_response.deviceAddr);
                     otCliOutputFormat("Wysylam pdr response");
                 }


              break;
      case AMCOM_PDR_REQUEST:
              pdr_response.recv_packets[packet->payload[0]]++;
              otCliOutputFormat("%d \n",packet->payload[1]|packet->payload[2]<<8);
              //otCliOutputFormat("%d ",pdr_response.recv_packets[packet->payload[0]]);
              break;
      case AMCOM_PDR_RESPONSE:  // Brak
              break;
      case AMCOM_RSSI_REQUEST:
              break;
      case AMCOM_RSSI_RESPONSE:
              break;
      default:
        break;
    }
}
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


void identify_request(void){
  static uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
  size_t bytesToSend = 0;

  sprintf(id_request.deviceName,"0x%04x", otThreadGetRloc16(otGetInstance()) );
  otNetifAddress *address = otIp6GetUnicastAddresses(otGetInstance());
  //while(address){
  otIp6AddressToString(&(address->mNext->mNext)->mAddress, id_request.deviceAddr, AMCOM_MAX_ADDRESS_LEN);

  //}
  id_request.deviceState = get_dev_state();

  bytesToSend = AMCOM_Serialize(AMCOM_IDENTIFY_REQUEST, &id_request, sizeof(id_request), amcomBuf);

  otCliOutputFormat("identify%d\n",bytesToSend);
  if (bytesToSend > 0) {
      UDPsend(amcomBuf, bytesToSend, MULTICAST_ADDR);
    }


}
