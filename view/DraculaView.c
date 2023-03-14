////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// DraculaView.c: the DraculaView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include "DraculaView.h"

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
// add your own #includes here

// TODO: ADD YOUR OWN STRUCTS HERE

struct draculaView
{
	GameView gv;
} draculaView;

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char *pastPlays, Message messages[])
{
	DraculaView new = malloc(sizeof(*new));
	if (new == NULL)
	{
		fprintf(stderr, "Couldn't allocate DraculaView\n");
		exit(EXIT_FAILURE);
	}
	new->gv = GvNew(pastPlays, messages);
	return new;
}

void DvFree(DraculaView dv)
{
	GvFree(dv->gv);
	free(dv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round DvGetRound(DraculaView dv) { return GvGetRound(dv->gv); }

int DvGetScore(DraculaView dv) { return GvGetScore(dv->gv); }

int DvGetHealth(DraculaView dv, Player player) { return GvGetHealth(dv->gv, player); }

PlaceId DvGetPlayerLocation(DraculaView dv, Player player) { return GvGetPlayerLocation(dv->gv, player); }

PlaceId DvGetVampireLocation(DraculaView dv) { return GvGetVampireLocation(dv->gv); }

PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps) { return GvGetTrapLocations(dv->gv, numTraps); }

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves)
{
	int num_places_without = 5;
	PlaceId *withoutTrial = GvGetReachableByType(dv->gv, PLAYER_DRACULA, GvGetRound(dv->gv),
												 GvGetPlayerLocation(dv->gv, PLAYER_DRACULA),
												 true, false, true, &num_places_without);
	PlaceId *withTrail = malloc(sizeof(PlaceId) * num_places_without);
	int num_trail = -1;
	bool canFree = false;
	PlaceId *my_trail = GvGetLastLocations(dv->gv, PLAYER_DRACULA, TRAIL_SIZE, &num_trail, &canFree);
	*numReturnedMoves = 0;
	for (int i = 0; i < num_places_without; i++)
	{
		if (containArray(my_trail, num_trail, withoutTrial[i]))
		{
			// if location is in the trail
			// check whether there are special moves to reach that location
			TrailNode curr = dv->gv->players[PLAYER_DRACULA].moves->tail;
			int counter = 0;
			// check how far that location is away from current location in the trail
			while (curr != NULL)
			{
				if (curr->item != withoutTrial[i])
					counter++;
				else
					break;
				curr = curr->prev;
			}
			// find corresponding special moves to get to that location
			// disregard that location if the special move is not available
			if (!counter)
			{
				if (!dv->gv->Dracula_Move.hide)
				{
					withTrail[*numReturnedMoves] = HIDE;
					(*numReturnedMoves)++;
				}
				if (!dv->gv->Dracula_Move.doubleBack)
				{
					withTrail[*numReturnedMoves] = DOUBLE_BACK_1;
					(*numReturnedMoves)++;
				}
			}
			else
			{
				if (!dv->gv->Dracula_Move.doubleBack)
				{
					switch (counter)
					{
					case 1:
						withTrail[*numReturnedMoves] = DOUBLE_BACK_2;
						break;
					case 2:
						withTrail[*numReturnedMoves] = DOUBLE_BACK_3;
						break;
					case 3:
						withTrail[*numReturnedMoves] = DOUBLE_BACK_4;
						break;
					case 4:
						withTrail[*numReturnedMoves] = DOUBLE_BACK_5;
						break;
					}
					(*numReturnedMoves)++;
				}
			}
		}
		else
		{
			// if location is not in trail, save it
			withTrail[*numReturnedMoves] = withoutTrial[i];
			(*numReturnedMoves)++;
		}
	}
	if (canFree)
		free(my_trail);
	// if Dracula has no legal move, return NULL
	if (!(*numReturnedMoves))
	{
		return NULL;
	}
	return withTrail;
}

PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs)
{
	int num_valid_moves = -1;
	// locations can be reached without considering trail
	PlaceId *validMoves = DvGetValidMoves(dv, &num_valid_moves);
	int num_trail = -1;
	bool canFree = false;
	// collect Dracula's trail
	PlaceId *my_trail = GvGetLastLocations(dv->gv, PLAYER_DRACULA, TRAIL_SIZE, &num_trail, &canFree);
	*numReturnedLocs = 0;
	PlaceId *validLocs = malloc(sizeof(PlaceId) * num_valid_moves);
	for (int i = 0; i < num_valid_moves; i++)
	{
		if (!placeIsReal(validMoves[i]))
		{
			// if the move is not real location
			// find its corresponding location and store it
			TrailNode tail = dv->gv->players[PLAYER_DRACULA].moves->tail;
			for (int i = DOUBLE_BACK_1; i < validMoves[i]; i++)
			{
				i++;
				tail = tail->prev;
			}
			validLocs[*numReturnedLocs] = tail->item;
		}
		else
		{
			// if the move is real location
			// store it into the array
			validLocs[*numReturnedLocs] = validMoves[i];
		}
		(*numReturnedLocs)++;
	}
	if (canFree)
		free(my_trail);
	// if Dracula has no valid place to go, go to castle
	if (!(*numReturnedLocs))
	{
		return NULL;
	}
	return validLocs;
	/* int numLocs = 10;
	PlaceId *validMoves = DvGetValidMoves(dv, &numLocs);
	int numLocs1 = 6; bool canFree = false;
	PlaceId *locs = GvGetLastLocations(dv->gv, PLAYER_DRACULA, TRAIL_SIZE, &numLocs1, &canFree);
	int size = numLocs1;
	PlaceId *reach = (PlaceId*) malloc(sizeof(PlaceId) * numLocs);
	int count = 0;
	for (int i = 0; i < numLocs; i++) {
		int n = validMoves[i] - DOUBLE_BACK_1;
		if (validMoves[i] == HIDE || validMoves[i] == DOUBLE_BACK_1) {
			if (!containArray(reach, locs[size - 1])) {
				reach[count] = locs[size -1 ];
				count++;
			}
		} else if (n >= (DOUBLE_BACK_2 - DOUBLE_BACK_1) && n <= (DOUBLE_BACK_5 - DOUBLE_BACK_1)) {
			if (!containArray(reach, locs[size - 1])) {
				reach[count] = locs[size - 1];
				count++;
			}

		}else {
			if (!containArray(reach, validMoves[i])) {
				reach[count] = validMoves[i];
				count++;
			}
		}
	}
	*numReturnedLocs = count;
	return reach;*/
}

PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
							 int *numReturnedLocs)
{
	PlaceId *ValidLocs = DvWhereCanIGo(dv, numReturnedLocs);
	PlaceId *ValidLocsByType = malloc(sizeof(PlaceId) * (*numReturnedLocs));
	if (!*numReturnedLocs)
		return NULL;
	return ValidLocsByType;
	for (int i = 0; i < (*numReturnedLocs); i++)
	{
		Map m = MapNew();
		bool connect_by = connections_is_location_connected(m, DvGetPlayerLocation(dv, PLAYER_DRACULA),
															ValidLocs[i], road, false, boat);
		if (connect_by)
		{
			ValidLocsByType[*numReturnedLocs] = ValidLocs[i];
			(*numReturnedLocs)++;
		}
	}
}

PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
						  int *numReturnedLocs)
{
	if (DvGetPlayerLocation(dv, player) == NOWHERE)
	{
		*numReturnedLocs = 0;
		return NULL;
	}

	if (player != PLAYER_DRACULA)
	{
		int round = 0;
		Player currPlayer_n = dv->gv->currTurn % NUM_PLAYERS;
		if (currPlayer_n >= player)
		{
			// this player has already moved for this round
			// need to expect next round moves for him
			round = dv->gv->currRound + 1;
		}
		else
		{
			// this player haven't moved for this round
			round = dv->gv->currRound;
		}

		PlaceId from = dv->gv->players[player].moves->tail->item;
		return GvGetReachable(dv->gv, player, round, from, numReturnedLocs);
	}
	else
	{
		int numlocs = 5;
		PlaceId *DraGo = DvWhereCanIGo(dv, &numlocs);
		*numReturnedLocs = numlocs;
		return DraGo;
	}
}

PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
								bool road, bool rail, bool boat,
								int *numReturnedLocs)
{
	if (DvGetPlayerLocation(dv, player) == NOWHERE)
	{
		*numReturnedLocs = 0;
		return NULL;
	}

	if (player != PLAYER_DRACULA)
	{
		int round = 0;
		Player currPlayer_n = dv->gv->currTurn % NUM_PLAYERS;
		if (currPlayer_n >= player)
		{
			// this player has already moved for this round
			// need to expect next round moves for him
			round = dv->gv->currRound + 1;
		} else {
			// this player haven't moved for this round
			round = dv->gv->currRound;
		}

		PlaceId from = dv->gv->players[player].moves->tail->item;
		return GvGetReachableByType(dv->gv, player, round, from, road, rail, boat, numReturnedLocs);
	} else {
		int numlocs = 5;
		PlaceId *DraGo = DvWhereCanIGoByType(dv, road, boat, &numlocs);
		*numReturnedLocs = numlocs;
		return DraGo;
	}
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

bool containArray(PlaceId *trail, int num_trail, PlaceId loc)
{
	for (int i = 0; i < num_trail; i++)
	{
		if (trail[i] == loc)
			return true;
	}
	return false;
}