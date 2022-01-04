#include "serverthread.h"
#include <WinSock2.h>
#include <stdio.h>
#define BUFFER_SIZE 512

unsigned int __stdcall serverTherad(void* data) {

    int port = *(int*)data;

    WSADATA wsa;
    SOCKET s, new_socket;
    struct sockaddr_in server, client;
    int c;
    char dataBuffer[BUFFER_SIZE];

    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    printf("Initialised.\n");

    //Create a socket
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d", WSAGetLastError());
    }

    printf("Socket created.\n");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    //Bind
    if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code : %d", WSAGetLastError());
    }

    puts("Bind done");
    printf("Port: %d\n", port);

    //Listen to incoming connections
    listen(s, 1);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");

    c = sizeof(struct sockaddr_in);
    new_socket = accept(s, (struct sockaddr*)&client, &c);
    if (new_socket == INVALID_SOCKET)
    {
        printf("accept failed with error code : %d", WSAGetLastError());
    }


    unsigned long l;
    ioctlsocket(new_socket, FIONREAD, &l);

    puts("Connection accepted");

    //Reply to client
    //strcpy(dataBuffer, (char*)"Hello Client , I have received your connection. But I have to go now, bye\n");
    //send(new_socket, (char*)dataBuffer, strlen(dataBuffer), 0);

    int iResult;

    while (true) {

        iResult = recv(new_socket, dataBuffer, BUFFER_SIZE, 0);

        if (iResult > 0)
        {
            dataBuffer[iResult] = '\0';
            printf("Message received from client %s:\n",dataBuffer);
            printf("Enter za dalje...\n");
            getchar();
            strcpy(dataBuffer, (char*)"Da\n");
            iResult = send(new_socket, (char*)&dataBuffer, strlen(dataBuffer), 0);

            // Check result of send function
            if (iResult == SOCKET_ERROR)
            {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(new_socket);
                WSACleanup();
                return 1;
            }

            printf("Message successfully sent. Total bytes: %ld\n", iResult);

        }
        else if (iResult == 0)
        {
            // connection was closed gracefully
            printf("Connection with client closed.\n");
            closesocket(new_socket);
            break;
        }
        else
        {
            // there was an error during recv
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(new_socket);
            break;
        }
    }

    getchar();

    closesocket(s);
    WSACleanup();

    return 0;
}