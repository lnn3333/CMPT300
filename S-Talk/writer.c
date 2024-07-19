#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "writer.h"
#include "list.h"
#include "helper.h"

#define MSG_MAX_LEN 1472

static pthread_t writerThread;
static List *list;
static char* message;

static pthread_mutex_t s_syncOkToWriteMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_syncOkToWriteCondVar = PTHREAD_COND_INITIALIZER;

void *writeThread(void *unused)
{
   while(1)
   {
        pthread_mutex_lock(&s_syncOkToWriteMutex);
        {
            pthread_cond_wait(&s_syncOkToWriteCondVar, &s_syncOkToWriteMutex);
        }
        pthread_mutex_unlock(&s_syncOkToWriteMutex);

        int i = 0;
        do
        {
            i++;

            message = removeMessage(list);
            if (message == NULL)
            {
                fprintf(stderr, "Writer: remove message error\n");
                break;
            }

            int writeResult = write(1, message, strlen(message));
            if (writeResult == -1)
            {
                perror("Writer: write() error\n");
                exit(1);
            }

            if (!strcmp(message, "!\n") && i == 1)
            {
                free(message);
                message = NULL;
                return NULL;
            }

            free(message);
            message = NULL;
        } while (getNumberOfMessages(list) != 0);
   }
   
   return NULL;
}

void Writer_init(List *l)
{
    list = l;
    pthread_create(&writerThread, NULL, writeThread, NULL);
}

void Writer_signaller()
{
    // Signal the writer, called by receiver
    pthread_mutex_lock(&s_syncOkToWriteMutex);
    {
        pthread_cond_signal(&s_syncOkToWriteCondVar);
    }
    pthread_mutex_unlock(&s_syncOkToWriteMutex);
}

void Writer_cancel()
{
    pthread_cancel(writerThread);
}

void Writer_shutdown()
{
    pthread_join(writerThread, NULL);
}