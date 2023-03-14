////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// HunterView.c: the HunterView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10   v3.0    Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include "HunterView.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "Game.h"
#include "GameView.h"
#include "Map.h"
#include "Places.h"
// add your own #includes here
#include "Connection.h"
#include "Define.h"
#include "Queue.h"

// ADD YOUR OWN STRUCTS HERE
#include "structure.h"

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

HunterView HvNew(char *pastPlays, Message messages[]) {
    HunterView new = malloc(sizeof(*new));
    if (new == NULL) {
        fprintf(stderr, "Couldn't allocate HunterView!\n");
        exit(EXIT_FAILURE);
    }
    new->gv = GvNew(pastPlays, messages);

    return new;
}

void HvFree(HunterView hv) {
    GvFree(hv->gv);
    free(hv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round HvGetRound(HunterView hv) { return GvGetRound(hv->gv); }

Player HvGetPlayer(HunterView hv) { return GvGetPlayer(hv->gv); }

int HvGetScore(HunterView hv) { return GvGetScore(hv->gv); }

int HvGetHealth(HunterView hv, Player player) { return GvGetHealth(hv->gv, player); }

PlaceId HvGetPlayerLocation(HunterView hv, Player player) { return GvGetPlayerLocation(hv->gv, player); }

PlaceId HvGetVampireLocation(HunterView hv) { return GvGetVampireLocation(hv->gv); }

////////////////////////////////////////////////////////////////////////
// Utility Functions

PlaceId HvGetLastKnownDraculaLocation(HunterView hv, Round *round) {
    TrailNode cur_pos = hv->gv->players[PLAYER_DRACULA].moves->tail;
    Round cur_round = hv->gv->players[PLAYER_DRACULA].moves->size;

    int count = TRAIL_SIZE;
    while (count > 0) {
        PlaceId Dracula_cur_pos = get_Dracula_Location(cur_pos);
        if (Dracula_cur_pos == NOWHERE) {
            break;
        }

        for (Player i = PLAYER_LORD_GODALMING; i < PLAYER_MINA_HARKER; i++) {
            if (place_in_moves(hv->gv, i, Dracula_cur_pos)) {
                if (hv->gv->currTurn % NUM_PLAYERS == PLAYER_DRACULA) {
                    *round = cur_round;
                } else {
                    *round = cur_round - 1;
                }

                return Dracula_cur_pos;
            }
        }

        cur_pos = cur_pos->prev;
        cur_round--;
    }

    *round = 0;
    return NOWHERE;
}

// iterative BFS algorithm to find path src...dest
PlaceId *HvGetShortestPathTo(HunterView hv, Player hunter, PlaceId dest, int *pathLength) {
    //Map m = MapNew();
	int round = hv->gv->currRound;
	int rCounter = 0;
	Queue locs = newQueue();
	PlaceId src = GvGetPlayerLocation(hv->gv, hunter);
	PlaceId pred[NUM_REAL_PLACES];
	for (int i = 0; i < NUM_REAL_PLACES; i++) pred[i] = UNKNOWN_PLACE;
	pred[src] = src;
	enterQueue(locs, src);
	while(!QueueIsEmpty(locs)) {
		
		PlaceId x = leaveQueue(locs);
		printf("		@x %-15s\n", placeIdToName(x));
		if (x == dest) {
			break;
		}
		int nPossibleLocs = -1;
		PlaceId *possibleLocs = GvGetReachable(hv->gv, hunter, round, x, &nPossibleLocs);
		
		for (int j = 0; (j < nPossibleLocs ); j++) printf("	@locs %-15s\n", placeIdToName(possibleLocs[j]));
		for (int i = 0; i < nPossibleLocs;i++) {
			if (pred[possibleLocs[i]] == UNKNOWN_PLACE) {
				pred[possibleLocs[i]] = x; 
				enterQueue(locs, possibleLocs[i]);
			} else {
				if (possibleLocs[i] == x) {
					continue;
				}
				int pathLength1 = 0;
				PlaceId v = pred[possibleLocs[i]];
				while (v != src) {
					pathLength1++;
					v = pred[v];
				}
				int pathLength2 = 0;
				PlaceId w = x;
				while (w != src) {
					pathLength2++;
					w = pred[w];
				}
				if (pathLength2 <= pathLength1) {
					pred[possibleLocs[i]] = x;
					enterQueue(locs,possibleLocs[i]);
				}
			}
		}

		if (rCounter) {
			rCounter--;
		}
		if(!rCounter) {
			round++;
		}
		if (!rCounter) {
			rCounter = queueLength(locs);
		}

	}
	*pathLength = 0;
	PlaceId v = dest;
	while (v != src) {
		(*pathLength)++;
		v = pred[v];
	}
	PlaceId *path = malloc(sizeof(PlaceId) * (*pathLength));
	int i = (*pathLength) - 1;
	PlaceId p = dest;
	while (i >= 0) {
		path[i] = p;
		p = pred[p];
		i--;
	}
	return path;
}
   

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *HvWhereCanIGo(HunterView hv, int *numReturnedLocs) {
    Player player = hv->gv->currRound % NUM_PLAYERS;
    PlaceId from = hv->gv->players[player].moves->tail->item;
    return GvGetReachable(hv->gv, player, hv->gv->currRound, from, numReturnedLocs);
}

PlaceId *HvWhereCanIGoByType(HunterView hv, bool road, bool rail, bool boat, int *numReturnedLocs) {
    Player player = hv->gv->currRound % NUM_PLAYERS;
    PlaceId from = hv->gv->players[player].moves->tail->item;
    return GvGetReachableByType(hv->gv, player, hv->gv->currRound, from, road, rail, boat, numReturnedLocs);
}

PlaceId *HvWhereCanTheyGo(HunterView hv, Player player, int *numReturnedLocs) {
    int round = 0;

    Player currPlayer_n = hv->gv->currTurn % NUM_PLAYERS;
    if (currPlayer_n >= player) {
        // this player has already moved for this round
        // need to expect next round moves for him
        round = hv->gv->currRound + 1;
    } else {
        // this player haven't moved for this round
        round = hv->gv->currRound;
    }

    PlaceId from = hv->gv->players[player].moves->tail->item;
    return GvGetReachable(hv->gv, player, round, from, numReturnedLocs);
}

PlaceId *HvWhereCanTheyGoByType(HunterView hv, Player player, bool road, bool rail, bool boat, int *numReturnedLocs) {
    int round = 0;

    Player currPlayer_n = hv->gv->currTurn % NUM_PLAYERS;
    if (currPlayer_n >= player) {
        // this player has already moved for this round
        // need to expect next round moves for him
        round = hv->gv->currRound + 1;
    } else {
        // this player haven't moved for this round
        round = hv->gv->currRound;
    }

    PlaceId from = hv->gv->players[player].moves->tail->item;

    PlaceId *AvailablePlaces = GvGetReachableByType(hv->gv, player, round, from, road, rail, boat, numReturnedLocs);
    MoveNonPlaceFromPlaceIdArray(AvailablePlaces, numReturnedLocs);

    return AvailablePlaces;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

bool place_in_moves(GameView gv, Player player, PlaceId place) {
    int length = gv->players[player].moves->size;
    for (TrailNode i = gv->players[player].moves->head; length > 0; i = i->next) {
        if (i->item == place) {
            return true;
        }
        length--;
    }
    return false;
}

