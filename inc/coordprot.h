#ifndef inc_COORD_PROT
#define inc_COORD_PROT

#include "../inc/coordinc.h"
using namespace std;
int coordSendPosnMsg (vector<int> pigPorts, vector<int> wallPosns, 
                       vector<int> pigPosns, int birdLoc);
void coordMsgHandler (int inMsgSize, char *inMsg);
int coordSendBirdLandMsg (vector<int> pigPorts, vector<int> wallPosns, 
                          vector<int> pigPosns, int birdLoc);

#endif
