**Aktualizacja 07.11.2023**
Internal Device:
- dodany pomiar RSSI dla wiadomości UDP przychodzących (network_config.c)
- pobranie identyfikatora RLOC16 i wysłanie go jako nazwa urządzenia w pakiecie służącym do identyfikacji (app_function.c)

External Device:
- dodanie możliwości uruchimienia danego testu z parametrami: (ilość testów, ilość pakietów w jednym teście, nazwa badanego urządzenia) (main.c)
- możliwość wypisania danych o podłaczonych urządzeniach (main.c)
- poprawienie funkcji pdr_test,() rtt_test(), search_addr() (app_function.c)
- dodanie funkcji dev_list() - wypisuje dane o podłaczonych urządzeniach (app_function.c)
- dodanie funkcji find_dev() - sprawdzenie czy urządzenie o podanej nazwie jest w tablicy urządzeń podłączonych, zwraca pozycje w tablicy (app_function.c)
