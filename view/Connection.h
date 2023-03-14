#ifndef Connection_H
#define Connection_H

#include "GameView.h"
#include "Map.h"
#include "Places.h"
#include "Queue.h"
#include "Trail.h"
// Takes in 'non-exact' moves and returns exactly where Dracula is
// if not resolved to an unknown location
PlaceId get_Dracula_Location(TrailNode position);

// Finds all locations accessible by Road
Queue RoadConnections(GameView gv, PlaceId position, Player p, Map m);

// Finds all locations accessible by Rail
Queue RailConnections(GameView gv, PlaceId position, Player p, Map m, Round round);

// Finds all locations accessible by Boat
Queue BoatConnections(GameView gv, PlaceId position, Player p, Map m);

// Checks if a location is in a player's trail
bool location_in_trail(GameView gv, Player player, PlaceId loc);
// Check if an ARRAY of locations is within a players trail
bool locations_in_trail(GameView gv, Player player, PlaceId *loc, int nLoc);

// find possible rail paths using BFS
Queue connections_rail_bfs(PlaceId loc, Map m, int depth);

// Helper function for BFS to enqueue items
// Returns the number of nodes added to the queue
int connections_bfs_process(Queue q, int item, bool *hasBeenVisited, Map m);

// Consider extra moves:
// Hunter: Rest
// Dracula: HIDE && DOUBLE_BACK_N
Queue connections_get_extras(GameView gv, PlaceId location, Player player);

bool connections_is_location_connected(Map m, PlaceId locA, PlaceId locB, bool road, bool rail, bool sea);

void MoveNonPlaceFromPlaceIdArray(PlaceId *array, int *length);

#endif