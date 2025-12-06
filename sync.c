#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "sync.h"

#define HELLO_MSG "hello"
#define WAKE_MSG "wake"
#define BUF_SIZE 16

static void safe_unlink(const char *path) {
  if (unlink(path) < 0 && errno != ENOENT)
    perror("unlink");
}

static int send_msg(sync_t *s, const char *msg) {
  return sendto(s->sockfd, msg, strlen(msg), 0, (struct sockaddr *)&s->peer,
                s->peer_len);
}

int sync_init(sync_t *s, pid_t id) {
  memset(s, 0, sizeof(*s));

  s->sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (s->sockfd < 0)
    return -1;

  /* self addr */
  memset(&s->self, 0, sizeof(s->self));
  s->self.sun_family = AF_UNIX;
  snprintf(s->self.sun_path, sizeof(s->self.sun_path), "/tmp/pp_%d.sock",
           getpid());

  /* peer addr */
  memset(&s->peer, 0, sizeof(s->peer));
  s->peer.sun_family = AF_UNIX;
  snprintf(s->peer.sun_path, sizeof(s->peer.sun_path), "/tmp/pp_%d.sock", id);
  s->peer_len = sizeof(s->peer);

  safe_unlink(s->self.sun_path);

  if (bind(s->sockfd, (struct sockaddr *)&s->self, sizeof(s->self)) < 0)
    return -1;

  /* ---- INIT HANDSHAKE ---- */
  char buffer[BUF_SIZE];

  int got_hello = 0;

  while (!got_hello) {

    usleep(10000);

    send_msg(s, HELLO_MSG);

    struct sockaddr_un from = {0};
    socklen_t flen = sizeof(from);
    ssize_t n = recvfrom(s->sockfd, buffer, BUF_SIZE, MSG_DONTWAIT,
                         (struct sockaddr *)&from, &flen);

    if (n > 0 && n == strlen(HELLO_MSG) && strncmp(buffer, HELLO_MSG, n) == 0) {

      got_hello = 1;
    }
  }

  return 0;
}

int sync_wait(sync_t *s) {
  char buffer[BUF_SIZE];

  for (;;) {
    struct sockaddr_un from;
    socklen_t flen = sizeof(from);

    ssize_t n = recvfrom(s->sockfd, buffer, BUF_SIZE, 0,
                         (struct sockaddr *)&from, &flen);

    if (n < 0) {
      if (errno == EINTR)
        continue;
      return -1;
    }

    if (n == strlen(WAKE_MSG) && strncmp(buffer, WAKE_MSG, n) == 0) {
      return 0;
    }
  }
}

int sync_signal(sync_t *s) { return send_msg(s, WAKE_MSG); }

void sync_destroy(sync_t *s) {
  close(s->sockfd);
  safe_unlink(s->self.sun_path);
}