#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "udp_config.h"
#include "app_function.h"
#include "amcom_packets.h"

// Maximum length of input buffer
#define MAX_BUFFER_SIZE 100
// Maximum length of test name
#define MAX_TEST_NAME_SIZE 10


extern void recv_function(void *arg);

int main(void){
    char buff[MAX_BUFFER_SIZE];
    char test_name[MAX_TEST_NAME_SIZE];
    int  number_of_tests, number_of_packet;
    char device_name [AMCOM_MAX_DEVICE_NAME_LEN];
    pthread_t recv_thread;
    

    initUdp();

    // Start receiving thread
    if(pthread_create(&recv_thread, NULL, recv_function,NULL)){
        perror("Error creating recv thread");
        exit(1);
    }

    while(1){
        printf("Uruchom test: ");
        fgets(buff, sizeof(buff), stdin);

        // Print list of connected device
        if(sscanf(buff, "%s\n", test_name) == 1 && strcmp(test_name,"LIST") == 0){
            printf("Wybrano LIST \n");
            dev_list();
            continue;
        }

        // Start Packet Delivery Ratio test
        if(strcmp(test_name,"PDR") == 0 && sscanf(buff, "%s %d %d %s\n", test_name, &number_of_tests, &number_of_packet, device_name) == 4){
            printf("Wybrano Test 1 \n");
            int it = find_dev(device_name);
            if(it >= 0 ){
                pdr_test(number_of_tests, number_of_packet, it);
            } else{
                printf("Urzadzenie o podanej nazwie nie istnieje\n");
            }
            continue;
        }

        // Start Round Trip Time test
        if(strcmp(test_name,"RTT")==0 && sscanf(buff, "%s %d %d %s\n", test_name, &number_of_tests, &number_of_packet, device_name) == 4){
            printf("Wybrano Test 2 \n");
            int it = find_dev(device_name);
            if(it >= 0 ){
                rtt_test(number_of_tests, number_of_packet, it);
            } else{
                printf("Urzadzenie o podanej nazwie nie istnieje\n");
            }
            continue;
        }

        // Start RSSI test
        if(strcmp(test_name,"RSSI") == 0 && sscanf(buff, "%s %d %d %s\n", test_name, &number_of_tests, &number_of_packet, device_name) == 4){
            printf("Wybrano Test 1 \n");
            int it = find_dev(device_name);
            if(it >= 0 ){
                rssi_test(number_of_tests, number_of_packet, it);
            } else{
                printf("Urzadzenie o podanej nazwie nie istnieje\n");
            }
            continue;
        }

        // Start Throughput test
        if(sscanf(buff, "%s %d %s\n", test_name, &number_of_packet, device_name) == 3 && strcmp(test_name,"THR")==0 ){
            printf("Wybrano Test 3 \n");
            int it = find_dev(device_name);
            if(it >= 0 ){
                thr_test(number_of_packet, it);
            } else{
                printf("Urzadzenie o podanej nazwie nie istnieje\n");
            }
            continue;
        }

                
        printf("Bledny wybor\n");
                

    }


    return 0;
}