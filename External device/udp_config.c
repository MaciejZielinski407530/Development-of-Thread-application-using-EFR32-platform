 #include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
 
//#include "amcom.h"
#include "udp_config.h"
//#include "app_function.h"
 
 
#define SendPORT 8888
#define RecvPORT 8080
 
 
static int send_sock;
static struct sockaddr_in6 send_appAddr;

/// Thread to listen on receive port
void *recv_function(void *arg){
    int recv_sock;
    struct sockaddr_in6 recv_appAddr, recv_clientAddr;
    socklen_t addr_len = sizeof(recv_clientAddr);
    int bytesReceived;
    static char buf[512];

    static AMCOM_Receiver amcomReceiver;
    AMCOM_InitReceiver(&amcomReceiver, amcomPacketHandler,NULL);

    recv_sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (recv_sock < 0) {
        perror("Błąd podczas tworzenia gniazda");
        exit(1);
    }

    memset(&recv_appAddr, 0, sizeof(recv_appAddr));
    recv_appAddr.sin6_family = AF_INET6;
    recv_appAddr.sin6_port = htons(RecvPORT);
    recv_appAddr.sin6_addr = in6addr_any;
    
    if (bind(recv_sock, (struct sockaddr *)&recv_appAddr, sizeof(recv_appAddr)) < 0) {
        perror("Błąd podczas bindowania gniazda");
        exit(1);
    }
    while(1){
        int bytesReceived = recvfrom(recv_sock, buf, sizeof(buf), 0, (struct sockaddr *)&recv_clientAddr, &addr_len);
        if (bytesReceived < 0) {
            perror("Błąd podczas odbierania wiadomości");
            exit(1);
        }
        char clientIP[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &(recv_clientAddr.sin6_addr), clientIP, INET6_ADDRSTRLEN);
        printf("Odebrano wiadomość od [%s]\n", clientIP);
        AMCOM_Deserialize(&amcomReceiver, buf, bytesReceived);
    }
    close(recv_sock);
    pthread_exit(NULL);
}

/// Initialization socket to sending UDP messages
void initUdp(void)
{
    send_sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (send_sock < 0) {
        perror("Błąd podczas tworzenia gniazda");
        exit(1);
    }

    memset(&send_appAddr, 0, sizeof(send_appAddr));
    send_appAddr.sin6_family = AF_INET6;
    send_appAddr.sin6_port = htons(SendPORT);
    send_appAddr.sin6_addr = in6addr_any;
 
    if (bind(send_sock, (struct sockaddr *)&send_appAddr, sizeof(send_appAddr)) < 0) {
        perror("Błąd podczas bindowania gniazda");
        exit(1);
    }
}
 
/// Function to sending UDP message
  void UDPsend (uint8_t buf[AMCOM_MAX_PACKET_SIZE], const char* send_addr){
     
    struct sockaddr_in6 clientAddr; 
    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin6_family = AF_INET6;
    clientAddr.sin6_port = htons(RecvPORT);
    if (inet_pton(AF_INET6, send_addr, &(clientAddr.sin6_addr)) <= 0) {
        perror("Błąd podczas konwersji adresu IP");
        exit(1);
    }


    
    size_t bytesToSent = sendto(send_sock, (const char*) buf, AMCOM_MAX_PACKET_SIZE, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
        if (bytesToSent < 0) {
            perror("Błąd podczas wysyłania wiadomości");
            exit(1);
        }

}
 
 