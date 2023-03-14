
////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// GameView.c: GameView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10   v3.0    Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include "GameView.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Game.h"
#include "Map.h"
#include "Places.h"

// add your own #includes here
#include "Connection.h"
#include "Queue.h"
#include "Trail.h"
#include "pretty.h"

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
    gv->Dracula_Move.vampFlyTime = 0;  //???
                                       // no vampire been placed at the start
    gv->encounters.vamp_location = NOWHERE;

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
    if (pastPlays != NULL) moveStr = strtok(strdup(pastPlays), " ");  //???

    Player currPlayer_n;

    while (moveStr != NULL) {
        currPlayer_n = gv->currTurn % NUM_PLAYERS;
        assert(strlen(moveStr) == 7);

        char locationStr[3];
        assert(memcpy(locationStr, moveStr + 1, 2) != NULL);
        char event[5];
        assert(memcpy(event, moveStr + 3, 4) != NULL);

        // same player should have same playerinfo
        gv->currPlayer = &(gv->players[currPlayer_n]);

        // add current move to the trail
        PlaceId location = placeAbbrevToId(locationStr);
        Trail_push(gv->currPlayer->moves, location);  //???
        // find Dracula's exact location
        // PlaceId DraExactLocation = get_Dracula_Location(gv->currPlayer->moves->tail);//???
        PlaceId DraExactLocation = GvGetPlayerLocation(gv, PLAYER_DRACULA);  //???
                                                                             /*
                                                                             =====================================ACTIONS=====================================
                                                                             */
                                                                             // record Dradula's actions and its consequences
        if (currPlayer_n == PLAYER_DRACULA) {
            // when immature vampire matures deduct score
            if (gv->encounters.vamp_location && --gv->Dracula_Move.vampFlyTime == 0) {
                gv->score -= SCORE_LOSS_VAMPIRE_MATURES;
            }

            // handles timer for hide
            //
            if (location == HIDE) {
                assert(gv->Dracula_Move.hide == 0);
                assert(placeIdToType(DraExactLocation) != SEA);  //???

                int *length_move = malloc(sizeof(int *));
                *length_move = gv->players[PLAYER_DRACULA].moves->size;
                // PlaceId *trail = GvGetLastLocations(gv, PLAYER_DRACULA, 6, length_move, false); ???
                PlaceId *trail = GvGetLastMoves(gv, PLAYER_DRACULA, 6, length_move, false);  //???

                int min_num = min(6, *length_move);
                for (int i = 0; i < min_num; i++) {
                    if (trail[i] == HIDE) {
                        assert(trail[i] != HIDE);
                    }
                }

                // Dracula cannot make a HIDE move if he has made a HIDE move in the last 5 rounds.
                gv->Dracula_Move.hide = 5;
            }

            if (location <= DOUBLE_BACK_5 && location >= DOUBLE_BACK_1) {
                // Dracula cannot make a DOUBLE_BACK move if he has already made a DOUBLE_BACK move in the last 5 rounds.
                //assert(gv->Dracula_Move.doubleBack == 0);

                gv->Dracula_Move.doubleBack = 5;
            }

            if (location >= DOUBLE_BACK_1 && location <= DOUBLE_BACK_5) {
                assert((gv->currPlayer->moves->size - 1) > (location - DOUBLE_BACK_1));  //??? -2 means minus first nowhere and the last double back
            }

            if (event[0] == 'T') {
                // Place trap in current location
                // assert(gv->encounters.trap[location] < 3); ???
                assert(gv->encounters.trap[DraExactLocation] < 3);  //???

                // gv->encounters.trap[location]++; ???
                gv->encounters.trap[DraExactLocation]++;  //???
            }

            if (event[1] == 'V') {
                // Check if Dracula has placed vampires before
                assert(gv->currRound % 13 == 0);
                // assert(gv->encounters.trap[location] < 3);
                //assert(gv->encounters.trap[DraExactLocation] < 3);  //???

                assert(gv->encounters.vamp_location == NOWHERE);
                //assert(gv->Dracula_Move.vampFlyTime == 0);

                // gv->encounters.vamp_location = location;
                gv->encounters.vamp_location = DraExactLocation;  //???
                gv->Dracula_Move.vampFlyTime = 6;
            }

            if (event[2] == 'M') {
                int *length_move = malloc(sizeof(int *));
                *length_move = gv->players[PLAYER_DRACULA].moves->size;

                PlaceId *t = GvGetLastLocations(gv, PLAYER_DRACULA, 6, length_move, false);  //??????
                assert(gv->encounters.trap[t[0]] > 0);                                       //?????

                // gv->encounters.trap[location]--;
                gv->encounters.trap[t[0]]--;  //???

            } else if (event[2] == 'V') {
                // gv->score -= SCORE_LOSS_VAMPIRE_MATURES; //???

                gv->Dracula_Move.vampFlyTime = 0;
                gv->encounters.vamp_location = NOWHERE;
            }

        } else {
			if (gv->players[currPlayer_n].health == 0 && gv->currRound != 0) gv->players[currPlayer_n].health = GAME_START_HUNTER_LIFE_POINTS;
            bool isAlive = true;  // Check if Hunter is Alive
			
            char *hunter_move = event;
            while (*hunter_move != '.' && *hunter_move != '\0' && isAlive) {
                switch (*hunter_move) {
                    case 'T':
                        // Hunter triggger Dracula's traps
                        for (int i = 0; i < gv->encounters.trap[location]; i++) {
                            gv->players[currPlayer_n].health -= LIFE_LOSS_TRAP_ENCOUNTER;
                            gv->encounters.trap[location]--;

                            isAlive = (gv->players[currPlayer_n].health > 0) ? true : false;
                            if (isAlive) {
                                continue;
                            } else {
                                break;
                            }
                        }
                        break;
                    case 'V':
                        // Hunter kill the vampire
                        gv->Dracula_Move.vampFlyTime = 0;
                        gv->encounters.vamp_location = NOWHERE;
                        break;

                    case 'D':
                        // Hunter encounter with Dracula
                        gv->players[currPlayer_n].health -= LIFE_LOSS_DRACULA_ENCOUNTER;
                        gv->players[PLAYER_DRACULA].health -= LIFE_LOSS_HUNTER_ENCOUNTER;
                        isAlive = (gv->players[currPlayer_n].health > 0) ? true : false;
                        break;
                }
                hunter_move++;
            }
        }

        // Trail_push(gv->currPlayer->moves, location); ???

        /*
=====================================UPDATE INFORMATION=====================================
        */

        // Update Dracula info: Timers, Health, Score

        if (currPlayer_n == PLAYER_DRACULA) {
            // Update Timers

            if (location != HIDE) {
                gv->Dracula_Move.hide--;
            }
            if (location < DOUBLE_BACK_1 || location > DOUBLE_BACK_5) {
                gv->Dracula_Move.doubleBack--;
            }

            if (gv->Dracula_Move.vampFlyTime) gv->Dracula_Move.vampFlyTime--;  //???

            // Update Health: Loses 2 health everytime Drac is at sea
            if (placeIdToType(DraExactLocation) == SEA) {  //???
                gv->players[PLAYER_DRACULA].health -= LIFE_LOSS_SEA;

            } else if (DraExactLocation == CASTLE_DRACULA) {  //???
                gv->players[PLAYER_DRACULA].health += LIFE_GAIN_CASTLE_DRACULA;
            }

            // Update Score: decreases by 1 each time Dracula finishes a
            // turn.
            gv->score -= SCORE_LOSS_DRACULA_TURN;
            gv->currRound++;

            // Hunters need to update: Health
        } else {
            // Update Health: A resting player (stayed in the same city)
            // gains 3 health

            if (GvGetHealth(gv, currPlayer_n) > 0 && playerRested(gv, currPlayer_n)) {
                gv->players[currPlayer_n].health += LIFE_GAIN_REST;
            }

            /*bool isResearch = true;
            for (int i = PLAYER_LORD_GODALMING; i <= PLAYER_MINA_HARKER; i++) {
                if (playerRested(gv, i) == false) {
                    isResearch = false;
                }
            }
            if (isResearch) {
                if (gv->players[PLAYER_DRACULA].moves->size < 6) { //???
                    break;
                } else {
                    gv->players[PLAYER_DRACULA].moves->tail->item = DraExactLocation; //???
                }
            }*/
        }

        // Update currPlayer
        gv->currTurn++;

        moveStr = strtok(NULL, " ");
    }

    // Game summary
    // PrintSummary(gv, currPlayer_n);

    return gv;
}

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

    if (player == PLAYER_DRACULA)
        return get_Dracula_Location(gv->players[player].moves->tail);
    else
        return gv->players[player].moves->tail->item;
}

PlaceId GvGetVampireLocation(GameView gv) { return gv->encounters.vamp_location; }

PlaceId *GvGetTrapLocations(GameView gv, int *numTraps) {
    int count = 0;
    TrailNode p = gv->players[PLAYER_DRACULA].moves->head;
    while (p != NULL) {
        if (gv->encounters.trap[p->item]) {
            count++;
        }
		p = p->next;
    }
    PlaceId *trap_locations = malloc(sizeof(PlaceId) * count);
    p = gv->players[PLAYER_DRACULA].moves->head;
    int j = 0;
    while (p != NULL) {
        if (gv->encounters.trap[p->item]) {
            trap_locations[j] = p->item;
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
    *numReturnedMoves = 0;
    *canFree = true;
    TrailNode curr = gv->players[player].moves->head;
    PlaceId *moveHistory = malloc(sizeof(PlaceId) * (gv->currTurn) + 1);
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
    *canFree = true;
    (*numReturnedMoves) = gv->currRound < numMoves ? gv->currRound : numMoves;
    PlaceId *lastMoves = malloc(sizeof(PlaceId) * (*numReturnedMoves));
    numMoves = *numReturnedMoves;
    TrailNode curr = gv->players[player].moves->tail;
    while (numMoves > 0) {
        lastMoves[numMoves] = curr->item;
        numMoves--;
        curr = curr->prev;
    }
    return lastMoves;
}

PlaceId *GvGetLocationHistory(GameView gv, Player player, int *numReturnedLocs, bool *canFree) {
    // Changed: 2020-07-24 VVhou
    *numReturnedLocs = 0;
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
    int numofmoves = gv->players[player].moves->size;

    PlaceId *loc = malloc(min(numLocs, numofmoves) * sizeof(PlaceId));
    *numReturnedLocs = min(numLocs, numofmoves);

    TrailNode start = gv->players[player].moves->tail;
    TrailNode end = gv->players[player].moves->tail;
    for (int i = 1; i < *numReturnedLocs; i++) {
        start = start->prev;
    }

    if (start == end) {
        // only one location
        loc[0] = start->item;
    } else {
        int count = 0;
        while (start != end) {
            loc[count] = start->item;
            start = start->next;
            count++;
        }
    }

    *canFree = false;
    return loc;
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
    if (round == gv->currRound && player == PLAYER_DRACULA) {
        // if there is non-extra move return Dracula location
        // else return unknown place
        from = get_Dracula_Location(gv->players[PLAYER_DRACULA].moves->tail);
    }

    assert(placeIsReal(from));

    Queue ValidMove = newQueue();
    PlaceId *loc;
    Map m = MapNew();

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

    Queue extra_moves = connections_get_extras(gv, from, player);
    QueueMerge(ValidMove, extra_moves);

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