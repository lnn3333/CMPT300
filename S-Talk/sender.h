#ifndef _SENDER_H
#define _SENDER_H

#include "list.h"

void Sender_init(List* list, char* hostname, char* port);
void Sender_signaller();
void Sender_cancel();
void Sender_shutdown();

#endif