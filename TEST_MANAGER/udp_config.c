#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <ifaddrs.h>
#include "udp_config.h"
#include <asm-generic/socket.h>

 
#define SendPORT 8888
#define RecvPORT 8080

/// @brief  Searching for a global IPv6 address
void find_addr(void);
 
static int send_sock;   // Socket to sending message
static struct sockaddr_in6 send_appAddr;    // Socket address structure


/// Thread to listen on RecvPORT
void *recv_function(void *arg){
    int recv_sock;
    struct sockaddr_in6 recv_appAddr, recv_clientAddr;
    socklen_t addr_len = sizeof(recv_clientAddr);
    int bytesReceived;
    static uint8_t buf [TEST_MAX_PACKET_SIZE];

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
        udpPacketHandler((uint8_t* )buf, bytesReceived, clientIP);  // Call Packet handler

    }
    close(recv_sock);
    pthread_exit(NULL);
}

/// Socket to sending UDP messages initialization 
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
    find_addr();  // Search a global IPv6 address for a specific interface
    
    // Bind the interface to the socket
    if(setsockopt(send_sock, SOL_SOCKET, SO_BINDTODEVICE, "wpan0", strlen("wpan0"))<0){
        perror("Błąd podczas ustawiania opcji SO_BINDTODEVICE\n");
        exit(1);
    }
    
    // Bind the address to the socket
    if (bind(send_sock, (struct sockaddr *)&send_appAddr, sizeof(send_appAddr)) < 0) {
        perror("Błąd podczas bindowania gniazda");
        exit(1);
    }
}
 
/// Function to sending UDP message
  void UDPsend (uint8_t* buf, size_t PayloadSize, const char* send_addr){
     
    struct sockaddr_in6 clientAddr; 
    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin6_family = AF_INET6;
    clientAddr.sin6_port = htons(RecvPORT);
    
    if (inet_pton(AF_INET6, send_addr, &(clientAddr.sin6_addr)) <= 0) {
        perror("Błąd podczas konwersji adresu IP");
        exit(1);
    }

    
    size_t bytesToSent = sendto(send_sock, (const char*) buf, PayloadSize, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
        if (bytesToSent < 0) {
            perror("Błąd podczas wysyłania wiadomości");
            exit(1);
        }

}
 
void find_addr(void){
    struct ifaddrs *ifaddr, *ifa;
 
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(1);
    }
 
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET6 && strcmp(ifa->ifa_name,"enp0s8")==0) {           
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)ifa->ifa_addr;

            if(!(ipv6->sin6_scope_id ==0)){ // Check if address is a global
                continue;
            }
            send_appAddr.sin6_addr = ipv6->sin6_addr;
            break;
        }
    }
    freeifaddrs(ifaddr);
}
 