#ifndef AMCOM_PACKETS_H_
#define AMCOM_PACKETS_H_
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>
#define TESTPACKED 							__attribute__((packed))

#define TEST_MAX_PACKET_SIZE 200
// Maximum length of device name
#define TEST_MAX_DEVICE_NAME_LEN 10
// Maximum number of PDR packets per test
#define TEST_MAX_PDR_PACKET  1000
// Maximum number of RTT packets per test
#define TEST_MAX_RTT_PACKET 10
// Maximum number of RSSI packets per test
#define TEST_MAX_RSSI_PACKET 10
// Maximum number of connected device
#define TEST_MAX_NEIGHBOR  10
// Maximum length of IPv6 address
#define TEST_MAX_ADDRESS_LEN  40


// Payload size of Throughput packet min + 5 
#define THROUGHPUT_PAYLOAD 100

 
 
typedef enum {
  TEST_NO_PACKET = 0,
 
  TEST_IDENTIFY_REQUEST = 1,   // Prośba o identyfikacje
  TEST_IDENTIFY_RESPONSE = 2,    // Odesłanie nazwy, adresu, state
 
  TEST_RTT_REQUEST = 3,      // Zapytanie o czas synchronizacji
  TEST_RTT_RESPONSE = 4,     // Odesłanie swojego czasu, stanu synchronizacji

  TEST_PDR_STOP = 6,     // Zakończenie testu PDR -> Klient odsyła AMCOM_PDR_RESPONSE
  TEST_PDR_REQUEST = 7,      // Pakiet w tescie PDR: Nr. próby, Nr pakietu
  TEST_PDR_RESPONSE = 8,     // Odpowiedź na AMCOM_PDR_STOP: informacja na temat ilości odebranych pakietów w danej turze
 
  TEST_RSSI_REQUEST = 9,     // Zapytanie o czas synchronizacji
  TEST_RSSI_RESPONSE = 10,   // Odesłanie swojego czasu, stanu synchronizacji
 
  TEST_TON_REQUEST = 11,          // Ton
  TEST_TON_RESPONSE = 12,
             
  TEST_THROUGHPUT_REQUEST = 14,   // Throughput
  TEST_THROUGHPUT_RESPONSE = 15,
  TEST_THROUGHPUT_STOP = 16,
 
} TEST_PacketType;
 
typedef struct TESTPACKED {
  char deviceName[TEST_MAX_DEVICE_NAME_LEN];
  char deviceAddr[TEST_MAX_ADDRESS_LEN];
  char deviceState;
  bool active;
} TEST_IdentifyRequestPayload;
 
 
typedef struct TESTPACKED {
  char deviceName[TEST_MAX_DEVICE_NAME_LEN];
  char deviceAddr[TEST_MAX_ADDRESS_LEN];
 
} TEST_IdentifyResponsePayload;
 
typedef struct TESTPACKED { 
  uint16_t packet_number;
} TEST_RTT_RequestPayload;
 
typedef struct TESTPACKED {
  char deviceName[TEST_MAX_DEVICE_NAME_LEN];
  uint16_t packet_number;
} TEST_RTT_ResponsePayload;

typedef struct TESTPACKED {
  struct timeval start;
  struct timeval stop;
}RTT_PACKET_TIME;


typedef struct TESTPACKED {
  uint8_t test_uid;
} TEST_PDR_StopPayload;
 
typedef struct TESTPACKED {
  uint8_t test_uid;
  uint16_t packet_number;
  uint16_t total_packets_number;
} TEST_PDR_RequestPayload;
 
typedef struct TESTPACKED {
  char deviceName[TEST_MAX_DEVICE_NAME_LEN];
  uint8_t test_uid;
  uint16_t recv_packets;
  uint16_t total_packets_number;
} TEST_PDR_ResponsePayload;
 
typedef struct TESTPACKED {
  uint16_t packet_number;
} TEST_RSSI_RequestPayload;
 
typedef struct TESTPACKED {
  char deviceName[TEST_MAX_DEVICE_NAME_LEN];
  uint16_t packet_number;
  int8_t rssi;
} TEST_RSSI_ResponsePayload;
 
 
typedef struct TESTPACKED {
  //Empty
} TEST_TON_RequestPayload;
 
typedef struct TESTPACKED {
  char deviceName[TEST_MAX_DEVICE_NAME_LEN];
  uint32_t time_on;
} TEST_TON_ResponsePayload;

typedef struct TESTPACKED {
  uint8_t test_uid;
  uint8_t packet_size;
} TEST_THROUGHPUT_StopPayload;
 
typedef struct {
  uint8_t test_uid;
  uint8_t test_payload[THROUGHPUT_PAYLOAD];
} TEST_THROUGHPUT_RequestPayload;

typedef struct TESTPACKED {
  char deviceName[TEST_MAX_DEVICE_NAME_LEN];
  uint8_t test_uid; 
  uint16_t recv_packets;
  uint8_t packet_size;
} TEST_THROUGHPUT_ResponsePayload;
 
 
 
 
#endif