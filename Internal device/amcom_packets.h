#ifndef AMCOM_PACKETS_H_
#define AMCOM_PACKETS_H_
#include <stdint.h>
#include <stdbool.h>
#include "amcom.h"
#include <openthread/thread.h>
#define AMCOM_MAX_DEVICE_NAME_LEN 24

#define AMCOM_MAX_PDR_TEST  10

#define AMCOM_MAX_NEIGHBOR  10

#define AMCOM_MAX_ADDRESS_LEN  40
#define MULTICAST_ADDR "ff03::1"



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

            // Ton

            // Throughput

} AMCOM_PacketType;

typedef struct AMPACKED {
  char deviceName[AMCOM_MAX_DEVICE_NAME_LEN];
  char deviceAddr[AMCOM_MAX_ADDRESS_LEN];
} AMCOM_IdentifyRequestPayload;


typedef struct AMPACKED {
  char deviceName[AMCOM_MAX_DEVICE_NAME_LEN];
  char deviceAddr[AMCOM_MAX_ADDRESS_LEN];
  char deviceState;
  bool empty;
} AMCOM_IdentifyResponsePayload;

typedef struct AMPACKED {
  //uint8_t test_number;
  uint8_t packet_number;
} AMCOM_RTT_RequestPayload;

typedef struct AMPACKED {
  //uint8_t test_number;
  uint8_t packet_nunmber;
  uint64_t current_time;
  bool sync_state;
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
  uint16_t recv_packets[AMCOM_MAX_PDR_TEST];
} AMCOM_PDR_ResponsePayload;

typedef struct AMPACKED {
  uint8_t test_number;
} AMCOM_RSSI_RequestPayload;

typedef struct AMPACKED {
  uint8_t Neighbor_ID;
  uint16_t RSSI;
} AMCOM_RSSI;

typedef struct AMPACKED {
  AMCOM_RSSI neighbor_rssi [AMCOM_MAX_NEIGHBOR];
} AMCOM_RSSI_ResponsePayload;

#endif
