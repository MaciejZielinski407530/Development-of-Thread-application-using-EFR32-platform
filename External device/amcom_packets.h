#ifndef AMCOM_PACKETS_H_
#define AMCOM_PACKETS_H_
#include <stdint.h>
#include <stdbool.h>
#include "amcom.h"
#include <sys/time.h>

#define AMCOM_MAX_DEVICE_NAME_LEN 24
 
#define AMCOM_MAX_PDR_TEST  10

#define AMCOM_MAX_RTT_TEST 10
#define AMCOM_MAX_RTT_PACKET 10
 
#define AMCOM_MAX_NEIGHBOR  10
 
#define AMCOM_MAX_ADDRESS_LEN  40
#define MULTICAST_ADDR "ff02::1"
 
 
 
typedef enum {
  AMCOM_NO_PACKET = 0,
 
  AMCOM_IDENTIFY_REQUEST = 1,   // Prośba o identyfikacje
  AMCOM_IDENTIFY_RESPONSE = 2,    // Odesłanie nazwy, adresu, state
 
  AMCOM_RTT_REQUEST = 3,      // Zapytanie o czas synchronizacji
  AMCOM_RTT_RESPONSE = 4,     // Odesłanie swojego czasu, stanu synchronizacji
 
  AMCOM_PDR_START = 5,      // Rozpoczęcie testu PDR: informacja ile będzie wysłanych wiadomości w jednej próbie, ile prób
  AMCOM_PDR_STOP = 6,     // Zakończenie testu PDR -> Klient odsyła AMCOM_PDR_RESPONSE
  AMCOM_PDR_REQUEST = 7,      // Pakiet w tescie PDR: Nr. próby, Nr pakietu
  AMCOM_PDR_RESPONSE = 8,     // Odpowiedź na AMCOM_PDR_STOP: informacja na temat ilości odebranych pakietów w danej turze
 
  AMCOM_RSSI_REQUEST = 9,     // Zapytanie o czas synchronizacji
  AMCOM_RSSI_RESPONSE = 10,   // Odesłanie swojego czasu, stanu synchronizacji
 
  AMCOM_TON_REQUEST = 11,          // Ton
  AMCOM_TON_RESPONSE = 12,
 
  AMCOM_THROUGHPUT_START = 13,      // Throughput
  AMCOM_THROUGHPUT_REQUEST = 14,
  AMCOM_THROUGHPUT_RESPONSE = 15,
 
 
} AMCOM_PacketType;
 
typedef struct AMPACKED {
  char deviceName[AMCOM_MAX_DEVICE_NAME_LEN];
  char deviceAddr[AMCOM_MAX_ADDRESS_LEN];
  char deviceState;
  bool active;
} AMCOM_IdentifyRequestPayload;
 
 
typedef struct AMPACKED {
  char deviceName[AMCOM_MAX_DEVICE_NAME_LEN];
  char deviceAddr[AMCOM_MAX_ADDRESS_LEN];
 
} AMCOM_IdentifyResponsePayload;
 
typedef struct AMPACKED {
  uint8_t test_number;
  uint16_t packet_number;
} AMCOM_RTT_RequestPayload;
 
typedef struct AMPACKED {
  uint8_t test_number;
  uint16_t packet_number;
} AMCOM_RTT_ResponsePayload;

typedef struct{
  struct timeval start;
  struct timeval stop;
}RTT_TIME;

typedef struct {
  RTT_TIME rtt_info_time [AMCOM_MAX_RTT_PACKET];
}RTT_INFO;
 
typedef struct AMPACKED {   // Czy wysyłać pakiet potwierdzający, że klient otrzymał AMCOM PDR_START
  uint8_t expect_tests;
  uint16_t expect_packets;
} AMCOM_PDR_StartPayload;
 
typedef struct AMPACKED {
  uint8_t perform_tests;
} AMCOM_PDR_StopPayload;
 
typedef struct AMPACKED {
  uint8_t test_number;
  uint16_t packet_number;
} AMCOM_PDR_RequestPayload;
 
typedef struct AMPACKED {
  uint16_t recv_packets[AMCOM_MAX_PDR_TEST];
} AMCOM_PDR_ResponsePayload;
 
typedef struct AMPACKED {
  uint8_t test_number;
  uint16_t packet_number;
} AMCOM_RSSI_RequestPayload;
 
 
typedef struct AMPACKED {
  uint8_t test_number;
  uint16_t packet_number;
  int8_t rssi;
} AMCOM_RSSI_ResponsePayload;
 
 
typedef struct AMPACKED {
  //uint8_t test_number;
} AMCOM_TON_RequestPayload;
 
typedef struct AMPACKED {
  int time_on;
} AMCOM_TON_ResponsePayload;
 
typedef struct AMPACKED {
  uint16_t expect_packet_size;
  uint16_t expect_packets;
} AMCOM_THROUGHPUT_StartPayload;
 
typedef struct AMPACKED {
  uint8_t payload [64];
} AMCOM_THROUGHPUT_RequestMinPayload;
 
typedef struct AMPACKED {
  uint8_t payload [128];              // Packet Size = 128B + 5B = 133B
} AMCOM_THROUGHPUT_RequestMidPayload;
 
typedef struct AMPACKED {
  uint8_t payload [200];              // Packet Size = 200B + 5B = 205B  -> Max AMCOM packet size
} AMCOM_THROUGHPUT_RequestMaxPayload;
 
typedef struct AMPACKED {
  //uint16_t packet_size;
  uint16_t recv_packets;
} AMCOM_THROUGHPUT_ResponsePayload;
 
 
 
 
#endif