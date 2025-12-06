#ifndef SYNC_H
#define SYNC_H

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

enum state { READY, SLEEP };

typedef struct {
  int sockfd;

  struct sockaddr_un self;
  struct sockaddr_un peer;
  socklen_t peer_len;

} sync_t;

int sync_init(sync_t *s, pid_t id);
int sync_wait(sync_t *s);
int sync_signal(sync_t *s);
void sync_destroy(sync_t *s);

#endif // SYNC_H
