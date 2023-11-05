
#define CURRENT_MODULE_NAME    "OPENTHREAD_SAMPLE_APP"

#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <openthread/cli.h>
#include <openthread/dataset_ftd.h>
#include <openthread/instance.h>
#include <openthread/message.h>
#include <openthread/thread.h>
#include <openthread/udp.h>
#include <openthread/network_time.h>
#include <openthread/platform/logging.h>
#include <common/code_utils.hpp>
#include <common/logging.hpp>

#include "app_function.h"
#include "amcom.h"


#include "sl_component_catalog.h"
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif

#define SendPORT 8888
#define RecvPORT 8080

otInstance *otGetInstance(void);
void mtdReceiveCallback(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);
extern void otSysEventSignalPending(void);

static otUdpSocket sMtdSocket;


void setNetworkConfiguration(void)
{
    //static char          aNetworkName[] = "Smart Home";
    static char          aNetworkName[] = "OpenThreadDemo";
    otError              error;
    otOperationalDataset aDataset;

    memset(&aDataset, 0, sizeof(otOperationalDataset));

    /*
     * Fields that can be configured in otOperationDataset to override defaults:
     *     Network Name, Mesh Local Prefix, Extended PAN ID, PAN ID, Delay Timer,
     *     Channel, Channel Mask Page 0, Network Key, PSKc, Security Policy
     */
    aDataset.mActiveTimestamp.mSeconds             = 1;
    aDataset.mComponents.mIsActiveTimestampPresent = true;

    /* Set Channel to 15 */
    aDataset.mChannel                      = 15;
    aDataset.mComponents.mIsChannelPresent = true;

    /* Set Pan ID to 2222 */
    //aDataset.mPanId                      = (otPanId)0x2222;
    aDataset.mPanId                      = (otPanId)0x1234;
    aDataset.mComponents.mIsPanIdPresent = true;

    /* Set Extended Pan ID to C0DE1AB5C0DE1AB5 */
    //uint8_t extPanId[OT_EXT_PAN_ID_SIZE] = {0xC0, 0xDE, 0x1A, 0xB5, 0xC0, 0xDE, 0x1A, 0xB5};
    uint8_t extPanId[OT_EXT_PAN_ID_SIZE] = {0x11, 0x11, 0x11, 0x11, 0x22, 0x22, 0x22, 0x22};
    memcpy(aDataset.mExtendedPanId.m8, extPanId, sizeof(aDataset.mExtendedPanId));
    aDataset.mComponents.mIsExtendedPanIdPresent = true;

    /* Set network key to 1234C0DE1AB51234C0DE1AB51234C0DE */
    //uint8_t key[OT_NETWORK_KEY_SIZE] = {0x12, 0x34, 0xC0, 0xDE, 0x1A, 0xB5, 0x12, 0x34,
    //                                    0xC0, 0xDE, 0x1A, 0xB5, 0x12, 0x34, 0xC0, 0xDE};
    uint8_t key[OT_NETWORK_KEY_SIZE] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                           0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    memcpy(aDataset.mNetworkKey.m8, key, sizeof(aDataset.mNetworkKey));
    aDataset.mComponents.mIsNetworkKeyPresent = true;

    /* Set Network Name to SleepyEFR32 */
    size_t length = strlen(aNetworkName);
    assert(length <= OT_NETWORK_NAME_MAX_SIZE);
    memcpy(aDataset.mNetworkName.m8, aNetworkName, length);
    aDataset.mComponents.mIsNetworkNamePresent = true;

    /* Set the Active Operational Dataset to this dataset */
    error = otDatasetSetActive(otGetInstance(), &aDataset);
    if (error != OT_ERROR_NONE)
    {
        otCliOutputFormat("otDatasetSetActive failed with: %d, %s\r\n", error, otThreadErrorToString(error));
        return;
    }
}


void initUdp(void)
{
    otError    error;
    otSockAddr bindAddr;

    // Initialize bindAddr
    memset(&bindAddr, 0, sizeof(bindAddr));
    bindAddr.mPort = RecvPORT;

    // Open the socket
    error = otUdpOpen(otGetInstance(), &sMtdSocket, mtdReceiveCallback, NULL);
    if (error != OT_ERROR_NONE)
    {
        otCliOutputFormat("MTD failed to open udp socket with: %d, %s\r\n", error, otThreadErrorToString(error));
        return;
    }

    // Bind to the socket. Close the socket if bind fails.
    error = otUdpBind(otGetInstance(), &sMtdSocket, &bindAddr, OT_NETIF_THREAD);
    if (error != OT_ERROR_NONE)
    {
        otCliOutputFormat("MTD failed to bind udp socket with: %d, %s\r\n", error, otThreadErrorToString(error));
        IgnoreReturnValue(otUdpClose(otGetInstance(), &sMtdSocket));
        return;
    }
}
void UDPsend (uint8_t* buf[AMCOM_MAX_PACKET_SIZE], size_t* bytesToSend, const char* send_addr){
      otMessageInfo    messageInfo;
      otMessage       *message = NULL;
      otIp6Address send_address;
      otIp6AddressFromString(send_addr, &send_address);

      VerifyOrExit((message = otUdpNewMessage(otGetInstance(), NULL)) != NULL);

      memset(&messageInfo, 0, sizeof(messageInfo));
      memcpy(&messageInfo.mPeerAddr, &send_address, sizeof messageInfo.mPeerAddr);
      messageInfo.mPeerPort = RecvPORT;

      SuccessOrExit(otMessageAppend(message, buf, bytesToSend));

      SuccessOrExit(otUdpSend(otGetInstance(), &sMtdSocket, message, &messageInfo));


      exit:
          return;

}
void mtdReceiveCallback(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
      otMessageInfo    messageInfo;
      otMessage       *message = NULL;
      otIp6Address              RecvAddress;
      static AMCOM_Receiver amcomReceiver;    // AMCOM receiver structure
      static char buf[512];                   // buffer for temporary data
      int receivedBytesCount;


      char   ipaddress[40];
      OT_UNUSED_VARIABLE(aContext);
      OT_UNUSED_VARIABLE(aMessageInfo);

      AMCOM_InitReceiver(&amcomReceiver, amcomPacketHandler,NULL);

      // Read the received message's payload
      receivedBytesCount = otMessageRead(aMessage, otMessageGetOffset(aMessage), buf, sizeof(buf) - 1);
      //buf[receivedBytesCount] = '\0';

      memcpy(&RecvAddress, &aMessageInfo->mPeerAddr, sizeof RecvAddress);
      otIp6AddressToString(&RecvAddress, ipaddress, sizeof(ipaddress));
      //otCliOutputFormat("Message Received from %s :%s\r\n", ipaddress , buf);
      otCliOutputFormat("Message Received from %s \r\n", ipaddress);


      AMCOM_Deserialize(&amcomReceiver, buf, receivedBytesCount);

      message = NULL;


}
