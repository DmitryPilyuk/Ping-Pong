#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 4
#define PATH_SIZE 13
#define N_ITERS 10

enum state { READY, SLEEP };

void act(int id);

int main(void) {
  pid_t pid = fork();
  if (pid == 0) {
    act(0);
  } else {
    act(1);
    wait(NULL);
  }
  return 0;
}

void act(int id) {
  enum state st = id == 0 ? SLEEP : READY;

  char buffer[BUFFER_SIZE];

  char sock_path[PATH_SIZE];
  char to_sock_path[PATH_SIZE];

  sprintf(sock_path, "/tmp/socket%c", id == 0 ? '0' : '1');
  sprintf(to_sock_path, "/tmp/socket%c", id == 0 ? '1' : '0');

  int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);

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
  for (int i = 0; i < N_ITERS; i++) {
    if (st == SLEEP) {

      while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                     (struct sockaddr *)&to_addr, &to_addr_len);

        if (n < 0) {
          perror("Ошибка при получении данных");
          continue;
        }
        if (n == cmd_len && !strncmp(buffer, cmd, n)) {
          st = READY;
          printf("process %d changed state to READY\n", id);
          break;
        }
      }
    } else {

      sleep(1);

      if (sendto(sockfd, cmd, cmd_len, 0, (const struct sockaddr *)&to_addr,
                 to_addr_len) < 0) {
        perror("Failed to send msg");
        close(sockfd);
        unlink(sock_path);
        exit(1);
      } else {
        printf("process %d changed state to SLEEP\n", id);
        st = SLEEP;
      }
    }
  }
  unlink(sock_path);
}