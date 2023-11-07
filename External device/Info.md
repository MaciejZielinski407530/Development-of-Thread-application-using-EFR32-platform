Informacje co znajduje się w plikach

**amcom.h oraz amcom.c**
- Implementacja AMCOM

**amcom_packets.h**
- Deklaracja AMCOM_PacketType
- Deklaracje struktur danych dla pkaietów AMCOM
- Definicje preprocesora określające: ( Maks. długość nazwy urządzenia, Maks. liczba testów oraz pakietów dla testu PDR oraz RTT, 
                                        Maks. liczba urządzeń w sieci, Maks. długość adresu, Adres Multicast)

**app_function.h oraz app_function.c**
Funkcje wykorzystwyane w aplikacji:
- amcomPacketHandler() - obsługa otrzymywanych pakietów
- pdr_test() - funkcja uruchamiajaca test PDR (Packet Delivery Ratio)
- rtt_test() - funkcja uruchamiajaca test RTT (Round Trip Time)
- search_addr() - funkcja pobierająca adres IPv6 interfejsu aplikacji

**udp_config.h oraz udp_config.c**
Obsługa UDP
- void *recv_function(void *arg) - osobny wątek służący wyłącznie do ciągłego odbioru pakietów (Dodatkowo inicjalizacja gniazda do odbioru)
- initUdp - inicjalizacja gniazda służącego do wysyłania pakietów
- UDPsend - funkcja służąca do wysyłania pakietów

**main.c**
- wywołuje initUDP()
- tworzy wątek do odbioru pakietów
- pętla główna - wybór rodzaju testu
