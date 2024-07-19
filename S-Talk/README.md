###S-Talk

- Build a mini chat using UDP threads
- In the terminal, argument passed by: 
``` ./s-talk (local port) (remote ip address) (remote port)```
- The project support in LINUX:
    - Message send are add to a list (structure of the list contain in list.h)
    - Message then queue and send to the receiver port using threads