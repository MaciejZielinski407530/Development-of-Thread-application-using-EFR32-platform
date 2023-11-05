#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "udp_config.h"
#include "app_function.h"

#define MAX_BUFFER_SIZE 1024

extern void recv_function(void *arg);

int main(void){
    char buff[MAX_BUFFER_SIZE];
    pthread_t recv_thread;
    

    initUdp();

    if(pthread_create(&recv_thread, NULL, recv_function,NULL)){
        perror("Error creating recv thread");
        exit(1);
    }

    while(1){
        printf("Wybierz test: ");
        fgets(buff, sizeof(buff), stdin);

        if(strcmp(buff,"Test Identify\n")==0){
            printf("Wybrano Identify \n");
        }
        else if(strcmp(buff,"Test 1\n")==0){
            printf("Wybrano Test 1 \n");
            pdr_test();
        }
        else if(strcmp(buff,"Test 2\n")==0){
            printf("Wybrano Test 2 \n");
            rtt_test();
        }
        else if(strcmp(buff,"Test 3\n")==0){
            printf("Wybrano Test 3 \n");
        }
        else if(strcmp(buff,"Test 4\n")==0){
            printf("Wybrano Test 4 \n");
        }
        else if(strcmp(buff,"Test 5\n")==0){
            printf("Wybrano Test 5 \n");
        }
        else{
            printf("Bledny wybor\n");
        }


    }


    pthread_exit(NULL);



    return 0;
}