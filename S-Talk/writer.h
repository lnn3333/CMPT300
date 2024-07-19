#ifndef _WRITER_H
#define _WRITER_H

#include "list.h"

void Writer_init(List* l);
void Writer_signaller();
void Writer_cancel();
void Writer_shutdown();

#endif