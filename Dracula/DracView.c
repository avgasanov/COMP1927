// DracView.c ... DracView ADT implementation

#include <stdlib.h>
#include <assert.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "DracView.h"
#include "commonFunctions.h"

typedef struct _encounterData {
    LocationID location;    // location ID of the trails
    int numTraps;           // number of traps in the location
    int numVamps;           // number of vampires in the location
} encounterData;

struct dracView {
    GameView view;
    encounterData *trail[TRAIL_SIZE];    // encounter detail in Dracula's trail
};

// Creates a new DracView to summarise the current state of the game
DracView newDracView(char *pastPlays, PlayerMessage messages[]) {
    int i;
    LocationID trail[TRAIL_SIZE];
    DracView dracView = malloc(sizeof(struct dracView));
    assert(dracView != NULL);
    dracView->view = newGameView(pastPlays, messages);
    for (i = 0; i < TRAIL_SIZE ; i++) {
        dracView->trail[i] = malloc(sizeof(encounterData));    // malloc to every trail
        assert(dracView->trail[i] != NULL);
    }
    giveMeTheTrail(dracView, PLAYER_DRACULA, trail);
    for (i = 0; i < TRAIL_SIZE ; i++) {
        dracView->trail[i]->location = trail[i];
        whatsThere(dracView, trail[i], &dracView->trail[i]->numTraps, &dracView->trail[i]->numVamps);
    }
    return dracView;
}

// Frees all memory previously allocated for the DracView toBeDeleted
void disposeDracView(DracView toBeDeleted) {
    assert(toBeDeleted != NULL);
    int i;
    disposeGameView(toBeDeleted->view);
    for (i = 0; i < TRAIL_SIZE ; i++) {
        free(toBeDeleted->trail[i]);    // free every trail
        toBeDeleted->trail[i] = NULL;
    }
    free(toBeDeleted);
    toBeDeleted = NULL;
}

//// Functions to return simple information about the current state of the game

// Get the current round
Round giveMeTheRound(DracView currentView) {
    assert(currentView != NULL && currentView->view != NULL);
    return currentView->view->roundNumber;
}

// Get the current score
int giveMeTheScore(DracView currentView) {
    assert(currentView != NULL && currentView->view != NULL);
    return currentView->view->gameScore;
}


// Get the current health points for a given player
int howHealthyIs(DracView currentView, PlayerID player) {
    assert(currentView != NULL && currentView->view != NULL);
    assert(player >= PLAYER_LORD_GODALMING && player < NUM_PLAYERS);
    return currentView->view->players[player]->playerHealth;
}

// Get the current location id of a given player
LocationID whereIs(DracView currentView, PlayerID player) {
    assert(currentView != NULL && currentView->view != NULL);
    assert(player >= PLAYER_LORD_GODALMING && player < NUM_PLAYERS);
    return currentView->view->players[player]->playerCurrLocation;
}

// Get the most recent move of a given player
void lastMove(DracView currentView, PlayerID player, LocationID *start, LocationID *end) {
    assert(currentView != NULL && currentView->view != NULL);
    assert(player >= PLAYER_LORD_GODALMING && player < NUM_PLAYERS);
    LocationID trail[TRAIL_SIZE];
    getHistory(currentView->view, player, trail);    // fill the trail
    *start = trail[1];
    *end = trail[0];
    return;
}

// Find out what minions are placed at the specified location
void whatsThere(DracView currentView, LocationID where,
                int *numTraps, int *numVamps) {
    assert(currentView != NULL && currentView->view != NULL);
    assert(validPlace(where) || where == NOWHERE);
    int i;
    int firstMove = PLAYER_DRACULA * CHARS_PER_TURN;            //index place of first move of Dracula in pastPlays
    int nChar = countChar(currentView->view->pastPlays);        //number of chars in pastPlays
    LocationID trail[TRAIL_SIZE];
    *numTraps = 0;        //initialise to 0 before counting
    *numVamps = 0;        //initialise to 0 before counting
    if (where == NOWHERE || idToType(where) == SEA) {
        return;           //return 0 encounters when the place is not allowed to have one
    } else {
        initialiseTrail(trail);
        //loop till the last round
        for (i = firstMove; i < nChar - 2; i += CHARS_PER_ROUND) {
            //get location of Dracula in each round
            char currLocation[] = {currentView->view->pastPlays[i+1], currentView->view->pastPlays[i+2], '\0'};
            //change the "other" location to actual location of Dracula as Dracula always knows where he is
            LocationID currID = dracSpecialLocation(otherToID(currLocation), trail);
            updatePlayerTrail(trail, currID);    //update the trail of Dracula
            if (currID == where) {
                //if the location of that round is the same as the specified one, check the type and update the number of encounters occurred in that place
                numEncounter(trail, currentView->view->pastPlays[i+3], where, numTraps, numVamps);
                numEncounter(trail, currentView->view->pastPlays[i+4], where, numTraps, numVamps);
            }
        }
    }
    return;
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void giveMeTheTrail(DracView currentView, PlayerID player,
                    LocationID trail[TRAIL_SIZE]) {
    assert(currentView != NULL && currentView->view != NULL);
    assert(player >= PLAYER_LORD_GODALMING && player < NUM_PLAYERS);
    int i;
    int firstMove = player * CHARS_PER_TURN;                    //index place of first move of player in pastPlays
    int nChar = countChar(currentView->view->pastPlays);        //number of chars in pastPlays
    if (player == PLAYER_DRACULA) {
        initialiseTrail(trail);
        //loop till the last round
        for (i = firstMove; i < nChar - 2; i += CHARS_PER_ROUND) {
            //get current location of Dracula
            char currLocation[] = {currentView->view->pastPlays[i+1], currentView->view->pastPlays[i+2], '\0'};
            //change the "other" location to actual location of Dracula as Dracula always knows where he is
            LocationID currID = dracSpecialLocation(otherToID(currLocation), trail);
            updatePlayerTrail(trail, currID);    //update the trail of Dracula
        }
    } else {
        //put original locations in the trail if the player is a hunter
        getHistory(currentView->view, player, trail);
    }
    return;
}

//// Functions that query the map to find information about connectivity

// What are my (Dracula's) possible next moves (locations)
LocationID *whereCanIgo(DracView currentView, int *numLocations, int road, int sea) {
    assert(currentView != NULL && currentView->view != NULL);
    return connectedLocations(currentView->view, numLocations, whereIs(currentView, PLAYER_DRACULA),
                              PLAYER_DRACULA, giveMeTheRound(currentView), road, FALSE, sea);
}

// What are the specified player's next possible moves
LocationID *whereCanTheyGo(DracView currentView, int *numLocations,
                           PlayerID player, int road, int rail, int sea) {
    assert(currentView != NULL && currentView->view != NULL);
    assert(player >= PLAYER_LORD_GODALMING && player < NUM_PLAYERS);
    return connectedLocations(currentView->view, numLocations, whereIs(currentView, player),
                              player, giveMeTheRound(currentView), road, rail, sea);
}
