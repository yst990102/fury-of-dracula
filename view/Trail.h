#ifndef Trail_H
#define Trail_H

#include "Places.h"
typedef struct trail_container *Trail;
typedef struct trail_node *TrailNode;

struct trail_node {
    PlaceId item;
    TrailNode prev;
    TrailNode next;
};

struct trail_container {
    TrailNode head;
    TrailNode tail;
    int size;
};

// Creates a new trail
Trail Trail_New(void);

// Creates a new trail_node
TrailNode TrailNode_New(int item);

// Adds a new item to the trail
void Trail_push(Trail list, int item);

// Deltes the trail
void Trail_destroy(Trail list);

#endif