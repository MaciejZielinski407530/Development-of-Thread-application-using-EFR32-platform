**Aktualizacja 07.11.2023**

Internal Device:
- dodany pomiar RSSI dla wiadomości UDP przychodzących (network_config.c)
- pobranie identyfikatora RLOC16 i wysłanie go jako nazwa urządzenia w pakiecie służącym do identyfikacji (app_function.c)

External Device:
- dodanie możliwości uruchimienia danego testu z parametrami: (ilość testów, ilość pakietów w jednym teście, nazwa badanego urządzenia) (main.c)
- możliwość wypisania danych o podłaczonych urządzeniach (main.c)
- poprawienie funkcji pdr_test(), rtt_test(), search_addr() (app_function.c)
- dodanie funkcji dev_list() - wypisuje dane o podłaczonych urządzeniach (app_function.c)
- dodanie funkcji find_dev() - sprawdzenie czy urządzenie o podanej nazwie jest w tablicy urządzeń podłączonych, zwraca pozycje w tablicy (app_function.c)

**Aktualizacja 09.11.2023**

Internal Device:
- poprawienie pomiaru RSSI dla wiadomości UDP przychodzących (network_config.c)
- dodanie struktur danych dla RSSI, Ton, Throughput (amcom_packets.h)
- dodana obsługa testów RSSI, Throughput (app_function.h)

External Device:
- dodanie testów rssi_test(), thr_test() (app_function.c / app_function.h)
- dodanie struktur danych dla RSSI, Ton, Throughput (amcom_packets.h)
- dodanie wywyołania dla testów RSSI, Throughput (main.c)

**Aktualizacja 12.11.2023**

Internal Device:
- poprawione dołączanie do sieci (Joiner, Commissioner) (app.c)
- poprawienie procesu identyfikacji (app_function.c / app_function.h)
- Dodanie komentarzy (all)

External Device:
- poprawienie procesu identyfikacji (app_function.c / app_function.h)

**Aktualizacja 14.11.2023**

Internal Device:
- Dodanie pomiar czasu Ton (app.c)
- Obsługa testu Ton (app_function.c / app_function.h)

External Device:
- poprawa testu thr_test() (app_function.c / app_function.h)
- dodanie testu Ton (app_function.c / app_function.h)
- dodanie komentarzy (all)
