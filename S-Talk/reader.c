#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "list.h"
#include "helper.h"

#include "reader.h"
#include "sender.h"
#include "receiver.h"
#include "writer.h"

#define MSG_MAX_LEN 1472

static pthread_t readerThread;
static List* list;

void *readThread(void *unused) 
{
    while(1) {
        char* message;
        char bufferStorage[MSG_MAX_LEN];
        int numberOfBytes;
        int numberOfReadAttempts = 0;

        do
        {
            numberOfReadAttempts++;
            memset(&bufferStorage, 0, MSG_MAX_LEN);
            numberOfBytes = read(0, bufferStorage, MSG_MAX_LEN);

            if (numberOfBytes == -1) {
                perror("Reader: Fail to read user input from keyboard!\n");
                exit(-1);
            }

            message = (char*)malloc(sizeof(char)*(numberOfBytes + 1));
            strncpy(message, bufferStorage, numberOfBytes);
            message[numberOfBytes] = '\0';

            int addedResult = addMessage(list, message);
            if (addedResult == -1) {
                fprintf(stderr, "Reader: Fail to enqueue message to the list\n");
            }

            if (!strcmp(message, "!\n") && numberOfReadAttempts == 1)
            {
                Sender_signaller();
                Receiver_cancel();
                Writer_cancel();
                return NULL;
            }
        } while (bufferStorage[numberOfBytes - 1] != '\n');
    
        // Signals sender to send message
        Sender_signaller();
    }
    
    return NULL;
}

void Reader_init(List* l)
{
    list = l;

    pthread_create(&readerThread, NULL, readThread, NULL);              
}

void Reader_cancel()
{
    pthread_cancel(readerThread);
}

void Reader_shutdown(void)
{
    pthread_join(readerThread, NULL);
}