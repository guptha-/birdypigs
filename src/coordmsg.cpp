/*==============================================================================
 *
 *       Filename:  coordmsg.cpp
 *
 *    Description:  This file does msg processing/send for the coordinator.
 *
 * =============================================================================
 */

#include "../inc/coordinc.h"

/* ===  FUNCTION  ==============================================================
 *         Name:  handleWasHitUni
 *  Description:  This function receives the list of hit pigs from the closest
 *                pig
 * =============================================================================
 */
void handleWasHitUni (char *inMsg, int inMsgSize)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X----- Hit Count --->
  <- Hit Pig Port 1 --...
  */
  cout<<"COORD: Got the unicast was hit response"<<endl;
  short unsigned int hitCount;
  memcpy (&hitCount, inMsg, WAS_HIT_SIZE);
  hitCount = ntohs(hitCount);
  inMsg += WAS_HIT_SIZE;
  inMsgSize -= WAS_HIT_SIZE;

  if (hitCount == 0)
  {
    cout<<"No pig was hit!"<<endl;
  }
  while (hitCount--)
  {
    unsigned short int pigPort;
    memcpy (&pigPort, inMsg, SRC_PORT_SIZE);
    pigPort = ntohs(pigPort);
    inMsg += SRC_PORT_SIZE;
    inMsgSize -= SRC_PORT_SIZE;

    cout<<pigPort<<" was HIT!"<<endl;
  }

  sleep(10);

  if (EXIT_FAILURE == coordSpawnPigs ())
  {
    return;
  }

  // Give the spawned pigs a chance to get their bearings
  sleep(1);

  cout<<endl<<endl<<"STARTING NEW GAME"<<endl;
  coordStartGame();
  return;
}		/* -----  end of function handleWasHitUni  ----- */


/* ===  FUNCTION  ==============================================================
 *         Name:  coordMsgHandler
 *  Description:  This function accepts all the messages, finds out their type,
 *                and calls their respective handlers.
 * =============================================================================
 */
void coordMsgHandler (int inMsgSize, char *tempInMsg)
{
  char actualInMsg[MAX_MSG_SIZE];
  char *inMsg = actualInMsg;
  memset(inMsg, 0, MAX_MSG_SIZE);
  memcpy(inMsg, tempInMsg, MAX_MSG_SIZE);
  free (tempInMsg);
  if (inMsgSize < 2)
  {
    cout<<"Corrupted message at coord"<<endl;
    return;
  }

  short unsigned int msgType;
  memcpy (&msgType, inMsg, MSG_TYPE_SIZE);
  msgType = ntohs(msgType);
  inMsg += MSG_TYPE_SIZE;
  inMsgSize -= MSG_TYPE_SIZE;

  switch (msgType)
  {
    case WAS_HIT_UNI_MSG:
    { 
      // This is the only msg that is received at the coordinator. After 
      // processing, the game is done.
      handleWasHitUni (inMsg, inMsgSize);
      break;
    }
    default:
    {
      cout<<"Corrupted message at coord";
    }
  }
  return;
}


/* ===  FUNCTION  ==============================================================
 *         Name:  sendMsg
 *  Description:  This function sends the given msg to the give port
 * =============================================================================
 */
void sendMsg(char *outMsg, int outMsgSize, unsigned short int destPort)
{
  UDPSocket sendSocket;
  sendSocket.sendTo(outMsg, outMsgSize, COM_IP_ADDR, destPort);
  return;
}		/* -----  end of function sendMsg  ----- */



/* ===  FUNCTION  ==============================================================
 *         Name:  checkIfAffected
 *  Description:  Returns 1 if affected.
 * =============================================================================
 */
static int checkIfAffected (vector<int> wallPosns, 
                            vector<int> pigPosns, int birdLoc, int destPort,
                            int destLoc)
{
  // This function sees if the dest pig is affected by the bird in any way. If
  // it is, a flag is set in the message sent notifying it of the bird landing.
  int myLoc = destLoc;
  // All the walls are assumed to have a height of 3 units
  if (((birdLoc - myLoc) > 3) || ((birdLoc - myLoc) < -3))
  {
    // Not even a wall present close by with a pig adjacent can crush us.
    return 0;
  }

  bool moveRight = false;
  bool moveLeft = false;
  for (auto &wallLoc : wallPosns)
  {
    if (birdLoc != wallLoc)
    {
      // The wall won't directly be hit by the bird.
      continue;
    }
    if (((wallLoc - myLoc) < 3) && ((wallLoc - myLoc) > 0)) 
    {
      // Wall is to the right and the bird will hit it. Backpedal to the left.
      moveLeft = true;
      break;
    }
    if (((wallLoc - myLoc) > -3) && ((wallLoc - myLoc) < 0))
    {
      // Wall is to the left and the bird will hit it. Move right.
      moveRight = true;
      break;
    }
    if ((wallLoc - myLoc) == 3)
    {
      // There needs to be a pig adjacent to us for anything to go amiss
      for (auto &pigLoc : pigPosns)
      {
        if ((pigLoc - myLoc) == 1)
        {
          moveLeft = true;
          break;
        }
      }
    }
    else if ((wallLoc - myLoc) == -3)
    {
      // There needs to be a pig adjacent to us for anything to go amiss
      for (auto &pigLoc : pigPosns)
      {
        if ((pigLoc - myLoc) == -1)
        {
          moveRight = true;
          break;
        }
      }
    }
  }

  // At this point, if the bird hits a stone column, we know what to do.
  // We now need to see what to do if the bird hits a pig.
  
  // A pig can't topple a wall.
  

  for (auto &pigLoc : pigPosns)
  {
    if (pigLoc != birdLoc)
    {
      continue;
    }
    if ((pigLoc - myLoc) == 1)
    {
      moveLeft = true;
      break;
    }
    if ((pigLoc - myLoc) == -1)
    {
      moveRight = true;
      break;
    }
  }

  // Finally, if we are hit directly, move left or right.
  if (myLoc == birdLoc)
  {
    moveLeft = true;
    moveRight = true;
  }

  if ((moveLeft == true) || (moveRight == true))
  {
    return 1;
  }
  return 0;
}		/* -----  end of function checkIfAffected  ----- */


/* ===  FUNCTION  ==============================================================
 *         Name:  coordSendStatusReqMsg
 *  Description:  This function sends the status request message to the closest
 *                pig. The closest pig queries the other pigs and replies back
 *                with the pig IDs of the hit pigs.
 * =============================================================================
 */
int coordSendStatusReqMsg (int destPort)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ---->
  */
  char actualOutMsg[MAX_MSG_SIZE];
  memset(&actualOutMsg, 0, MAX_MSG_SIZE);
  char *outMsg = actualOutMsg;
  int outMsgSize = 0;
  if (MSG_TYPE_SIZE > MAX_MSG_SIZE)
  {
    cout<<"Message is too big to be sent"<<endl;
    return EXIT_FAILURE;
  }

  short unsigned int msgType = STATUS_REQ_UNI_MSG;
  msgType = htons(msgType);
  memcpy (outMsg, &msgType, MSG_TYPE_SIZE);
  outMsg += MSG_TYPE_SIZE;
  outMsgSize += MSG_TYPE_SIZE;

  sendMsg(actualOutMsg, outMsgSize, destPort);

  return EXIT_SUCCESS;

}		/* -----  end of function coordSendStatusReqMsg  ----- */
/* ===  FUNCTION  ==============================================================
 *         Name:  coordSendBirdLandMsg
 *  Description:  This function sends the bird lands message to all the pigs. 
 *                This information does not propagate via the p2p network 
 *                because there is no global clock.
 * =============================================================================
 */
int coordSendBirdLandMsg (vector<int> wallPosns, 
                          vector<int> pigPosns, int birdLoc)
{
  // This is the message sent once the bird has landed.
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X---- Dest Loc ----->
  <----- Is Target? -->
  */
  char actualOutMsg[MAX_MSG_SIZE];
  memset(actualOutMsg, 0, MAX_MSG_SIZE);
  char *outMsg = actualOutMsg;
  int outMsgSize = 0;
  if ((MSG_TYPE_SIZE + PORT_LOC_SIZE + IS_TARGET_SIZE) > MAX_MSG_SIZE)
  {
    cout<<"Message is too big to be sent"<<endl;
    return EXIT_FAILURE;
  }

  short unsigned int msgType = BIRD_LAND_MSG;
  msgType = htons(msgType);
  memcpy (outMsg, &msgType, MSG_TYPE_SIZE);
  outMsg += MSG_TYPE_SIZE;
  outMsgSize += MSG_TYPE_SIZE;

  char *storedMsgPtr = outMsg;
  unsigned short int index = 0;
  unsigned short int storedMsgSize = outMsgSize;
  gPigPortsMutex.lock();
  vector<int> tempPigPorts(pigPorts);
  gPigPortsMutex.unlock();
  for (auto &destPort : tempPigPorts)
  {
    outMsg = storedMsgPtr;
    outMsgSize = storedMsgSize;
    short unsigned int destLoc = pigPosns[index];
    unsigned short int isTarget = checkIfAffected(wallPosns, pigPosns,
                                                  birdLoc, destPort, destLoc);
    destLoc = htons(destLoc);
    memcpy (outMsg, &destLoc, PORT_LOC_SIZE);
    outMsg += PORT_LOC_SIZE;
    outMsgSize += PORT_LOC_SIZE;

    isTarget = htons(isTarget);
    memcpy (outMsg, &isTarget, IS_TARGET_SIZE);
    outMsg += IS_TARGET_SIZE;
    outMsgSize += IS_TARGET_SIZE;
    
    sendMsg(actualOutMsg, outMsgSize, destPort);
    index++;
  }

  return EXIT_SUCCESS;
}		/* -----  end of function coordBirdLangMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  coordSendPosnMsg
 *  Description:  This function sends the positions of all the pigs to the 
 *                closest pig. It also sends the landing posn of the bird.
 * =============================================================================
 */
int coordSendPosnMsg (vector<int> wallPosns, 
                       vector<int> pigPosns, int birdLoc)
{
  auto closestPig = min_element (pigPosns.begin(), pigPosns.end()) -
                    pigPosns.begin();
  gPigPortsMutex.lock();
  vector<int> tempPigPorts(pigPorts);
  gPigPortsMutex.unlock();
  short unsigned int closestPigPort = tempPigPorts[closestPig];
  
  // This is the message sent at the beginning of each launch. This 
  // gives details about the other pigs and walls.
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X---- Src Port ----->
  <---- Pig Count ----X---- Wall Count --->
  <---- Pig Port 1 ---X---- Pig Port 2 .... Pig Port N -->
  <---- Pig Loc 1 ----X---- Pig Loc 2 ..... Pig Loc N --->
  <---- Wall Loc 1 ---X---- Wall Loc 2 .... Wall Loc M -->
  <----- Bird Loc ----X---- Hop Limit ---->
  */
  char actualOutMsg[MAX_MSG_SIZE];
  char *outMsg = actualOutMsg;
  int outMsgSize = 0;
  if ((MSG_TYPE_SIZE + SRC_PORT_SIZE + NUMBER_PIGS_SIZE + NUMBER_WALLS_SIZE +
      (tempPigPorts.size() * (OTHER_PORT_SIZE + PORT_LOC_SIZE)) +
      (wallPosns.size() * WALL_LOC_SIZE) + PORT_LOC_SIZE + 
      HOP_LIMIT_SIZE) > MAX_MSG_SIZE)
  {
    cout<<"Message is too big to be sent"<<endl;
    return EXIT_FAILURE;
  }

  short unsigned int msgType = INIT_POSN_MSG;
  msgType = htons(msgType);
  memcpy (outMsg, &msgType, MSG_TYPE_SIZE);
  outMsg += MSG_TYPE_SIZE;
  outMsgSize += MSG_TYPE_SIZE;

  short unsigned int srcPort = 0;
  srcPort = htons(srcPort);
  memcpy (outMsg, &srcPort, SRC_PORT_SIZE);
  outMsg += SRC_PORT_SIZE;
  outMsgSize += SRC_PORT_SIZE;

  short unsigned int numberPigs = tempPigPorts.size();
  numberPigs = htons(numberPigs);
  memcpy (outMsg, &numberPigs, NUMBER_PIGS_SIZE);
  outMsg += NUMBER_PIGS_SIZE;
  outMsgSize += NUMBER_PIGS_SIZE;

  short unsigned int numberWalls = wallPosns.size();
  numberWalls = htons(numberWalls);
  memcpy (outMsg, &numberWalls, NUMBER_WALLS_SIZE);
  outMsg += NUMBER_WALLS_SIZE;
  outMsgSize += NUMBER_WALLS_SIZE;

  for (auto &otherPort : tempPigPorts)
  {
    otherPort = htons(otherPort);
    memcpy (outMsg, &otherPort, OTHER_PORT_SIZE);
    outMsg += OTHER_PORT_SIZE;
    outMsgSize += OTHER_PORT_SIZE;
  }

  for (auto &portLoc : pigPosns)
  {
    portLoc = htons(portLoc);
    memcpy (outMsg, &portLoc, PORT_LOC_SIZE);
    outMsg += PORT_LOC_SIZE;
    outMsgSize += PORT_LOC_SIZE;
  }

  for (auto &wallLoc : wallPosns)
  {
    wallLoc = htons(wallLoc);
    memcpy (outMsg, &wallLoc, WALL_LOC_SIZE);
    outMsg += WALL_LOC_SIZE;
    outMsgSize += WALL_LOC_SIZE;
  }

  birdLoc = htons(birdLoc);
  memcpy (outMsg, &birdLoc, PORT_LOC_SIZE);
  outMsg += PORT_LOC_SIZE;
  outMsgSize += PORT_LOC_SIZE;

  short unsigned int hopCount = tempPigPorts.size() / 2 + 1;
  hopCount = htons(hopCount);
  memcpy (outMsg, &hopCount, HOP_LIMIT_SIZE);
  outMsg += HOP_LIMIT_SIZE;
  outMsgSize += HOP_LIMIT_SIZE;

  sendMsg(actualOutMsg, outMsgSize, closestPigPort);
  return EXIT_SUCCESS;
}		/* -----  end of function coordSendPosnMsg  ----- */
