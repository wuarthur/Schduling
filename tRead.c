#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <assert.h>
#include "queue.h"
#include "disk.h"
#include "uthread.h"

int sum = 0;

queue_t prq;

void interruptServiceRoutine () {
  uthread_unblock(queue_dequeue (&prq));
}

void blockUntilComplete() {
  uthread_block();
  queue_enqueue(&prq, uthread_self());


}

void handleRead (char* buf, int nbytes, int blockno) {
  assert (*((int*)buf) == blockno);
  sum += *(((int*) buf) + 1);
}

/**
 * Struct provided as argument to readAndHandleBlock
 */
struct readAndHandleBlockArgs {
  char* buf;
  int   nbytes;
  int   blockno;
};

void* readAndHandleBlock (void* args_voidstar) {
  // TODO
  // Synchronously:
  //   (1) call disk_scheduleRead to request the block
  //   (2) wait read for it to complete
  //   (3) call handleRead
  struct readAndHandleBlockArgs* args = args_voidstar; 
  queue_enqueue(&prq, uthread_self());
  disk_scheduleRead (args->buf, args->nbytes, args->blockno);
  blockUntilComplete();
  handleRead (args->buf, args->nbytes, args->blockno);
  return NULL;
}

void run (int numBlocks) {
  uthread_t thread [numBlocks];
  char buf[numBlocks][4096];
  struct readAndHandleBlockArgs args[numBlocks];
  for (int blockno = 0; blockno < numBlocks; blockno++) {
    // TODO
    // call readAndHandleBlock in a way that allows this
    // operation to be synchronous without stalling the CPU
   
    args[blockno].buf = buf[blockno];
    args[blockno].nbytes = sizeof buf;
    args[blockno].blockno = blockno;

    thread[blockno] = uthread_create(readAndHandleBlock, &args[blockno]);

  }
  for (int i=0; i<numBlocks; i++)
    uthread_join (thread [i], 0);
}

int main (int argc, char** argv) {
  static const char* usage = "usage: tRead numBlocks";
  int numBlocks = 0;
  
  if (argc == 2)
    numBlocks = strtol (argv [1], NULL, 10);
  if (argc != 2 || (numBlocks == 0 && errno == EINVAL)) {
    printf ("%s\n", usage);
    return EXIT_FAILURE;
  }
  
  uthread_init (1);
  disk_start   (interruptServiceRoutine);
  
  run (numBlocks);
  queue_init (&prq);
  
  printf ("%d\n", sum);
}
