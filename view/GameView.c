#include "GameView.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "Game.h"
#include "Map.h"
#include "Places.h"

// add your own #includes here
#include "Connection.h"
#include "Queue.h"
#include "Trail.h"

// ADD YOUR OWN STRUCTS HERE
#include "structure.h"

// ADD YOUR OWN DEFINE HERE
#include "Define.h"

// Your own interface functions:
PlaceId gv_get_location(GameView gv, Player player);
static bool playerRested(GameView gv, Player p);
void gv_get_history(GameView gv, enum player player, PlaceId trail[TRAIL_SIZE]);

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

GameView GvNew(char *pastPlays, Message messages[]) {
    // DONE: GvNew
    GameView gv = malloc(sizeof(*gv));

    if (gv == NULL) {
        fprintf(stderr, "Couldn't allocate GameView!\n");
        exit(EXIT_FAILURE);
    }
	
	// initialise numeric fields 
    gv->currTurn = 0;
    gv->currRound = 0;
    gv->score = GAME_START_SCORE;
	// dracula can use doubleBack and hide at the beginning of the game
    gv->Dracula_Move.doubleBack = 0;
    gv->Dracula_Move.hide = 0;
	gv->Dracula_Move.vampFlyTime = 0; 
	// no vampire been placed at the start
    gv->encounters.vamp_location = NOWHERE;
	for (int i = 0; i < NUM_REAL_PLACES; i++) gv->encounters.trap[i] = 0;
	// initialise playerInfo
	// all players starts with UNKNOWN_PLACE
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (i == PLAYER_DRACULA) {
            gv->players[i].type = PLAYER_DRACULA;
            gv->players[i].health = GAME_START_BLOOD_POINTS;
            gv->players[i].moves = Trail_New();
        } else {
            gv->players[i].type = i;
            gv->players[i].health = GAME_START_HUNTER_LIFE_POINTS;
            gv->players[i].moves = Trail_New();
        }
    }

// parse pastPlay to load status of the game
    char *moveStr;
    if (pastPlays != NULL) moveStr = strtok(strdup(pastPlays), " ");

    while (moveStr != NULL) {
        assert(strlen(moveStr) == 7);

		// identify the current player
		Player currPlayer_n = gv->currTurn % NUM_PLAYERS;
		moveStr++;
		// identify the location
		char *locationStr = strdup(moveStr);
		locationStr[2] = '\0';
        /* assert(memcpy(locationStr, moveStr + 1, 2) != NULL);
		locationStr[2] = '\0'; */
		PlaceId location = placeAbbrevToId(locationStr);
        //assert(placeIsReal(location));

		moveStr += 2;
		// identify the events happened
		char *event = strdup(moveStr);
        /* assert(memcpy(event, moveStr + 3, 4) != NULL);
		event[4] = '\0'; */

		// grab current player's info
        //gv->currPlayer = &(gv->players[currPlayer_n]);
		// save Player's move into his move history
		Trail_push(gv->players[currPlayer_n].moves, location);


		
        

//////////////////////////////////////  record consequences of Dracula‘s actions   /////////////////////////////////////////
        if (currPlayer_n == PLAYER_DRACULA) {
			PlaceId DraExactLocation = GvGetPlayerLocation(gv, PLAYER_DRACULA);
			// printf("    Dracula  @ %-15s\n", placeIdToName(DraExactLocation));
			//printf("%d", DraExactLocation);
			//////////////////    location      ////////////////////////////
					/****how location updates Dracula_Move****/
			if (location == HIDE) {
				assert(gv->Dracula_Move.hide == 0);
                assert(placeIdToType(DraExactLocation) !=SEA);
				// set to 6 to count down
				// so that when HIDE move is available is known
				gv->Dracula_Move.hide = 5;
			} else if (location >= DOUBLE_BACK_1 && location<= DOUBLE_BACK_5) {
				gv->Dracula_Move.doubleBack = 5;
			} 
			
			if (location != HIDE && gv->Dracula_Move.hide) {
				// count down for HIDE 
				gv->Dracula_Move.hide--;
			}
			if ((location <= DOUBLE_BACK_1 || location >= DOUBLE_BACK_5) && gv->Dracula_Move.doubleBack) {
				// count down for DOUBLE_BACK
				gv->Dracula_Move.doubleBack--;
			}
			 if (location >= DOUBLE_BACK_1 && location <= DOUBLE_BACK_5) {
                assert((gv->players[PLAYER_DRACULA].moves->size - 1)> (location - DOUBLE_BACK_1)); //??? -2 means minus first nowhere and the last double back
            }

					/******* how location updates Dracula's health *******/
			// Find the Dracula's physical location
            if (placeIdToType(DraExactLocation) == SEA) {
				// Loses 2 points of health if at sea
                gv->players[PLAYER_DRACULA].health -= LIFE_LOSS_SEA;
            } else if (DraExactLocation == CASTLE_DRACULA) {
				// gains 10 points of health if in castle
                gv->players[PLAYER_DRACULA].health += LIFE_GAIN_CASTLE_DRACULA;
            }
					
			

				//////////////////////////       event       ////////////////////////////
					/***********   event part 1: encounter   *******************/
			if (event[0] == 'T') {
				if (placeIsReal(DraExactLocation)) assert(gv->encounters.trap[DraExactLocation] < 3);
				// place a trap in current
                gv->encounters.trap[DraExactLocation]++;
			} else if (event[1] == 'V') {
				// Check if Dracula has placed vampires before
                assert(gv->currRound % 13 == 0);
                if (placeIsReal(DraExactLocation)) assert(gv->encounters.trap[DraExactLocation] < 3);
                assert(gv->encounters.vamp_location == NOWHERE);
                assert(gv->Dracula_Move.vampFlyTime == 0);

				// when immature vampire being placed
				// record its location and start timing
                gv->encounters.vamp_location = DraExactLocation;
                gv->Dracula_Move.vampFlyTime = 6;
			} 
					/************   event part 2: action phase   **********/
			if (event[2] == 'M') {
				// when trap fails off the trail, it vanishes
                // assert(gv->encounters.trap[DraExactLocation] > 0);
				bool canFree = false;
				int numReturnedLocs = 0; 
				//PlaceId *t = GvGetLocationHistory(gv, PLAYER_DRACULA, &numReturnedLocs, &canFree);

				PlaceId *t = GvGetLastLocations(gv, PLAYER_DRACULA, TRAIL_SIZE + 1, &numReturnedLocs, &canFree);
				for (int i = 0; i < numReturnedLocs; i++) printf("    locs  @ %-15s\n", placeIdToName(t[i])); 
                gv->encounters.trap[t[0]]--;
            } else if (event[2] == 'V') {
				// when vampire matures deduct score
				// reset time for vamFlyTime
				// clear vampire's location
                gv->score -= SCORE_LOSS_VAMPIRE_MATURES;
                gv->Dracula_Move.vampFlyTime = 0;
                gv->encounters.vamp_location = NOWHERE;
            }

					//////////////////      game timer      /////////////////////////
			// game score decreases by 1 each time Dracula finishes a turn.
            gv->score -= SCORE_LOSS_DRACULA_TURN;
			gv->currTurn++;
            gv->currRound++;

////////////////////////////////////////////////   record consequences of hunter‘s actions  /////////////////////////////////////////////
        } else {
					/////////////////		event			///////////////////////////////
							/*********      health increment		***********/
		if (GvGetHealth(gv, currPlayer_n) > 0 && GvGetHealth(gv, currPlayer_n) < GAME_START_HUNTER_LIFE_POINTS && 
			playerRested(gv, currPlayer_n)
		) {
                gv->players[currPlayer_n].health += LIFE_GAIN_REST;
        } 
							/*********		health re-initialisation	*******/
			// reset the hunter's health if they died last round
			if (!gv->players[currPlayer_n].health) {
				gv->players[currPlayer_n].health = GAME_START_HUNTER_LIFE_POINTS;
			}
							/********	 	health decrement		*********/
			char *hunter_event = strdup(event);
            while (*hunter_event != '.' && *hunter_event != '\0') {
                switch (*hunter_event) {
					// the hunter encounters Dracula's trap
                    case 'T':
						gv->encounters.trap[location]--;
						gv->players[currPlayer_n].health -= LIFE_LOSS_TRAP_ENCOUNTER;
                        break;
					// the hunter kills the immature vampire
                    case 'V':
                        gv->Dracula_Move.vampFlyTime = 0;
                        gv->encounters.vamp_location = NOWHERE;
                        break;
					// the hunter encounter Dracula
                    case 'D':
                        gv->players[currPlayer_n].health -= LIFE_LOSS_DRACULA_ENCOUNTER;
                        gv->players[PLAYER_DRACULA].health -= LIFE_LOSS_HUNTER_ENCOUNTER;
                        break;
                }
                hunter_event++;
            }
			//free(hunter_event);
			if (gv->players[currPlayer_n].health <= 0) {
				gv->score -= SCORE_LOSS_HUNTER_HOSPITAL;
				gv->players[currPlayer_n].health = 0;
			}
			
					////////////////		game timer		///////////////////////////////
			gv->currTurn++;
		}
        moveStr = strtok(NULL, " ");
    }

    return gv;
}

///////////////			gvNew Helper function			//////////////////////////////////
////////////////////////////////////////////////////////////////////////
// Game State Information

void GvFree(GameView gv) {
    for (int i = 0; i < NUM_PLAYERS; i++) {
        Trail_destroy(gv->players[i].moves);
    }

    free(gv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round GvGetRound(GameView gv) { return gv->currRound; }

Player GvGetPlayer(GameView gv) { return gv->currTurn % NUM_PLAYERS; }

int GvGetScore(GameView gv) { return gv->score; }

int GvGetHealth(GameView gv, Player player) { return gv->players[player].health; }

PlaceId GvGetPlayerLocation(GameView gv, Player player) {  //???
    if (gv->players[player].moves->head == NULL) return NOWHERE;

    if (player == PLAYER_DRACULA) {
		return get_Dracula_Location(gv->players[player].moves->tail);
		
	} else {
		if (gv->players[player].health <= 0) return ST_JOSEPH_AND_ST_MARY;
		return gv->players[player].moves->tail->item;
	}
        
}

PlaceId GvGetVampireLocation(GameView gv) { return gv->encounters.vamp_location; }

PlaceId *GvGetTrapLocations(GameView gv, int *numTraps) {
    int count = 0;
    TrailNode p = gv->players[PLAYER_DRACULA].moves->head;
    while (p != NULL) {
		int t = gv->encounters.trap[p->item];
        while (t) {
			t--;
            count++;
        }
		p = p->next;
    }
    PlaceId *trap_locations = malloc(sizeof(PlaceId) * count);
    p = gv->players[PLAYER_DRACULA].moves->head;
    int j = 0;
    while (p != NULL) {
		int t = gv->encounters.trap[p->item];
		while (t) {
            trap_locations[j] = p->item;
			t--;
			j++;
        }
		p = p->next;
    }
    *numTraps = count;
    return trap_locations;
}

////////////////////////////////////////////////////////////////////////
// Game History

PlaceId *GvGetMoveHistory(GameView gv, Player player, int *numReturnedMoves, bool *canFree) {
    // CHanged: 2020-07-24 Rachel
	(*numReturnedMoves) = 0;
	if (!gv->players[player].moves->size) return NULL;
    //*canFree = false;
    TrailNode curr = gv->players[player].moves->head;
	int space = gv->currRound + 1;
    PlaceId *moveHistory = malloc(sizeof(PlaceId) * space);
    int i = 0;
    while (curr != NULL) {
        moveHistory[i] = curr->item;
        (*numReturnedMoves)++;
        i++;
        curr = curr->next;
    }
    return moveHistory;
}

PlaceId *GvGetLastMoves(GameView gv, Player player, int numMoves, int *numReturnedMoves, bool *canFree) {
    // CHanged: 2020-07-24 Rachel
	(*numReturnedMoves) = 0;
	if (!gv->players[player].moves->size) return NULL;
	if (numMoves <= 0) return NULL;
    *canFree = true;
    (*numReturnedMoves) = min((gv->players[player].moves->size), numMoves);
    PlaceId *lastMoves = malloc(sizeof(PlaceId) * (*numReturnedMoves));
    numMoves = *numReturnedMoves - 1;
    TrailNode curr = gv->players[player].moves->tail;
    while (numMoves >= 0) {
        lastMoves[numMoves] = curr->item;
        numMoves--;
        curr = curr->prev;
    }
    return lastMoves;
}

PlaceId *GvGetLocationHistory(GameView gv, Player player, int *numReturnedLocs, bool *canFree) {
    // Changed: 2020-07-24 VVhou
	*numReturnedLocs = 0;
	if (!gv->players[player].moves->size) return NULL;
    TrailNode move = gv->players[player].moves->head;
    int number = gv->currRound;
    PlaceId *locationhistory = malloc(sizeof(PlaceId) * number);
    if (player != PLAYER_DRACULA) {
        int i = 0;
        while (move != NULL) {
            locationhistory[i] = move->item;
            (*numReturnedLocs)++;
            move = move->next;
            i++;
        }
        *canFree = false;
    } else {
        int i = 0;
        while (move != NULL) {
            locationhistory[i] = move->item;
            if ((DOUBLE_BACK_1 <= locationhistory[i] && locationhistory[i] <= DOUBLE_BACK_5) || (locationhistory[i] = TELEPORT) || (locationhistory[i] = HIDE)) {
                locationhistory[i] = get_Dracula_Location(move);
            }
            (*numReturnedLocs)++;
            move = move->next;
            i++;
        }
        *canFree = false;
    }
    return locationhistory;
}

PlaceId *GvGetLastLocations(GameView gv, Player player, int numLocs, int *numReturnedLocs, bool *canFree) {
    // DONE: GvGetLastLocations
	*numReturnedLocs = 0;
    if (!gv->players[player].moves->size) return NULL;
	if (numLocs <= 0) return NULL;

	*numReturnedLocs = min(numLocs, gv->players[player].moves->size);
	PlaceId *lastLocs = malloc(sizeof(PlaceId) * (*numReturnedLocs));
	bool Free = false;
	if (player != PLAYER_DRACULA) {
		lastLocs = GvGetLastMoves(gv, player, numLocs, numReturnedLocs, &Free);
	}
	else {
		// dracula's move may contain non-real place
		int num_locs = *numReturnedLocs - 1;
		TrailNode curr = gv->players[PLAYER_DRACULA].moves->tail;
		for (int i = 0; i < (*numReturnedLocs); i++) {
			if (!placeIsReal(curr->item) && curr->item != CITY_UNKNOWN && curr->item != SEA_UNKNOWN && curr->item != UNKNOWN_PLACE) {
				PlaceId exact_loc = get_Dracula_Location(curr);
				lastLocs[num_locs] = exact_loc;
				num_locs--;
				curr = curr->prev;
				continue;
			}
			lastLocs[num_locs] = curr->item;
			num_locs--;
			curr = curr->prev;
		}
	}

	// avoid memory leak
	*canFree = true;
	return lastLocs;
}
////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *GvGetReachable(GameView gv, Player player, Round round, PlaceId from, int *numReturnedLocs) {
    // DONE: GvGetReachable
    if (player == PLAYER_DRACULA) {
        return GvGetReachableByType(gv, player, round, from, true, false, true, numReturnedLocs);
    } else {
        return GvGetReachableByType(gv, player, round, from, true, true, true, numReturnedLocs);
    }
}

PlaceId *GvGetReachableByType(GameView gv, Player player, Round round, PlaceId from, bool road, bool rail, bool boat, int *numReturnedLocs) {
    // DONE: GvGetReachableByType
    if (player == PLAYER_DRACULA) {
        // if there is non-extra move return Dracula location
        // else return unknown place
        from = get_Dracula_Location(gv->players[PLAYER_DRACULA].moves->tail);
    }

    assert(placeIsReal(from));

    Queue ValidMove = newQueue();
    PlaceId *loc;
    Map m = MapNew();
	/* PlaceId *pred = malloc(sizeof(PlaceId) * NUM_REAL_PLACES);
	for (int i = 0; i < NUM_REAL_PLACES; i++) pred[i] = -1; */

    if (road) {
        Queue road_move = RoadConnections(gv, from, player, m);
        QueueMerge(ValidMove, road_move);
    }

    if (rail) {
        Queue rail_move = RailConnections(gv, from, player, m, round);
        QueueMerge(ValidMove, rail_move);
    }

    if (boat) {
        Queue boat_move = BoatConnections(gv, from, player, m);
        QueueMerge(ValidMove, boat_move);
    }

    /* Queue extra_moves = connections_get_extras(gv, from, player);
    QueueMerge(ValidMove, extra_moves); */

    int size_q = queueLength(ValidMove);

    if (player == PLAYER_DRACULA && size_q == 0) {
        loc = malloc(sizeof(PlaceId));
        loc[0] = TELEPORT;
        *numReturnedLocs = 1;
    } else {
        loc = malloc(size_q * sizeof(PlaceId));
        for (int i = 0; i < size_q; i++) {
            loc[i] = leaveQueue(ValidMove);
        }
        *numReturnedLocs = size_q;
    }
    dropQueue(ValidMove);
    MapFree(m);
    return loc;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions
void PrintSummary(GameView gv, Player currPlayer_n) {
    // DONE: PrintSummary
    // Game summary
    printf_blue("\n\n------------SUMMARY------------\n");
    printf("    %d turns made. Now in round no %d\n    I am player: %d (%s)\n", gv->currTurn, gv->currRound, currPlayer_n, currPlayer_n == 4 ? "Dracula" : "Hunter");
    //    printf("    Player `%c` @ `%s` | HP: %d\n", player, location, lID,
    //    _event);
    for (enum player i = 0; i < PLAYER_DRACULA; i++) {
        printf("    Hunter %d @ %-15s | HP: %d\n", i, placeIdToName(gv_get_location(gv, i)), gv->players[i].health);
    }

    PlaceId draculaLocation = gv_get_location(gv, PLAYER_DRACULA);
    switch (draculaLocation) {
        case DOUBLE_BACK_1:
        case DOUBLE_BACK_2:
        case DOUBLE_BACK_3:
        case DOUBLE_BACK_4:
        case DOUBLE_BACK_5:
        case HIDE:
            printf("    Dracula  @ %-15s | HP: %d\n", placeIdToName(get_Dracula_Location(gv->players[PLAYER_DRACULA].moves->tail)), gv->players[PLAYER_DRACULA].health);
            break;
        default:
            printf("    Dracula  @ %-15s | HP: %d\n", placeIdToName(draculaLocation), gv->players[PLAYER_DRACULA].health);
    }

    printf_yellow("    SCORE: %d\n", GvGetScore(gv));
    printf_blue("------------SUMMARY------------\n\n\n");
}

/* Get the current location a given player is at
 * Needs to account for whether they're at the hospital or not (Not in TRAIL) */
PlaceId gv_get_location(GameView gv, Player player) {
    if (gv->players[player].health <= 0) return HOSPITAL_PLACE;
    return gv->players[player].moves->tail->item;
}

/* Checks whether a player (HUNTER) has rested */
static bool playerRested(GameView gv, Player p) {
    assert(p != PLAYER_DRACULA);
	if (gv->players[p].moves->head == NULL) return false;
    // Current location of Hunter (must exist so assert)
    TrailNode currNode = gv->players[p].moves->tail;
    assert(currNode);
    PlaceId currLocation = currNode->item;

    // Previous location of Hunter (if it exists)
    TrailNode prevNode = currNode->prev;
    if (!prevNode) return false;
    PlaceId prevLocation = currNode->prev->item;

    return currLocation == prevLocation;
}

/* Given an array, fill that array out with the given player's past moves */
void gv_get_history(GameView gv, enum player player, PlaceId trail[TRAIL_SIZE]) {
    TrailNode move = gv->players[player].moves->tail;
    for (int i = 0; i < TRAIL_SIZE; i++) {
        // Need to check if the node exists (May be less than 6 moves played)
        trail[i] = move ? move->item : -1;
        if (move) move = move->prev;
    }
}