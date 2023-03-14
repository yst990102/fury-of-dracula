////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// testGameView.c: test the GameView ADT
//
// As supplied, these are very simple tests.  You should write more!
// Don't forget to be rigorous and thorough while writing tests.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v1.2	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Game.h"
#include "GameView.h"
#include "Places.h"
#include "testUtils.h"

int main(void) {
    {  ///////////////////////////////////////////////////////////////////
        printf("Testing last move/location \n");
        char *trail =
            "GLS.... SGE.... HGE.... MGE.... DST.V.. "
            "GCA.... SGE.... HGE.... MGE.... DC?T... "
            "GGR.... SGE.... HGE.... MGE.... DC?T... "
            "GAL.... SGE.... HGE.... MGE.... DD3T... "
            "GSR.... SGE.... HGE.... MGE.... DHIT... "
            "GSN.... SGE.... HGE.... MGE.... DC?T... "
            "GMA.... SSTTTV.";

        Message messages[32] = {};
        GameView gv = GvNew(trail, messages);

        assert(GvGetHealth(gv, PLAYER_DR_SEWARD) == GAME_START_HUNTER_LIFE_POINTS - 2 * LIFE_LOSS_TRAP_ENCOUNTER);
        assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == CITY_UNKNOWN);
        assert(GvGetVampireLocation(gv) == NOWHERE);

        // Lord Godalming's  last move/location
        {
            printf("Test for Lord Godalming's  last move/location\n");
            int numMoves = 7;
            bool canFree = false;
            int numReturnedMoves = 0;
            PlaceId *moves = GvGetLastMoves(gv, PLAYER_LORD_GODALMING, numMoves, &numReturnedMoves, &canFree);
            assert(numReturnedMoves == 7);
            assert(moves[0] == LISBON);
            assert(moves[1] == CADIZ);
            assert(moves[2] == GRANADA);
            assert(moves[3] == ALICANTE);
            assert(moves[4] == SARAGOSSA);
            assert(moves[5] == SANTANDER);
            assert(moves[6] == MADRID);
            if (canFree) free(moves);
            printf("Test 1 passed\n");
        }

        {
            int numMoves = 8;
            bool canFree = false;
            int numReturnedMoves = 0;
            PlaceId *moves = GvGetLastMoves(gv, PLAYER_LORD_GODALMING, numMoves, &numReturnedMoves, &canFree);
            assert(numReturnedMoves == 7);
            assert(moves[0] == LISBON);
            assert(moves[1] == CADIZ);
            assert(moves[2] == GRANADA);
            assert(moves[3] == ALICANTE);
            assert(moves[4] == SARAGOSSA);
            assert(moves[5] == SANTANDER);
            assert(moves[6] == MADRID);
            if (canFree) free(moves);
            printf("Test 2 passed\n");
        }
        {
            int numMoves = 6;
            bool canFree = false;
            int numReturnedMoves = 0;
            PlaceId *moves = GvGetLastMoves(gv, PLAYER_LORD_GODALMING, numMoves, &numReturnedMoves, &canFree);
            assert(numReturnedMoves == 6);

            assert(moves[0] == CADIZ);
            assert(moves[1] == GRANADA);
            assert(moves[2] == ALICANTE);
            assert(moves[3] == SARAGOSSA);
            assert(moves[4] == SANTANDER);
            assert(moves[5] == MADRID);
            if (canFree) free(moves);
            printf("Test 3 passed\n");
        }
        {
            int numMoves = 0;
            bool canFree = false;
            int numReturnedMoves = 0;
            PlaceId *moves = GvGetLastMoves(gv, PLAYER_LORD_GODALMING, numMoves, &numReturnedMoves, &canFree);
            assert(moves == NULL);
            printf("Test 4 passed\n");
        }
        // dracula lastmove/location
        {
            printf("Test for dracula lastmove/location\n");
            int numMoves = 7;
            bool canFree = false;
            int numReturnedMoves = 0;
            PlaceId *moves = GvGetLastMoves(gv, PLAYER_DRACULA, numMoves, &numReturnedMoves, &canFree);
            assert(numReturnedMoves == 6);
            assert(moves[0] == STRASBOURG);
            assert(moves[1] == CITY_UNKNOWN);
            assert(moves[2] == CITY_UNKNOWN);
            assert(moves[3] == DOUBLE_BACK_3);
            assert(moves[4] == HIDE);
            assert(moves[5] == CITY_UNKNOWN);
            if (canFree) free(moves);
            printf("Test 1 passed\n");
        }
        {
            int numMoves = 6;
            bool canFree = false;
            int numReturnedMoves = 0;
            PlaceId *moves = GvGetLastMoves(gv, PLAYER_DRACULA, numMoves, &numReturnedMoves, &canFree);
            assert(numReturnedMoves == 6);
            assert(moves[0] == STRASBOURG);
            assert(moves[1] == CITY_UNKNOWN);
            assert(moves[2] == CITY_UNKNOWN);
            assert(moves[3] == DOUBLE_BACK_3);
            assert(moves[4] == HIDE);
            assert(moves[5] == CITY_UNKNOWN);
            if (canFree) free(moves);
            printf("Test 2 passed\n");
        }
        {
            int numMoves = 5;
            bool canFree = false;
            int numReturnedMoves = 0;
            PlaceId *moves = GvGetLastMoves(gv, PLAYER_DRACULA, numMoves, &numReturnedMoves, &canFree);
            assert(numReturnedMoves == 5);

            assert(moves[0] == CITY_UNKNOWN);
            assert(moves[1] == CITY_UNKNOWN);
            assert(moves[2] == DOUBLE_BACK_3);
            assert(moves[3] == HIDE);
            assert(moves[4] == CITY_UNKNOWN);
            if (canFree) free(moves);
            printf("Test 3 passed\n");
        }
        {
            int numMoves = 0;
            bool canFree = false;
            int numReturnedMoves = 0;
            PlaceId *moves = GvGetLastMoves(gv, PLAYER_DRACULA, numMoves, &numReturnedMoves, &canFree);
            assert(moves == NULL);
            printf("Test 4 passed\n");
        }
        {
            int numMoves = 7;
            bool canFree = false;
            int numReturnedMoves = 0;
            PlaceId *locs = GvGetLastLocations(gv, PLAYER_DRACULA, numMoves, &numReturnedMoves, &canFree);
            assert(numReturnedMoves == 6);
            assert(locs[0] == STRASBOURG);
            assert(locs[1] == CITY_UNKNOWN);
            assert(locs[2] == CITY_UNKNOWN);
            assert(locs[3] == STRASBOURG);
            assert(locs[4] == STRASBOURG);
            assert(locs[5] == CITY_UNKNOWN);
            if (canFree) free(locs);
            printf("Test 5 passed\n");
        }
        {
            int numMoves = 6;
            bool canFree = false;
            int numReturnedMoves = 0;
            PlaceId *locs = GvGetLastLocations(gv, PLAYER_DRACULA, numMoves, &numReturnedMoves, &canFree);
            assert(numReturnedMoves == 6);
            assert(locs[0] == STRASBOURG);
            assert(locs[1] == CITY_UNKNOWN);
            assert(locs[2] == CITY_UNKNOWN);
            assert(locs[3] == STRASBOURG);
            assert(locs[4] == STRASBOURG);
            assert(locs[5] == CITY_UNKNOWN);
            if (canFree) free(locs);
            printf("Test 6 passed\n");
        }
        {
            int numMoves = 5;
            bool canFree = false;
            int numReturnedMoves = 0;
            PlaceId *locs = GvGetLastLocations(gv, PLAYER_DRACULA, numMoves, &numReturnedMoves, &canFree);
            assert(numReturnedMoves == 5);

            assert(locs[0] == CITY_UNKNOWN);
            assert(locs[1] == CITY_UNKNOWN);
            assert(locs[2] == STRASBOURG);
            assert(locs[3] == STRASBOURG);
            assert(locs[4] == CITY_UNKNOWN);
            if (canFree) free(locs);
            printf("Test 7 passed\n");
        }
        {
            int numMoves = 0;
            bool canFree = false;
            int numReturnedMoves = 0;
            PlaceId *locs = GvGetLastLocations(gv, PLAYER_DRACULA, numMoves, &numReturnedMoves, &canFree);
            assert(locs == NULL);
            printf("Test 8 passed\n");
        }

        printf("============ALL Test Passed!===============\n");
    }
    return EXIT_SUCCESS;
}