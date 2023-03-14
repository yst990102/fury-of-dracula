#include <assert.h>
#include <stdlib.h>

#include "Trail.h"

// Creates a new trail
Trail Trail_New(void) {
    Trail new = malloc(sizeof(*new));
    new->head = NULL;
    new->tail = NULL;
    new->size = 0;

    return new;
}

// Creates a new trail_node
TrailNode TrailNode_New(int item) {
    TrailNode new = malloc(sizeof(*new));
    new->item = item;
    new->next = NULL;
    new->prev = NULL;

    return new;
}

// Adds a new item to the trail
void Trail_push(Trail list, int item) {
    //TrailNode head = list->head;
    //TrailNode tail = list->tail;

    TrailNode add_node = TrailNode_New(item);
	// empty list, add as new node
    if (list->head == NULL) {
		list->head = list->tail = add_node;

		list->size++;
		return;
	} else {
		// only one elements in the list
		/*if (list->size == 1) {
			head->next = add_node;
			add_node->prev = head;

		} */
		list->tail->next = add_node;
		add_node->prev = list->tail;
		list->tail = add_node;
	}
	list->size++;
	
	


    /*if (!*tail)
        (*tail)->next = add_node;
    add_node->prev = *tail;
    *tail = add_node;*/

   
}

// Deltes the trail
void Trail_destroy(Trail list) {
    for (TrailNode node = list->head; node;) {
        TrailNode node_next = node->next;
        free(node);
        node = node_next;
    }
    free(list);
}