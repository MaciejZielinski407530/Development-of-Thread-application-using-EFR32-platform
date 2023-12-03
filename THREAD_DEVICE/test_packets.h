#ifndef TEST_PACKETS_H_
#define TEST_PACKETS_H_
#include <stdint.h>
#include <stdbool.h>

#include <openthread/thread.h>

#define TESTPACKED  __attribute__((packed))

#define TEST_MAX_PACKET_SIZE 200
// Maximum length of device name
#define AMCOM_MAX_DEVICE_NAME_LEN 10
// Maximum length of IPv6 address
#define AMCOM_MAX_ADDRESS_LEN  46

typedef enum {
  TEST_NO_PACKET = 0,

  TEST_IDENTIFY_REQUEST = 1,   // IDENTIFICATION
  TEST_IDENTIFY_RESPONSE = 2,

  TEST_RTT_REQUEST = 3,      // RTT
  TEST_RTT_RESPONSE = 4,

  TEST_PDR_STOP = 5,         // PDR
  TEST_PDR_REQUEST = 6,
  TEST_PDR_RESPONSE = 7,

  TEST_RSSI_REQUEST = 8,     //RSSI
  TEST_RSSI_RESPONSE = 9,

  TEST_TON_REQUEST = 10,          // Ton
  TEST_TON_RESPONSE = 11,

  TEST_THROUGHPUT_REQUEST = 12,     // Throughput
  TEST_THROUGHPUT_RESPONSE = 13,
  TEST_THROUGHPUT_STOP = 14,
} TEST_PacketType;


typedef struct TESTPACKED {
  char deviceName[AMCOM_MAX_DEVICE_NAME_LEN];
} TEST_IdentifyResponsePayload;

typedef struct TESTPACKED {
  uint16_t packet_number;
} TEST_RTT_RequestPayload;

typedef struct TESTPACKED {
  char deviceName[AMCOM_MAX_DEVICE_NAME_LEN];
  uint16_t packet_nunmber;
} TEST_RTT_ResponsePayload;

typedef struct TESTPACKED {
  uint8_t test_uid;
} TEST_PDR_StopPayload;

typedef struct TESTPACKED {
  uint8_t test_uid;
  uint16_t packet_nunmber;
  uint16_t total_packets_number;
} TEST_PDR_RequestPayload;

typedef struct TESTPACKED {
  char deviceName[AMCOM_MAX_DEVICE_NAME_LEN];
  uint8_t test_uid;
  uint16_t recv_packets;
  uint16_t total_packets_number;
} TEST_PDR_ResponsePayload;

typedef struct TESTPACKED {
  uint16_t packet_number;
} TEST_RSSI_RequestPayload;

typedef struct TESTPACKED {
  char deviceName[AMCOM_MAX_DEVICE_NAME_LEN];
  uint16_t packet_number;
  int8_t rssi;
} TEST_RSSI_ResponsePayload;

typedef struct TESTPACKED {
  char deviceName[AMCOM_MAX_DEVICE_NAME_LEN];
  uint32_t time_on;
} TEST_TON_ResponsePayload;

typedef struct TESTPACKED {
  char deviceName[AMCOM_MAX_DEVICE_NAME_LEN];
  uint8_t test_uid;
  uint16_t recv_packets;
  uint8_t packet_size;
} TEST_THROUGHPUT_ResponsePayload;


#endif
