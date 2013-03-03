#ifndef inc_COORD_PROT
#define inc_COORD_PROT

#include "../inc/coordinc.h"
using namespace std;
int coordSendPosnMsg (vector<int> pigPorts, vector<int> wallPosns, 
                       vector<int> pigPosns);
void coordMsgHandler (int inMsgSize, char *inMsg);
void initRand ();

#endif
