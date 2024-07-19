// References: Brian Fraser's Workshops & Beej's Guide to Network Programming

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "list.h"
#include "helper.h"

#include "receiver.h"
#include "writer.h"
#include "reader.h"
#include "sender.h"

#define MSG_MAX_LEN 1472

static pthread_t receiverThread;

static List *list;
static char *myPort;

static int sockfd;
static struct addrinfo *servinfo;

void *receiveThread(void *unused)
{
    struct addrinfo hints, *p;
    int getAddrInfoResult;
    int bindResult;
    int numberOfBytes;
    char buffer[MSG_MAX_LEN];
    char *message;
    struct sockaddr_in remoteAddr;
    socklen_t addrLength;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    getAddrInfoResult = getaddrinfo(NULL, myPort, &hints, &servinfo);
    if (getAddrInfoResult != 0)
    {
        fprintf(stderr, "Receiver: getaddrinfo : %s\n", gai_strerror(getAddrInfoResult));
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
        {
            perror("Receiver: socket() error");
            continue;
        }

        bindResult = bind(sockfd, p->ai_addr, p->ai_addrlen);
        if (bindResult == -1)
        {
            perror("Receiver: bind() error");
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "Receiver: Failed to bind socket\n");
        freeaddrinfo(servinfo);
        exit(2);
    }

    freeaddrinfo(servinfo);

    while (1)
    {
        int i = 0;
        do
        {
            i++;
            memset(&buffer, 0, MSG_MAX_LEN);

            addrLength = sizeof(remoteAddr);
            numberOfBytes = recvfrom(sockfd, buffer, MSG_MAX_LEN, 0, (struct sockaddr *)&remoteAddr, &addrLength);

            if (numberOfBytes == -1)
            {
                perror("Receiver: recvfrom() error\n");
                exit(1);
            }

            message = (char *)malloc(sizeof(char) * (numberOfBytes + 1));
            strncpy(message, buffer, numberOfBytes);
            message[numberOfBytes] = '\0';

            int addedResult = addMessage(list, message);
            if (addedResult == -1)
            {
                fprintf(stderr, "Receiver: Fail to add message to the list\n");
            }

            if (!strcmp(message, "!\n") && i == 1)
            {
                Writer_signaller();
                Reader_cancel();
                Sender_cancel();
                return NULL;
            }
        } while (buffer[numberOfBytes - 1] != '\n' && i != LIST_MAX_NUM_NODES);

        // Signals writer to write message to screen
        Writer_signaller();
    }

    return NULL;
}

void Receiver_init(List *l, char *myP)
{
    list = l;
    myPort = myP;

    pthread_create(&receiverThread, NULL, receiveThread, NULL);
}

void Receiver_cancel()
{
    pthread_cancel(receiverThread);
}

void Receiver_shutdown(void)
{
    close(sockfd);

    pthread_join(receiverThread, NULL);
}