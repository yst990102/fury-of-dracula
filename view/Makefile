########################################################################
# COMP2521 20T2 // the Fury of Dracula // the View
# view/Makefile: build tests for GameView/HunterView/DraculaView
#
# You can modify this if you add additional files
#
# 2018-12-31	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
# 2020-07-10    v2.0    Team Dracula <cs2521@cse.unsw.edu.au>
#
########################################################################

CC = gcc
CFLAGS = -Wall -Werror -g
BINS = testGameView testHunterView testDraculaView testMap gv_extra_tests

OBJS = GameView.o Map.o Places.o Queue.o Trail.o Connection.o

all: $(BINS)

gv_extra_tests: gv_extra_tests.o ${OBJS}
gv_extra_tests.o: Game.h GameView.h Places.h testUtils.h 

testGameView: testGameView.o testUtils.o ${OBJS}
testGameView.o: testGameView.c GameView.h Map.h Places.h Game.h Trail.h Connection.h

testHunterView: testHunterView.o testUtils.o HunterView.o ${OBJS}
testHunterView.o: testHunterView.c HunterView.h GameView.h Map.h Places.h Game.h Connection.h

testDraculaView: testDraculaView.o testUtils.o DraculaView.o ${OBJS}
testDraculaView.o: testDraculaView.c DraculaView.h GameView.h Map.h Places.h Game.h

GameView: GameView.o
testMap: testMap.o Map.o Places.o
testMap.o: testMap.c Map.h Places.h

Places.o: Places.c Places.h Game.h
Map.o: Map.c Map.h Places.h Game.h
GameView.o:	GameView.c GameView.h Game.h 
HunterView.o: HunterView.c HunterView.h Game.h
DraculaView.o: DraculaView.c DraculaView.h Game.h
Queue.o: Queue.c Queue.h
Trail.o: Trail.c Trail.h
Connection.o: Connection.c Connection.h Trail.h

testUtils.o: testUtils.c Places.h Game.h

.PHONY: clean
clean:
	-rm -f ${BINS} *.o core
