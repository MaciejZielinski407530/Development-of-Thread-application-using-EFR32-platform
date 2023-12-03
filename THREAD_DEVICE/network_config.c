#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <openthread/cli.h>
#include <openthread/dataset_ftd.h>
#include <openthread/instance.h>
#include <openthread/message.h>
#include <openthread/thread.h>
#include <openthread/udp.h>
#include <openthread/platform/logging.h>
#include <common/code_utils.hpp>
#include <common/logging.hpp>

#include "test_packets.h"

#include "app_function.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif

#define PORT 8080


otInstance *otGetInstance(void);
void ReceiveCallback(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);


static otUdpSocket RecvSocket;
static int8_t rssi;

/// Function return RSSI for last message
int8_t get_rssi(void){
  return rssi;
}

/// Initialization UDP
void initUdp(void)
{
    otError    error;
    otSockAddr bindAddr;

    otIp6Address multicast_addr;          // ADD MULTICAST address for identification
    otIp6AddressFromString(MULTICAST_ADDRESS, &multicast_addr);
    otIp6SubscribeMulticastAddress(otGetInstance(), &multicast_addr);

    // Initialize bindAddr
    memset(&bindAddr, 0, sizeof(bindAddr));
    bindAddr.mPort = PORT;

    // Bind SLAAC address to the socket
    const otNetifAddress *address = otIp6GetUnicastAddresses(otGetInstance());
    while(address->mNext !=NULL){
        if (address->mAddressOrigin == OT_ADDRESS_ORIGIN_SLAAC){
            bindAddr.mAddress = address->mAddress;
            break;
        }
        address= address->mNext;
    }
    // Open the socket
    error = otUdpOpen(otGetInstance(), &RecvSocket, ReceiveCallback, NULL);
    if (error != OT_ERROR_NONE)
    {
        otCliOutputFormat("Device failed to open udp socket with: %d, %s\r\n", error, otThreadErrorToString(error));
        return;
    }

    // Bind to the socket
    error = otUdpBind(otGetInstance(), &RecvSocket, &bindAddr, OT_NETIF_THREAD);
    if (error != OT_ERROR_NONE)
    {
        otCliOutputFormat("Device failed to bind udp socket with: %d, %s\r\n", error, otThreadErrorToString(error));
        IgnoreReturnValue(otUdpClose(otGetInstance(), &RecvSocket));
        return;
    }
}

/// UDP sending function
void UDPsend (uint8_t buf[TEST_MAX_PACKET_SIZE], size_t bytesToSend, const char* send_addr){
      otMessageInfo    messageInfo;
      otMessage       *message = NULL;
      otIp6Address     send_address;

      // Transform String to otIp6Address
      otIp6AddressFromString(send_addr, &send_address);

      // Create a new UDP message
      VerifyOrExit((message = otUdpNewMessage(otGetInstance(), NULL)) != NULL);

      memset(&messageInfo, 0, sizeof(messageInfo));
      memcpy(&messageInfo.mPeerAddr, &send_address, sizeof messageInfo.mPeerAddr);
      messageInfo.mPeerPort = PORT;

      // Append new message
      SuccessOrExit(otMessageAppend(message, buf, bytesToSend));

      // Send UDP message
      SuccessOrExit(otUdpSend(otGetInstance(), &RecvSocket, message, &messageInfo));

      exit:
          return;

}

/// Callback for received UDP message
void ReceiveCallback(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
      OT_UNUSED_VARIABLE(aContext);
      OT_UNUSED_VARIABLE(aMessageInfo);


      otIp6Address     RecvAddress;                               // Na koniec do usuniecia
      static uint8_t   buf [TEST_MAX_PACKET_SIZE];       // Buffer for received data
      int              receivedBytesCount;                // Number of bytes received
      char             ipaddress [AMCOM_MAX_ADDRESS_LEN];

      // Read Message RSSI
      rssi = otMessageGetRss(aMessage);

      // Read the received message's payload
      receivedBytesCount = otMessageRead(aMessage, otMessageGetOffset(aMessage), buf, sizeof(buf) - 1);

      memcpy(&RecvAddress, &aMessageInfo->mPeerAddr, sizeof RecvAddress);
      otIp6AddressToString(&RecvAddress, ipaddress, sizeof(ipaddress));

      otCliOutputFormat("Message Received from %s  RSSI = %d dBm\r\n", ipaddress,rssi);

      udpPacketHandler((uint8_t*)buf,receivedBytesCount, ipaddress);



}
