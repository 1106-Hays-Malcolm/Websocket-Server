#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#include <openssl/sha.h>
#include "NibbleAndAHalf/NibbleAndAHalf/base64.h"
#define MAX 80
#define PORT 8080
#define SA struct sockaddr
#define GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

// I used code from this website as a base: https://www.geeksforgeeks.org/c/tcp-server-client-implementation-in-c/
void func(int connfd)
{
    char buff[MAX];
    int n;
    // infinite loop for chat
    for (;;) {
        bzero(buff, MAX);

        // read the message from client and copy it in buffer
        read(connfd, buff, sizeof(buff));
        // print buffer which contains the client contents
        printf("From client: %s\t To client : ", buff);
        bzero(buff, MAX);
        n = 0;
        // copy server message in the buffer
        while ((buff[n++] = getchar()) != '\n');

        // and send that buffer to client
        write(connfd, buff, sizeof(buff));

        // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("exit", buff, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }
}

// Generates a key to prove to the client that the server supports the Websocket protocol
void generate_Sec_WebSocket_Accept(char* Sec_WebSocket_Key, unsigned char* Sec_WebSocket_Accept)
{
    // Concatenate the client's key with the GUID
    char concat[200];
    strcpy(concat, Sec_WebSocket_Key);
    strcat(concat, GUID);

    // Hash the concatenated string with SHA1
    unsigned char hash[SHA_DIGEST_LENGTH];
    size_t concat_len = strlen(concat);
    SHA1(concat, concat_len, hash);

    // Encode the hash with Base64
    int flen;
    char* encoded = base64(hash, strlen(hash), &flen);

    // Output the final accept key to Sec_WebSocket_Accept
    strcpy(Sec_WebSocket_Accept, encoded);
}

// Driver function
int main()
{
    unsigned char accept_key[200];
    generate_Sec_WebSocket_Accept("dGhlIHNhbXBsZSBub25jZQ==", accept_key);
    printf("%s\n", accept_key);
    // for(int i = 0; i < strlen(accept_key); i++)
    // {
    //     printf("0x%02x\n", accept_key[i]);
    // }

    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");

    // Function for chatting between client and server
    func(connfd);

    // After chatting close the socket
    close(sockfd);
}
