#include <stdio.h>
#include <stdlib.h>

#include "list.h"

#include "reader.h"
#include "writer.h"
#include "receiver.h"
#include "sender.h"

int main (int argc, char** argv)
{
    char* myPort = argv[1];
    char* remoteMachineName = argv[2];
    char* remotePort = argv[3];

    List* list = List_create();

    // Initializes modules
    Reader_init(list);
    Sender_init(list, remoteMachineName, remotePort);
    Receiver_init(list, myPort);
    Writer_init(list);

    // Shutdown my modules     
    Reader_shutdown();
    Sender_shutdown();
    Receiver_shutdown();
    Writer_shutdown();

    List_free(list, free);

    return 0;
}