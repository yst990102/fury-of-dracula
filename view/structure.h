
#include "Trail.h"
#include "Game.h"
#include "Places.h"

typedef struct playerInfo playerInfo;
typedef int Turn;

struct playerInfo {
    enum player type;
    int health;
    Trail moves;
};

struct gameView {
    Turn currTurn;
    Round currRound;

    playerInfo *currPlayer;
    playerInfo players[NUM_PLAYERS];
    int score;

    struct Dracula_Move {
        int doubleBack;  // Preset to 0, set to 6 when done, -1 every round
        int vampFlyTime;
        int hide;
    } Dracula_Move;

    struct encounters {
        PlaceId vamp_location;
        int trap[NUM_REAL_PLACES];
    } encounters;
};