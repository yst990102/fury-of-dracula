// Queue.c ... list implementation of a queue

#include "Queue.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct QueueNode *Link;

struct QueueNode {
    Item item;
    Link next;
};

struct QueueRep {
    Link head;
    Link tail;
    int size;
};

// private function for creating list nodes
static Link newNode(Item item) {
    Link n = malloc(sizeof(struct QueueNode));
    assert(n != NULL);
    n->item = item;
    n->next = NULL;
    return n;
}

// create an initially empty Queue
Queue newQueue(void) {
    Queue q = malloc(sizeof(struct QueueRep));
    assert(q != NULL);
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

// free all memory used by the Queue
void dropQueue(Queue q) {
    Link curr;
    Link next;
    assert(q != NULL);
    curr = q->head;
    while (curr != NULL) {
        next = curr->next;
		free(curr);
        curr = next;
    }
    free(q);
}

// add new Item to the head of the Queue
void enterQueue(Queue q, Item it) {
    assert(q != NULL);
    Link n = newNode(it);
    if (q->head == NULL) {
        q->head = n;
        q->tail = n;
    } else {
        q->tail->next = n;
		q->tail = n;
    }
    q->size++;
}

// remove Item from tail of Queue; return it
Item leaveQueue(Queue q) {
    assert(q != NULL);
    assert(q->size > 0);
	Item it = q->head->item;
	Link delNode = q->head;
	q->head = q->head->next;
	free(delNode);
	q->size--;
	return it;




   /* Item it = q->tail->item;
    Link delNode = q->tail;
    Link newtail = q->head;
    while (newtail->next != delNode && newtail->next != NULL) {
        newtail = newtail->next;
    }
    free(delNode);
    q->tail = newtail;
    q->size--;
    return it;*/
}

// return count of Items in Queue
int queueLength(Queue q) {
    assert(q != NULL);
    return q->size;
}

// display Queue as list of 2-digit numbers
void showQueue(Queue q) {
    printf("H");
    Link curr;
    curr = q->head;
    while (curr != NULL) {
        printf(" %02d", curr->item);
        curr = curr->next;
    }
    printf(" T\n");
}

// Merge Queue p to Queue q with no repetition
void QueueMerge(Queue q, Queue p) {
    for (Link i = p->head; i != NULL; i = i->next) {
        if (!containQueue(q, i->item)) {
            enterQueue(q, i->item);
        }
    }
    dropQueue(p);
}

// Check if value is in Queue q
bool containQueue(Queue q, Item value) {
    for (Link i = q->head; i != NULL; i = i->next) {
        if (i->item == value) {
            return true;
        }
    }
    return false;
}

// Check if Queue is empty
bool QueueIsEmpty(Queue Q) { return (Q->head == NULL); }

// sort Queue q in alphbetical order
void sortQueue(Queue q) {
    Link cur;
    Link end;
    Item temp = 0;
    cur = q->head;
    end = NULL;
    while (cur != end) {
        while (cur->next != end) {
            if (cur->item > cur->next->item) {
                temp = cur->item;
                cur->item = cur->next->item;
                cur->next->item = temp;
            }
            cur = cur->next;
        }
        end = cur;
        cur = q->head;
    }
}

// drop specific Link p from Queue q
void drop_p_from_Queue(Queue q, Link p) {
    if (q == NULL || q->head == NULL || q->tail == NULL) {
        return;
    }

    if (p->item == q->head->item) {
        q->head = q->head->next;
    } else if (p->item == q->tail->item) {
        Link cur = q->head;
        while (cur->next->item != p->item) {
            cur = cur->next;
        }
        cur->next = p->next;
        q->tail = cur;
    } else {
        assert(q->size >= 3);
        Link pre = q->head;
        Link cur = q->head->next;
        while (cur->item != p->item) {
            cur = cur->next;
            pre = pre->next;
        }
        pre->next = p->next;
    }

    free(p);
    q->size--;
}

// q1 = q1 - q2, only drop repeat-Links in q1
void QueueSubtraction(Queue q1, Queue q2) {
    Link p1 = q1->head;
    Link p2 = q2->head;

    bool isfound = false;
    while (p1 != NULL) {
        while (p2 != NULL) {
            if (p2->item == p1->item) {
                isfound = true;
                Link freelink = p1;
                p1 = p1->next;
                drop_p_from_Queue(q1, freelink);
                break;
            }
            p2 = p2->next;
        }

        if (isfound == false) {
            p1 = p1->next;
        }
        p2 = q2->head;
        isfound = false;
    }
}