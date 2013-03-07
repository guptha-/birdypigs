#ifndef inc_PIG_DS
#define inc_PIG_DS

#include "../inc/piginc.h"

// The pig's own info. Using atomic, so no other locks required to protect.
typedef struct
{
  atomic<short unsigned int> portNumber;
  atomic<short unsigned int> logNbrPorts[2];
  atomic<short unsigned int> physLoc;
  atomic<short unsigned int> birdLoc;
  atomic<bool> isHit;
}PigOwnNode;
extern PigOwnNode gPigOwnNode;
// END pig's own info

// Other pig nodes and associated objects
struct sPigOtherList
{
  vector<int> portNumber;
  vector<int> physLoc;
  deque<bool> wasHit;
  vector<bool> gotResp;
};
typedef struct sPigOtherList PigOtherList;
extern mutex gPigOtherMutex;
extern PigOtherList gPigOtherList;
// END other pig nodes

// Wall nodes and associated objects
struct sPigWallList
{
  vector<int> physLoc;
};
typedef struct sPigWallList PigWallList;
extern mutex gPigWallMutex;
extern PigWallList gPigWallList;
// END wall nodes

#endif
