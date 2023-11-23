#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "amcom.h"

/// Start of packet character
const uint8_t  AMCOM_SOP         = 0xA1;
const uint16_t AMCOM_INITIAL_CRC = 0xFFFF;

static uint16_t AMCOM_UpdateCRC(uint8_t byte, uint16_t crc)
{
	byte ^= (uint8_t)(crc & 0x00ff);
	byte ^= (uint8_t)(byte << 4);
	return ((((uint16_t)byte << 8) | (uint8_t)(crc >> 8)) ^ (uint8_t)(byte >> 4) ^ ((uint16_t)byte << 3));
}


void AMCOM_InitReceiver(AMCOM_Receiver* receiver, AMCOM_PacketHandler packetHandlerCallback, void* userContext) {
	assert(receiver);
	assert(packetHandlerCallback);

	receiver->packetHandler = packetHandlerCallback;
	receiver->payloadCounter = 0;
	receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
	receiver->userContext = userContext;
}

size_t AMCOM_Serialize(uint8_t packetType, const void* payload, size_t payloadSize, uint8_t* destinationBuffer) {
    assert(packetType);
    assert(destinationBuffer);
    size_t i;
    destinationBuffer[0] = AMCOM_SOP;     // First byte   SOP
    destinationBuffer[1] = packetType;    // Second byte  TYPE
    destinationBuffer[2] = payloadSize;   // Third byte   LENGTH
    //CRC counting start
    uint16_t crc = AMCOM_UpdateCRC(packetType, AMCOM_INITIAL_CRC);
    crc = AMCOM_UpdateCRC(payloadSize, crc);
    if (0 != payloadSize) {
        for (int i = 0; i < payloadSize; i++) {
            crc = AMCOM_UpdateCRC(*((const char*)payload + i), crc);
        }
    }
    //CRC counting stop
    destinationBuffer[3] = crc & 0x00ff;    // CRC LSB
    destinationBuffer[4] = crc >> 8;        // CRC MSB

    if (0 == payloadSize) {                 // Jesli LENGTH == 0 to pakiet ma rozmiar 5B
        return 5;
    }

    for (i = 5; i < (payloadSize + 5); i++) {  // Jesli LENGTH > 0 to dodanie danych do pakietu
        destinationBuffer[i] = *((const char*)payload + i - 5);

    }

    return i;
}

void AMCOM_Deserialize(AMCOM_Receiver* receiver, const void* data, size_t dataSize) {
    assert(receiver);

    for (size_t i = 0; i < dataSize; i++) {
        // Stan EMPTY - odebrano SOP
        if (*((const uint8_t*)data + i) == AMCOM_SOP && receiver->receivedPacketState == AMCOM_PACKET_STATE_EMPTY) {
            receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_SOP;
            receiver->receivedPacket.header.sop = AMCOM_SOP;
        }
        // Stan GOT_SOP - odebrano kolejny bajt
        else if (receiver->receivedPacketState == AMCOM_PACKET_STATE_GOT_SOP) {
            receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_TYPE;
            receiver->receivedPacket.header.type = *((const uint8_t*)data + i);
        }
        // Stan GOT_TYPE - odebrano kolejny bajt (Wartosc LENGTH musi byc z przedzialu <0,200>)
        else if (receiver->receivedPacketState == AMCOM_PACKET_STATE_GOT_TYPE) {
            if (200 >= *((const uint8_t*)data + i)) {
                receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_LENGTH;
                receiver->receivedPacket.header.length = *((const uint8_t*)data + i);
            }
            else {
                receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
            }
        }
        // Stan GOT_LENGTH - odebrano kolejny bajt (CRC LSB)
        else if (receiver->receivedPacketState == AMCOM_PACKET_STATE_GOT_LENGTH) {
            receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_CRC_LO;
            receiver->receivedPacket.header.crc = *((const uint8_t*)data + i);
        }
        // Stan GOT_CRC_LO - odebrano kolejny bajt (CRC MSB)
        // Jesli LENGTH == 0, Stan -> GOT_WHOLE_PACKET i sprawdz CRC oraz wywolaj Handler
        else if (receiver->receivedPacketState == AMCOM_PACKET_STATE_GOT_CRC_LO) {
            if (receiver->receivedPacket.header.length == 0) {
                receiver->receivedPacket.header.crc |= *((const uint8_t*)data + i) << 8;
                receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_WHOLE_PACKET;
                // Count and check CRC start
                uint16_t crc = AMCOM_UpdateCRC(receiver->receivedPacket.header.type, AMCOM_INITIAL_CRC);
                crc = AMCOM_UpdateCRC(receiver->receivedPacket.header.length, crc);

                if (crc == receiver->receivedPacket.header.crc) {
                    receiver->payloadCounter = 0;
                    receiver->packetHandler(&receiver->receivedPacket, receiver->userContext);   // Handler
                }
                receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
                // Count and check CRC stop
            }
            else {
                receiver->receivedPacketState = AMCOM_PACKET_STATE_GETTING_PAYLOAD;
                receiver->receivedPacket.header.crc |= *((const uint8_t*)data + i) << 8;
            }

        }
        // Stan GETTING_PAYLOAD - odbieranie kolejnych bajtow danych
        else if (receiver->receivedPacketState == AMCOM_PACKET_STATE_GETTING_PAYLOAD) {

            receiver->receivedPacket.payload[receiver->payloadCounter++] = *((const uint8_t*)data + i);

            // Uzyskano odpowiednia ilosc danych
            if (receiver->payloadCounter == receiver->receivedPacket.header.length) {
                receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_WHOLE_PACKET;
                // Count and check CRC start
                uint16_t crc = AMCOM_UpdateCRC(receiver->receivedPacket.header.type, AMCOM_INITIAL_CRC);
                crc = AMCOM_UpdateCRC(receiver->receivedPacket.header.length, crc);
                if (0 != receiver->receivedPacket.header.length) {
                    for (int i = 0; i < receiver->receivedPacket.header.length; i++) {
                        crc = AMCOM_UpdateCRC(receiver->receivedPacket.payload[i], crc);
                    }
                }

                if (receiver->receivedPacket.header.crc == crc) {
                    receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
                    receiver->packetHandler(&receiver->receivedPacket, receiver->userContext);   // Handler

                }
                else
                    receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;

                // Count and check CRC stop
                receiver->payloadCounter = 0;
            }
        }
    }
}
