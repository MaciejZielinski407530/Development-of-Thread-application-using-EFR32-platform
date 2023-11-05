**AKTUALNIE NIC NIE JEST MODYFIKOWANE**

Informacje co znajduje się w plikach

**amcom.h oraz amcom.c**
- Implementacja AMCOM

**amcom_packets.h**
- Deklaracja AMCOM_PacketType
- Deklaracje struktur danych dla pkaietów AMCOM
- Definicje preprocesora określające: ( Maks. długość nazwy urządzenia, Maks. liczba testów oraz pakietów dla testu PDR, 
                                        Maks. liczba urządzeń w sieci, Maks. długość adresu, Adres Multicast)

**fun_main.h oraz fun_main.c**
Funkcje wykorzystwyane w aplikacji:
- amcomPacketHandler() - obsługa otrzymywanych pakietów
- identify_request() - funkcja wysyłająca swoją Nazwe, Adres IPv6 na adres MULTICAST, aby pozostałe urządzenia wsieci zidentyfikowały główne urządzenie i odesłały swój adres i nazwe
- pdr_test() - wywołanie testu PDR
- rtt_test() - wywołanie testu RTT


**network_config.c**
- setNetworkConfiguratio() - incjalizacja sieci Thread
- initUdp() - incjalizacja gniazda dla komunikacji UDP
- UDPsend() - funkcja do wysyłania pakietów UDP
- mtdReceiveCallback() - obsługa odebranych pakietów UDP

**app.h oraz app.c**
- incjalizacje systemowe dla OpenThread
- inicjalizacja UDP, sieci Thread,  SysTick
- app_process_action() - funkcja wywoływana przez pętle główną w main.c

**main.c**
- inicjalizacje systemowe
- pętla główna
