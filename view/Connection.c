#include "Connection.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "Define.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"
#include "Places.h"
#include "Queue.h"
#include "Trail.h"
#include "structure.h"

// Takes in 'non-exact' moves and returns exactly where Dracula is
// if not resolved to an unknown location
PlaceId get_Dracula_Location(TrailNode position) {
    if (position == NULL) {
        return NOWHERE;
    }

    PlaceId pos = position->item;
    if (pos == TELEPORT) {
        return CASTLE_DRACULA;
    } else if (pos == CITY_UNKNOWN || pos == SEA_UNKNOWN || pos == UNKNOWN_PLACE) {
        return pos;
    } else if (placeIsReal(pos)) {
        return pos;
    }

    while (!placeIsReal(pos) && pos != SEA_UNKNOWN && pos != CITY_UNKNOWN && pos != UNKNOWN_PLACE) {
        // FIne the location of DOUBLE_BACK_N
        if (DOUBLE_BACK_1 <= pos && pos <= DOUBLE_BACK_5) {
            int distance = pos - DOUBLE_BACK_1;
            for (int i = 0; i <= distance; i++) {
                position = position->prev;
            }

        } else if (pos == HIDE) {
            position = position->prev;
        }

        pos = position->item;
    }

    return pos;
}

// Finds all locations accessible by road
Queue RoadConnections(GameView gv, PlaceId position, Player p, Map m) {
    Queue q = newQueue();
    for (ConnList tmp = m->connections[position]; tmp != NULL; tmp = tmp->next) {
        if (tmp->type != ROAD) continue;  //????
        if (p == PLAYER_DRACULA) {
            // Dracula can't visit the hospital..
            if (tmp->p == HOSPITAL_PLACE) continue;
        }
		/* if (pred[tmp->p] != -1) continue; */
		//pred[tmp->p] = position;
        enterQueue(q, tmp->p);
    }

    if (!containQueue(q, position) /* && pred[position] == -1 */) {
		//pred[position] = position;
        enterQueue(q, position);
    }
    return q;
}

Queue RailConnections(GameView gv, PlaceId position, Player p, Map m, Round round) {
    // Dracula can't go by rail
    assert(p != PLAYER_DRACULA);

    int sum = round + p;

    Queue q = newQueue();
    for (ConnList tmp = m->connections[position]; tmp != NULL; tmp = tmp->next) {
		if (tmp->type != RAIL) continue;
        switch (sum % 4) {
            case 0:
                return q;
            case 1:
				enterQueue(q, tmp->p);
                break;
            case 2:
                QueueMerge(q, connections_rail_bfs(tmp->p, m, 0));
				break;
            case 3:
                QueueMerge(q, connections_rail_bfs(tmp->p, m, 1));
				break;
        }
    }
	if (!containQueue(q, position) /* && pred[position] == -1 */) {
	/* 	pred[position] = position; */
        enterQueue(q, position);
    }
    return q;
	
}


// Finds all locations accessible by boat
Queue BoatConnections(GameView gv, PlaceId position, Player p, Map m) {
    Queue q = newQueue();
    for (ConnList tmp = m->connections[position]; tmp != NULL; tmp = tmp->next) {
        if (tmp->type != BOAT) continue;  //????
		//if (pred[tmp->p] != -1) continue;
		//pred[tmp->p] = position;
        enterQueue(q, tmp->p);
    }

    if (!containQueue(q, position) /* && pred[position] == -1 */) {
		//pred[position] = position;
        enterQueue(q, position);
    }

    return q;
}

/* Check if an ARRAY of locations is within a players trail*/
bool locations_in_trail(GameView gv, Player player, PlaceId *loc, int nLoc) {
    int *p = malloc(sizeof(int *));
    *p = TRAIL_SIZE;
    bool canFree = false;
    PlaceId *trail = GvGetMoveHistory(gv, player, p, &canFree);

    for (int i = 0; i < nLoc; i++) {
        // The oldest move in the trail can be discarded
        for (int j = 0; j < TRAIL_SIZE - 1; j++)
            if (loc[i] == trail[j]) return true;
    }
    return false;
}

/* Checks if A SINGLE location is in a Dracula's trail */
bool location_in_trail(GameView gv, Player player, PlaceId loc) {
    int *p = malloc(sizeof(int *));
    *p = TRAIL_SIZE;
    bool canFree = false;
    PlaceId *trail = GvGetMoveHistory(gv, player, p, &canFree);

    // The oldest move in the trail can be discarded
    for (int i = 0; i < TRAIL_SIZE - 1; i++)
        if (trail[i] == loc) return true;

    return false;
}

// Helper Function to find possible rail paths using BFS
Queue connections_rail_bfs(PlaceId loc, Map m, int depth) {
    // Keep track of what's been visited
    bool hasBeenVisited[NUM_REAL_PLACES] = {false};
    hasBeenVisited[loc] = true;

    Queue q = newQueue();
    int currDepth = 0;      // Keeps track of how many stations we have hopped
    int countDown = 1;      // Counts down the number of elements until we increase the depth
    int countDownNext = 0;  // Counts down number of elements (doesn't include curr)

	enterQueue(q,loc);

    while (queueLength(q) != 0) {
        int value = leaveQueue(q);
        int nChildren = connections_bfs_process(q, value, hasBeenVisited, m);
        countDownNext += nChildren;

        // When there are no more children to check
        if (--countDown == 0) {
            // And we've reached the depth we want
            if (++currDepth > depth) break;
            countDown = countDownNext;
            countDownNext = 0;
        }
    }
    dropQueue(q);

    // Queue to return
    q = newQueue();
    for (int i = 0; i < NUM_REAL_PLACES; i++) {
		if (hasBeenVisited[i]) enterQueue(q,i);
	}
        /*if (hasBeenVisited[i]) {
			if (pred[i] == -1 ) {
				pred[i] = loc;
				enterQueue(q, i);
			} else  {
				// i has been visited 
				// check if loc is the shorter path to src
				int pathLength1 = 0;
				for (PlaceId v = dest; v != src; v = pred[v]) (*pathLength)++;
				PlaceId v = pred[i];
				while (v != LISBON) {
					pathLength1++;
					v = pred[v];
				}
				int pathLength2 = 0;
				PlaceId w = loc;
				while (w != LISBON) {
					pathLength2++;
					w = pred[w];
				}
				if (pathLength2 <= pathLength1) {
					pred[i] = loc;
					enterQueue(q,i);
				}
			}
		}*/
    

    return q;
}

// Helper function for BFS to enqueue items
// Returns the number of nodes added to the queue
int connections_bfs_process(Queue q, int item, bool *hasBeenVisited, Map m) {
    int counter = 0;

    for (ConnList tmp = m->connections[item]; tmp; tmp = tmp->next) {
        if (tmp->type == RAIL && !hasBeenVisited[(int)tmp->p]) {
            enterQueue(q, (int)tmp->p);
			//pred[tmp->p] = item;

            // Need to update that we have visited it
            hasBeenVisited[(int)tmp->p] = true;
            counter++;
        }
    }
    return counter;
}

/* Consider extra moves:
 * Hunter: Rest
 * Dracula: HIDE && DOUBLE_BACK_N */
Queue connections_get_extras(GameView gv, PlaceId location, Player player) {
    Queue q = newQueue();

    // Hunter: REST
    if (player != PLAYER_DRACULA) {
        enterQueue(q, location);
        return q;
    }

    // Dracula: HIDE
    PlaceId lastLoc = get_Dracula_Location(gv->players[PLAYER_DRACULA].moves->tail);
    if (gv->Dracula_Move.hide == 0 && placeIdToType(lastLoc) != SEA) {
		
        enterQueue(q, gv->players[PLAYER_DRACULA].moves->tail->item);
    }

    // Dracula: DOUBLE_BACK_N
    Map m = MapNew();

    if (gv->Dracula_Move.doubleBack == 0) {
        int size = min(gv->players[PLAYER_DRACULA].moves->size, TRAIL_SIZE);
        PlaceId realpos = get_Dracula_Location(gv->players[PLAYER_DRACULA].moves->tail);

		TrailNode destpos = gv->players[PLAYER_DRACULA].moves->tail;
		int to_consider = size + (int )DOUBLE_BACK_1;
        for (PlaceId i = DOUBLE_BACK_1; i < to_consider; i++) {
            int count = i - DOUBLE_BACK_1;
            
            for (int j = 0; j < count; j++) {
                destpos = destpos->prev;
            }
            PlaceId dest = destpos->item;
            if (dest == HIDE) {
                destpos = destpos->prev;
                dest = destpos->item;
            }

            if (size != 0 && connections_is_location_connected(m, realpos, dest, true, false, true)) {
                enterQueue(q, dest);
                size--;
            }
        }
    }

    MapFree(m);
    return q;
}

bool connections_is_location_connected(Map m, PlaceId locA, PlaceId locB, bool road, bool rail, bool sea) {
    assert(placeIsReal(locA));
    assert(placeIsReal(locB));
    if (locA == locB) {
        return true;
    }

    bool createMap = false;
    if (!m) {
        createMap = true;
        m = MapNew();
    }

    bool status = false;
    for (ConnList tmp = m->connections[locA]; tmp; tmp = tmp->next) {
        if (tmp->p == locB) {
            if ((tmp->type == ROAD && road) || (tmp->type == RAIL && rail) || (tmp->type == BOAT && sea)) {
                status = true;
                break;
            }
        }
    }

    if (createMap) MapFree(m);
    return status;
}

TrailNode GetRealPlace(Trail moves, TrailNode p) {
    if (p->item == HIDE) {
        return GetRealPlace(moves, p->prev);
    }

    if (p->item <= DOUBLE_BACK_5 && p->item >= DOUBLE_BACK_1) {
        int count = p->item - DOUBLE_BACK_1;
        while (count != 0) {
            p = p->prev;
            count--;
        }
        return GetRealPlace(moves, p);
    }

    if (placeIsReal(p->item)) {
        return p;
    }

    TrailNode unknown = TrailNode_New(UNKNOWN_PLACE);
    return unknown;
}

void MoveNonPlaceFromPlaceIdArray(PlaceId *array, int *length) {
    int i, j;
    for (i = 0; array[i]; i++) {
        if (placeIsReal(array[i])) {
            continue;
        } else {
            for (j = i; j < array[j + 1]; j++) {
                array[j] = array[j + 1];
            }
            array[j] = '\0';
            (*length)--;
            i--;
        }
    }
}