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
            printf("Choice: LIST \n");
            dev_list();
            continue;
        }

        // Start Packet Delivery Ratio test
        if(strcmp(test_name,"PDR") == 0 && sscanf(buff, "%s %d %d %s\n", test_name, &number_of_tests, &number_of_packet, device_name) == 4){
            printf("Choice: Test PDR \n");
            if(AMCOM_MAX_PDR_PACKET < number_of_packet || AMCOM_MAX_PDR_TEST < number_of_tests ){
                printf("Error: Bad argumen\nt");
                continue;
            }
            int it = find_dev(device_name);
            if(it >= 0 ){
                pdr_test(number_of_tests, number_of_packet, it);
            } else{
                printf("Device do not exist\n");
            }
            continue;
        }

        // Start Round Trip Time test
        if(strcmp(test_name,"RTT")==0 && sscanf(buff, "%s %d %d %s\n", test_name, &number_of_tests, &number_of_packet, device_name) == 4){
            printf("Choice: Test RTT \n");
            if(AMCOM_MAX_RTT_PACKET < number_of_packet || AMCOM_MAX_RTT_TEST < number_of_tests ){
                printf("Error: Bad argument\n");
                continue;
            }
            int it = find_dev(device_name);
            if(it >= 0 ){
                rtt_test(number_of_tests, number_of_packet, it);
            } else{
                printf("Device do not exist\n");
            }
            continue;
        }

        // Start RSSI test
        if(strcmp(test_name,"RSSI") == 0 && sscanf(buff, "%s %d %d %s\n", test_name, &number_of_tests, &number_of_packet, device_name) == 4){
            printf("Choice: Test RSSI \n");
            if(AMCOM_MAX_RSSI_PACKET < number_of_packet || AMCOM_MAX_RSSI_TEST < number_of_tests ){
                printf("Error: Bad argument\n");
                continue;
            }
            int it = find_dev(device_name);
            if(it >= 0 ){
                rssi_test(number_of_tests, number_of_packet, it);
            } else{
                printf("Device do not exist\n");
            }
            continue;
        }

        // Start Throughput test
        if(sscanf(buff, "%s %d %s\n", test_name, &number_of_packet, device_name) == 3 && strcmp(test_name,"THR")==0 ){
            printf("Choice: Test Throughput \n");
            if(THROUGHPUT_MIN_PAYLOAD != number_of_packet && THROUGHPUT_MID_PAYLOAD != number_of_packet && THROUGHPUT_MAX_PAYLOAD != number_of_packet ){
                printf("Error: Bad argument\n");
                continue;
            }
            int it = find_dev(device_name);
            if(it >= 0 ){
                thr_test(number_of_packet, it);
            } else{
                printf("Device do not exist\n");
            }
            continue;
        }

        // Start Ton test
        if(sscanf(buff, "%s %s\n", test_name, device_name) == 2 && strcmp(test_name,"TON")==0 ){
            printf("Choice: Test Ton \n");
            int it = find_dev(device_name);
            if(it >= 0 ){
                ton_test(it);
            } else{
                printf("Device do not exist\n");
            }
            continue;
        }

                
        printf("Error: Wrong choice\n");
                

    }


    return 0;
}