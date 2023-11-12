

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

#include "app_function.h"
#include "amcom_packets.h"
#include "amcom.h"


#include "sl_component_catalog.h"
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif

//#define SendPORT 8888
#define PORT 8080

otInstance *otGetInstance(void);
void ReceiveCallback(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);
extern void otSysEventSignalPending(void);

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

    // Initialize bindAddr
    memset(&bindAddr, 0, sizeof(bindAddr));
    bindAddr.mPort = PORT;

    // Open the socket
    error = otUdpOpen(otGetInstance(), &RecvSocket, ReceiveCallback, NULL);
    if (error != OT_ERROR_NONE)
    {
        otCliOutputFormat("Device failed to open udp socket with: %d, %s\r\n", error, otThreadErrorToString(error));
        return;
    }

    // Bind to the socket. Close the socket if bind fails.
    error = otUdpBind(otGetInstance(), &RecvSocket, &bindAddr, OT_NETIF_THREAD);
    if (error != OT_ERROR_NONE)
    {
        otCliOutputFormat("Device failed to bind udp socket with: %d, %s\r\n", error, otThreadErrorToString(error));
        IgnoreReturnValue(otUdpClose(otGetInstance(), &RecvSocket));
        return;
    }
}

/// UDP sending function
void UDPsend (uint8_t* buf[AMCOM_MAX_PACKET_SIZE], size_t* bytesToSend, const char* send_addr){
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

      otMessageInfo    messageInfo;
      otMessage        *message = NULL;
      otIp6Address     RecvAddress;                               // Na koniec do usuniecia
      static AMCOM_Receiver amcomReceiver;                // AMCOM receiver structure
      static char      buf [AMCOM_MAX_PACKET_SIZE];       // Buffer for received data
      int              receivedBytesCount;                // Number of bytes received
      char             ipaddress [AMCOM_MAX_ADDRESS_LEN];

      // Read Message RSSI
      rssi = otMessageGetRss(aMessage);

      // Initialization AMCOM Receiver
      AMCOM_InitReceiver(&amcomReceiver, amcomPacketHandler,NULL);

      // Read the received message's payload
      receivedBytesCount = otMessageRead(aMessage, otMessageGetOffset(aMessage), buf, sizeof(buf) - 1);

      memcpy(&RecvAddress, &aMessageInfo->mPeerAddr, sizeof RecvAddress);               // Na koniec do usuniecia
      otIp6AddressToString(&RecvAddress, ipaddress, sizeof(ipaddress));                 // Na koniec do usuniecia

      otCliOutputFormat("Message Received from %s  oraz RSSI = %d dBm\r\n", ipaddress,rssi); // Na koniec do usuniecia

      // Deserialization of received data
      AMCOM_Deserialize(&amcomReceiver, buf, receivedBytesCount);

      message = NULL;


}
