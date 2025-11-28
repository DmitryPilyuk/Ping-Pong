#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define BUFF_SIZE 1024
#define PATH_SIZE 13

enum state { READY, SLEEP };

int sockfd;
char sock_path[PATH_SIZE];

void handle_signal() {
  printf("\nExiting...\n");
  close(sockfd);
  unlink(sock_path);
  exit(0);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s [0 or 1]\n", argv[0]);
    exit(1);
  }

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  int proc_idx = atoi(argv[1]);
  enum state st = proc_idx == 1 ? READY : SLEEP;

  char buffer[BUFF_SIZE];

  char to_sock_path[PATH_SIZE];

  sprintf(sock_path, "/tmp/socket%c", proc_idx == 0 ? '0' : '1');
  sprintf(to_sock_path, "/tmp/socket%c", (1 - proc_idx == 0) ? '0' : '1');

  sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);

  if (sockfd < 0) {
    perror("Failed to create socket");
    exit(1);
  }

  struct sockaddr_un addr = {0};
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);

  struct sockaddr_un to_addr = {0};
  socklen_t to_addr_len = sizeof(to_addr);
  to_addr.sun_family = AF_UNIX;
  strncpy(to_addr.sun_path, to_sock_path, sizeof(to_addr.sun_path) - 1);

  if (bind(sockfd, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("Failed to bind socket");
    close(sockfd);
    exit(1);
  }

  const char *cmd = "wake";
  ssize_t cmd_len = strlen(cmd);
  ssize_t n;
  while (1) {
    if (st == SLEEP) {
      memset(buffer, 0, BUFF_SIZE);

      n = recvfrom(sockfd, buffer, BUFF_SIZE, 0, (struct sockaddr *)&to_addr,
                   &to_addr_len);

      if (n < 0) {
        perror("Ошибка при получении данных");
        continue;
      }
      if (n == cmd_len && !strncmp(buffer, cmd, n)) {
        st = READY;
        printf("process %d changed state to READY\n", proc_idx);
      }
    } else {

      sleep(1);

      if (sendto(sockfd, cmd, cmd_len, 0, (const struct sockaddr *)&to_addr,
                 to_addr_len) < 0) {
        perror("Failed to send msg");
      } else {
        printf("process %d changed state to SLEEP\n", proc_idx);
        st = SLEEP;
      }
    }
  }
}