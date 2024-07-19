#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "list.h"
#include "helper.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int addMessage(List* list, char* message)
{
    int result;
    
    pthread_mutex_lock(&mutex);
    {
        result = List_prepend(list, message);
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

char* removeMessage(List* list)
{
    char* message;
    
    pthread_mutex_lock(&mutex);
    {
        message = List_trim(list);
    }
    pthread_mutex_unlock(&mutex);

    return message;
}

int getNumberOfMessages(List* list)
{
    int count = 0;
    pthread_mutex_lock(&mutex);
    {
        count = List_count(list);
    }
    pthread_mutex_unlock(&mutex);

    return count;
}