#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <assert.h>
#include "uthread.h"
#include "queue.h"
#include "disk.h"

int sum = 0;

/**
 * Description of 1 pendind read.  
 * Each pending read has one of these stored on the prq queue.
 */
struct PendingRead {
  // TODO
  char* buf;
  int nbytes;
  int blockno;
  void (*handler) (char*, int, int);
  
};

/**
 * Queue of pending reads.
 */
queue_t prq;

void interruptServiceRoutine () {
  // called when disk fires an interrupt TODO
  
}

void asyncRead (char* buf, int nbytes, int blockno, void (*handler) (char*, int, int)) {
  // call disk_scheduleRead to schedule a read TODO
  struct PendingRead* PR = malloc(sizeof *PR);
  PR->buf = buf;
  PR->nbytes = nbytes;
  PR->blockno = blockno;
  PR->handler = handler;
  queue_enqueue (&prq, PR);
  disk_scheduleRead (buf, nbytes, blockno);
  disk_waitForReads ();
  
  
}

/**
 * This is the read completion routine.  This is where you 
 * would place the code that processed the data you got back
 * from the disk.  In this case, we'll just verify that the
 * data is there.
 */
void handleRead (char* buf, int nbytes, int blockno) {
  assert (*((int*)buf) == blockno);
  sum += *(((int*) buf) + 1);
}

/**
 * Read numBlocks blocks from disk sequentially starting at block 0.
 */
void run (int numBlocks) {
  for (int blockno = 0; blockno < numBlocks; blockno++) {
	  struct PendingRead* item = queue_dequeue (&prq);
	  char* buf = item->buf;
	  int nbytes = item->nbytes;
      int blockno = item->blockno;
      void (*handler) (char*, int, int) = item->handler;
	//TODO, ask TA  uthread_create (asyncRead(buf, nbytes, item->blockno, void (*handler) (char*, int, int) ), &blockno);
    // call asyncRead to schedule read TODO
  }
  disk_waitForReads();
}

int main (int argc, char** argv) {
  static const char* usage = "usage: aRead numBlocks";
  int numBlocks = 0;
  queue_init (&prq);
  
  if (argc == 2)
    numBlocks = strtol (argv [1], NULL, 10);
  if (argc != 2 || (numBlocks == 0 && errno == EINVAL)) {
    printf ("%s\n", usage);
    return EXIT_FAILURE;
  }
  
  uthread_init (1);
  disk_start   (interruptServiceRoutine);
  
  run (numBlocks);
  
  printf ("%d\n", sum);
}