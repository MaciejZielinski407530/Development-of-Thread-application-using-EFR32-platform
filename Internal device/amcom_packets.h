#ifndef AMCOM_PACKETS_H_
#define AMCOM_PACKETS_H_
#include <stdint.h>
#include <stdbool.h>
#include "amcom.h"
#include <openthread/thread.h>

// Maximum length of device name
#define AMCOM_MAX_DEVICE_NAME_LEN 10
// Maximum number of PDR tests
#define AMCOM_MAX_PDR_TEST  10
// Maximum length of IPv6 address
#define AMCOM_MAX_ADDRESS_LEN  40
// Main application IPv6 address
//#define APPLICATION_ADDR "fd11:1111:1122:2222:bc3f:2f4a:abaa:b460"
#define APPLICATION_ADDR "fd11:1111:1122:2222:9133:c65d:5301:132e"
//#define APPLICATION_ADDR "fd11:1111:1122:2222:c99e:51b2:2265:51db"
//#define APPLICATION_ADDR "2a02:a31a:a137:8680:33fe:ac5a:3d62:b044"



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

  AMCOM_RSSI_REQUEST = 9,     //
  AMCOM_RSSI_RESPONSE = 10,   //

  AMCOM_TON_REQUEST = 11,          // Ton
  AMCOM_TON_RESPONSE = 12,

  AMCOM_THROUGHPUT_START = 13,      // Throughput
  AMCOM_THROUGHPUT_REQUEST = 14,
  AMCOM_THROUGHPUT_RESPONSE = 15,
  AMCOM_THROUGHPUT_STOP = 16,


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
  char deviceName[AMCOM_MAX_DEVICE_NAME_LEN];
  uint8_t test_number;
  uint16_t packet_nunmber;
} AMCOM_RTT_ResponsePayload;

typedef struct AMPACKED {   // Czy wysyłać pakiet potwierdzający, że klient otrzymał AMCOM PDR_START
  uint8_t expect_tests;
  uint16_t expect_packets;
} AMCOM_PDR_StartPayload;

typedef struct AMPACKED {
  uint8_t perform_tests;
} AMCOM_PDR_StopPayload;

typedef struct AMPACKED {
  uint8_t test_number;
  uint16_t packet_nunmber;
} AMCOM_PDR_RequestPayload;

typedef struct AMPACKED {
  char deviceName[AMCOM_MAX_DEVICE_NAME_LEN];
  uint16_t recv_packets[AMCOM_MAX_PDR_TEST];
} AMCOM_PDR_ResponsePayload;

typedef struct AMPACKED {
  uint8_t test_number;
  uint16_t packet_number;
} AMCOM_RSSI_RequestPayload;

typedef struct AMPACKED {
  char deviceName[AMCOM_MAX_DEVICE_NAME_LEN];
  uint8_t test_number;
  uint16_t packet_number;
  int8_t rssi;
} AMCOM_RSSI_ResponsePayload;


typedef struct AMPACKED {
  //uint8_t test_number;
} AMCOM_TON_RequestPayload;

typedef struct AMPACKED {
  char deviceName[AMCOM_MAX_DEVICE_NAME_LEN];
  uint16_t time_on;
} AMCOM_TON_ResponsePayload;

typedef struct AMPACKED {
  uint16_t expect_packet_size;
  uint16_t expect_packets;
} AMCOM_THROUGHPUT_StartPayload;

typedef struct AMPACKED {
  uint8_t payload [64];               // Packet Size = 64B + 5B = 69B
} AMCOM_THROUGHPUT_RequestMinPayload;

typedef struct AMPACKED {
  uint8_t payload [128];              // Packet Size = 128B + 5B = 133B
} AMCOM_THROUGHPUT_RequestMidPayload;

typedef struct AMPACKED {
  uint8_t payload [200];              // Packet Size = 200B + 5B = 205B  -> Max AMCOM packet size
} AMCOM_THROUGHPUT_RequestMaxPayload;

typedef struct AMPACKED {
  //uint16_t packet_size;
  char deviceName[AMCOM_MAX_DEVICE_NAME_LEN];
  uint16_t recv_packets;
} AMCOM_THROUGHPUT_ResponsePayload;


#endif
