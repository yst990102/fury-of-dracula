#ifndef Queue_H
#define Queue_H

#include <stdbool.h>

// Queue.h ... Queue ADT

// Queue items are positive integers
typedef int Item;
// Queues has a hidden representation
typedef struct QueueRep *Queue;

// Queue.h ... Queue ADT
typedef struct QueueNode *Link;

// create an initially empty Queue
Queue newQueue(void);

// free all memory used by the Queue
void dropQueue(Queue);

// add new Item to the tail of the Queue
void enterQueue(Queue, Item);

// remove Item from head of Queue; return it
Item leaveQueue(Queue);

// return count of Items in Queue
int queueLength(Queue);

// display Queue as list of 2-digit numbers
void showQueue(Queue);

// Merge Queue p to Queue q with no repetition
void QueueMerge(Queue q, Queue p);

// Check if value is in Queue q
bool containQueue(Queue q, Item value);

// check whether queue is empty
bool QueueIsEmpty(Queue Q);

// drop specific Link p from Queue q
void drop_p_from_Queue(Queue q, Link p);

// sort Queue q in alphbetical order
void sortQueue(Queue q);

// q1 = q1 - q2, only drop repeat-Links in q1
void QueueSubtraction(Queue q1, Queue q2);

#endif