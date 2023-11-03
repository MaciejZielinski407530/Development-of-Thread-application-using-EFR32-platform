#include "fun_main.h"
#include "amcom_packets.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <openthread/udp.h>

#include <openthread/thread.h>

#define DEVICE_NAME "Inna"

AMCOM_IdentifyRequestPayload id_request;
AMCOM_RTT_RequestPayload rtt;

AMCOM_PDR_StartPayload pdr_start;
AMCOM_PDR_StopPayload pdr_stop;
AMCOM_PDR_RequestPayload pdr_request;
AMCOM_PDR_ResponsePayload pdr_response;

AMCOM_RSSI_RequestPayload rssi;


char get_dev_state(void);
void Identify_response(AMCOM_IdentifyResponsePayload* id);

void amcomPacketHandler(const AMCOM_Packet* packet, void* userContext){
    static uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE];
    size_t bytesToSend = 0;

    switch (packet->header.type) {
      case AMCOM_IDENTIFY_REQUEST:

        for(int i = 0; i< AMCOM_MAX_DEVICE_NAME_LEN; i++){
            id_request.deviceName [i] = packet->payload[i];
        }
        otCliOutputFormat("BR name: %s\n", id_request.deviceName);

        for(int i = AMCOM_MAX_DEVICE_NAME_LEN; i< (AMCOM_MAX_DEVICE_NAME_LEN+AMCOM_MAX_ADDRESS_LEN); i++){
            id_request.deviceAddr [i-AMCOM_MAX_DEVICE_NAME_LEN] = packet->payload[i];
        }
        otCliOutputFormat("BR addr: %s\n", id_request.deviceAddr);

        AMCOM_IdentifyResponsePayload id_response;
        Identify_response(&id_response);
        bytesToSend = AMCOM_Serialize(AMCOM_IDENTIFY_RESPONSE, &id_response, sizeof(id_response), amcomBuf);

        if (bytesToSend > 0) {
             UDPsend(amcomBuf, bytesToSend, id_request.deviceAddr);
             otCliOutputFormat("I sent");
         }

        break;
      case AMCOM_IDENTIFY_RESPONSE:
              break;
      case AMCOM_RTT_REQUEST:
              break;
      case AMCOM_RTT_RESPONSE:
              break;
      case AMCOM_PDR_START:
            pdr_start.expect_tests = packet->payload[0];
            pdr_start.expect_packets = packet->payload[1] | (packet->payload[2]<<8);
            otCliOutputFormat("Expect tests: %d Expect packets: %d", pdr_start.expect_tests, pdr_start.expect_packets);
            for (int i = 0; i<AMCOM_MAX_PDR_TEST; i++){
                pdr_response.recv_packets[i]=0;
            }

              break;
      case AMCOM_PDR_STOP:
            bytesToSend = AMCOM_Serialize(AMCOM_PDR_RESPONSE, &pdr_response, sizeof(pdr_response), amcomBuf);

                if (bytesToSend > 0) {
                     UDPsend(amcomBuf, bytesToSend, id_request.deviceAddr);
                     otCliOutputFormat("I sent pdr response");
                 }


              break;
      case AMCOM_PDR_REQUEST:

              pdr_response.recv_packets[packet->payload[0]]++;
              otCliOutputFormat("%d ",packet->payload[1]|packet->payload[2]<<8);
              otCliOutputFormat("%d ",pdr_response.recv_packets[packet->payload[0]]);

              break;
      case AMCOM_PDR_RESPONSE:
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

void Identify_response(AMCOM_IdentifyResponsePayload* id){
  sprintf(id->deviceName, DEVICE_NAME );
  otNetifAddress *address = otIp6GetUnicastAddresses(otGetInstance());
  otIp6AddressToString(&address->mAddress, id->deviceAddr, AMCOM_MAX_ADDRESS_LEN);
  id->deviceState = get_dev_state();

}
