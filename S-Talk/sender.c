// References: Brian Fraser's Workshops & Beej's Guide to Network Programming 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "sender.h"
#include "list.h"
#include "helper.h"


static pthread_mutex_t s_syncOkToSendMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_syncOkToSendCondVar = PTHREAD_COND_INITIALIZER;

static pthread_t senderThread;

static int sockfd;
static struct addrinfo *servinfo;

static List* list;
static char* remoteMachineName;
static char* remotePort;

static char* message;

void *sendThread(void *unused)
{
    struct addrinfo hints, *p;
    int rv;
    int numberOfBytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM;

    rv = getaddrinfo(remoteMachineName, remotePort, &hints, &servinfo);
    if (rv != 0)
    {
        fprintf(stderr, "Sender: getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (sockfd == -1) 
        {
            perror("Sender: socket");
            continue;
        }

        break; // if we get here, we must have connected successfully
    }

    if (p == NULL)
    {
        fprintf(stderr, "Sender: Failed to create socket");
        exit(2);
    }

    while(1)
    {
        pthread_mutex_lock(&s_syncOkToSendMutex);
        {
            pthread_cond_wait(&s_syncOkToSendCondVar, &s_syncOkToSendMutex);
        }
        pthread_mutex_unlock(&s_syncOkToSendMutex);
    
        int iteration = 0;
        
        do
        {
            iteration++;

            message = removeMessage(list);
            if (message == NULL)
            {
                fprintf(stderr, "Sender: remove message error\n");
                break;
            }

            numberOfBytes = sendto(sockfd, message, strlen(message), 0, p->ai_addr, p->ai_addrlen);
            if (numberOfBytes == -1)
            {
                perror("Sender: Sendto error");
                exit(2);
            }

            if (!strcmp(message, "!\n") && iteration == 1)
            {
                free(message);
                message = NULL;
                return NULL;
            }

            // Deallocating message
            free(message);
            message = NULL;
        } while (getNumberOfMessages(list) != 0);
    }
    return NULL;
}

void Sender_init(List* l, char* hostname, char* port)
{
    list = l;
    remoteMachineName = hostname;
    remotePort = port;

    pthread_create(&senderThread, NULL, sendThread, NULL);
}

void Sender_signaller()
{
    // Signal other thread
    pthread_mutex_lock(&s_syncOkToSendMutex);
    {
        pthread_cond_signal(&s_syncOkToSendCondVar);
    }
    pthread_mutex_unlock(&s_syncOkToSendMutex);
}

void Sender_cancel()
{
    pthread_cancel(senderThread);
}

void Sender_shutdown()
{
    freeaddrinfo(servinfo);

    close(sockfd);

    pthread_join(senderThread, NULL);
}