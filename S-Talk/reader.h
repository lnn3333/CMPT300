#ifndef _READER_H_
#define _READER_H_

#include "list.h"

void Reader_init(List* myList);
void Reader_cancel();
void Reader_shutdown();

#endif