#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#include <openssl/sha.h>
#include <stdint.h>
#include "NibbleAndAHalf/NibbleAndAHalf/base64.h"
#define MAX 5000
#define PORT 8080
#define SA struct sockaddr
#define GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

#define bool _Bool
#define true 1
#define false 0

size_t parse_html_headers(char* raw_headers, char names[][500], char values[][500])
{
    printf("Raw headers: %s\n", raw_headers);
    char* line_save;
    char* line = strtok_r(raw_headers, "\n", &line_save);

    int index = 0;
    while (line != NULL)
    {
        printf("Line!\n");
        printf("%s\n", line);
        char name[500] = {0};
        char value[500] = {0};

        if (index != 0)
        {
            bool bad = false;

            char* split_line = strtok(line, ":");
            if (split_line != NULL)
                strcpy(name, split_line);
            else
                bad = true;

            split_line = strtok(NULL, "\r\n");
            if (split_line != NULL)
                // Because first character is always space
                strcpy(value, &(split_line[1]));
            else
                bad = true;

            if (!bad)
            {
                strcpy(names[index], name);
                strcpy(values[index], value);
            }
        }

        line = strtok_r(NULL, "\r\n", &line_save);
        index++;
    }

    return index;
}

void get_Sec_WebSocket_Key(size_t num_headers, char names[][500], char values[][500], char* out)
{
    printf("Num headers: %lu\n", num_headers);
    for (int i = 0; i < num_headers; i++)
    {
        printf("Value comparing: %s\n", names[i]);
        if (strcmp(names[i], "Sec-WebSocket-Key") == 0)
        {
            strcpy(out, values[i]);
            return;
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
    char* encoded = base64(hash, SHA_DIGEST_LENGTH, &flen);

    // Output the final accept key to Sec_WebSocket_Accept
    for (int i = 0; i < flen; i++)
    {
        Sec_WebSocket_Accept[i] = encoded[i];
    }
    // strcpy(Sec_WebSocket_Accept, encoded);
}

void generate_response(char* Sec_WebSocket_Accept, char* response)
{
    char start_of_response[1000] = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";
    char end_of_response[1000] = "\r\nSec-WebSocket-Protocol: chat\r\n\r\n";

    strcpy(response, start_of_response);
    strcat(response, Sec_WebSocket_Accept);
    strcat(response, end_of_response);
}

// I used code from this website as a base: https://www.geeksforgeeks.org/c/tcp-server-client-implementation-in-c/
void func(int connfd)
{
    bool handshake = true;
    unsigned char buff[MAX];
    int n;
    // infinite loop for chat
    for (;;) {
        bzero(buff, MAX);

        // read the message from client and copy it in buffer
        read(connfd, buff, sizeof(buff));
        // print buffer which contains the client contents
        // printf("From client: %s\nTo client : ", buff);

        if (handshake)
        {
            // Names and values of headers
            char names[500][500];
            char values[500][500];
            size_t num_headers = parse_html_headers(buff, names, values);

            // Get WebSocket key from parsed headers
            char Sec_WebSocket_Key[500];
            get_Sec_WebSocket_Key(num_headers, names, values, Sec_WebSocket_Key);
            printf("KEY HERE: %s\n", Sec_WebSocket_Key);

            // Generate accept key from WebSocket key
            unsigned char Sec_WebSocket_Accept[500];
            generate_Sec_WebSocket_Accept(Sec_WebSocket_Key, Sec_WebSocket_Accept);
            printf("ACCEPT KEY: %s\n", Sec_WebSocket_Accept);

            char accept_response[1000];
            generate_response(Sec_WebSocket_Accept, accept_response);

            printf("%s\n", accept_response);

            write(connfd, accept_response, strlen(accept_response));

            handshake = false;
        }
        else
        {
            printf("BYTES:\n\n");
            for (int j = 0; j < 16; j++)
            {
                for (int i = 0; i < 4; i++)
                {
                    printf("0x%01x ", buff[i + (4 * j)]);
                }
                printf("\n");
            }
            printf("\n\n");

            bool FIN, RSV1, RSV2, RSV3;
            u_int8_t opcode;
            bool MASK;
            u_int8_t payload_len;
            u_int64_t ext_payload_len;
            u_int32_t masking_key;
            unsigned char* payload_data;


        }

        // bzero(buff, MAX);
        // n = 0;
        // copy server message in the buffer
        // while ((buff[n++] = getchar()) != '\n');

        // and send that buffer to client
        // write(connfd, buff, sizeof(buff));

        // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("exit", buff, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }
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

    // Fix from: https://stackoverflow.com/questions/577885/what-are-the-use-cases-of-so-reuseaddr
    int one = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

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
