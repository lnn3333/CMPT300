#ifndef _HELPER_H
#define _HELPER_H

#include "list.h"

int addMessage(List* l, char* msg);
char* removeMessage(List* l);
int getNumberOfMessages(List* l);

#endif