#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "sync.h"

#define N_ITERS 10

void act(pid_t pid);

int main(void) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("Failed to create process\n");
  }
  if (pid == 0) {
    act(getppid());
  } else {
    act(pid);
    wait(NULL);
  }
  return 0;
}

void act(pid_t pid) {
  pid_t self_pid = getpid();

  enum state s = pid < self_pid ? READY : SLEEP;

  sync_t sync;
  sync_init(&sync, pid);

  for (int i = 0; i < N_ITERS; i++) {
    if (s == SLEEP) {
      sync_wait(&sync);
      printf("Process %d: Changed state to READY\n", self_pid);
      fflush(stdout);
    } else {
      sleep(1);
      sync_signal(&sync);
      printf("Process %d: Changed state to SLEEP\n", self_pid);
      fflush(stdout);
    }
  }
  sync_destroy(&sync);
}