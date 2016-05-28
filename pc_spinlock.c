#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "spinlock.h"

#define MAX_ITEMS 10

int items = 0;

const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

spinlock_t lock;

void produce() {
  // TODO ensure proper synchronization
  while(1) {
	while (items >= MAX_ITEMS) {
	  spinlock_lock(&lock);
	  producer_wait_count++;
	  spinlock_unlock(&lock);
  }
  spinlock_lock(&lock);
  if (items < MAX_ITEMS) {
	  items++;
	  histogram [items] += 1;
      spinlock_unlock(&lock);
	  break;
		}
//  assert (items < MAX_ITEMS);
	
	spinlock_unlock(&lock);
  }
}

void consume() {
  // TODO ensure proper synchronization
  
//  assert (items > 0);
  while(1) {
	while (items == 0) {
	  spinlock_lock(&lock);
	  consumer_wait_count++;
	  spinlock_unlock(&lock);
  }
  spinlock_lock(&lock);
  if (items > 0) {
	    items--;
		histogram [items] += 1;
		spinlock_unlock(&lock);
		break;
		}	
	
	spinlock_unlock(&lock);
  }
}

void* producer() {
  // TODO - You might have to change this procedure slightly
  for (int i=0; i < NUM_ITERATIONS; i++)
    produce();
     return NULL;
     }

void* consumer() {
  // TODO - You might have to change this procedure slightly
  for (int i=0; i< NUM_ITERATIONS; i++)
    consume();
  return NULL;
}

int main (int argc, char** argv) {
  // TODO create threads to run the producers and consumers
  uthread_init(4);
  uthread_t t[4];
  spinlock_create (&lock);
  
  for (int i = 0; i < 2; i++) {
	  t[i] = uthread_create(producer, 0);
  }
  
   for (int i = 2; i < 4; i++) {
	  t[i] = uthread_create(consumer, 0);
  }
  
  for (int i = 0; i < 4; i++) {
	  uthread_join(t[i], 0);
  }
  
  printf("Producer wait: %d\nConsumer wait: %d\n",
         producer_wait_count, consumer_wait_count);
  for(int i=0;i<MAX_ITEMS+1;i++)
    printf("items %d count %d\n", i, histogram[i]);
}
