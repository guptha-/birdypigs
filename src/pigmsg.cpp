/*==============================================================================
 *
 *       Filename:  pigmsg.cpp
 *
 *    Description:  All the messages and their handlers.
 *
 * =============================================================================
 */
#include "../inc/piginc.h"

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
}   /* -----  end of function sendMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  computeLogicalNeighbors
 * =============================================================================
 */
static void computeLogicalNeighbors ()
{
  // The logical neighbors are the ports which are to either side of this one.
  
  short unsigned int myPort = gPigOwnNode.portNumber;
  
  gPigOtherMutex.lock();

  auto minIndex = min_element (gPigOtherList.portNumber.begin(), 
                              gPigOtherList.portNumber.end()) -
                 gPigOtherList.portNumber.begin();
  auto maxIndex = max_element (gPigOtherList.portNumber.begin(), 
                              gPigOtherList.portNumber.end()) - 
                 gPigOtherList.portNumber.begin();

  auto minElem = gPigOtherList.portNumber[minIndex];
  auto maxElem = gPigOtherList.portNumber[maxIndex];
  short unsigned int higherPort = MAX_PORT;
  short unsigned int lowerPort = 0;
  for (auto &tempElem : gPigOtherList.portNumber)
  {
    if ((tempElem < higherPort) && (tempElem > lowerPort))
    {
      if (tempElem > myPort)
      {
        higherPort = tempElem;
      }
      else if (tempElem < myPort)
      {
        lowerPort = tempElem;
      }
    }

  }

  if (myPort == minElem)
  {
    gPigOwnNode.logNbrPorts[0] = maxElem;
    gPigOwnNode.logNbrPorts[1] = higherPort;
  }
  else if (myPort == maxElem)
  {
    gPigOwnNode.logNbrPorts[0] = lowerPort;
    gPigOwnNode.logNbrPorts[1] = minElem;
  }
  else
  {
    gPigOwnNode.logNbrPorts[0] = lowerPort;
    gPigOwnNode.logNbrPorts[1] = higherPort;
  }
  gPigOtherMutex.unlock();

  return;
}		/* -----  end of function computeLogicalNeighbors  ----- */


/* ===  FUNCTION  ==============================================================
 *         Name:  handlePhyNbrMsg
 *  Description:  This function handles a physical nbr message
 * =============================================================================
 */
static void handlePhyNbrMsg (int inMsgSize, char *inMsg)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X---- Dest Port ---->
  <----- Dest Loc ----X---- Hop Limit ---->
  */
  char *permInMsg = inMsg;
  int permInMsgSize = inMsgSize;
  
  // For message type
  inMsg += MSG_TYPE_SIZE;
  inMsgSize -= MSG_TYPE_SIZE;

  if (inMsgSize < SRC_PORT_SIZE + PORT_LOC_SIZE + HOP_LIMIT_SIZE)
  {
    cout<<"Corrupted message at pig "<<gPigOwnNode.portNumber<<endl;
    return;
  }

  short unsigned int destPort;
  memcpy (&destPort, inMsg, SRC_PORT_SIZE);
  destPort = ntohs(destPort);
  inMsg += SRC_PORT_SIZE;

  cout<<gPigOwnNode.portNumber<<": Rec phy nbr msg for "<<destPort<<endl;
  short unsigned int destLoc;
  memcpy (&destLoc, inMsg, PORT_LOC_SIZE);
  destLoc = ntohs(destLoc);
  inMsg += PORT_LOC_SIZE;

  short unsigned int hopLimit;
  memcpy (&hopLimit, inMsg, HOP_LIMIT_SIZE);
  hopLimit = ntohs(hopLimit);

  if (destPort != gPigOwnNode.portNumber)
  {
    if (gPigOwnNode.logNbrPorts[1] == 0)
    {
      // We don't know the topology yet. Can't forward.
      return;
    }
    if (hopLimit == 0)
    {
      return;
    }
    // Forward it on.
    hopLimit--;
    hopLimit = htons(hopLimit);
    memcpy (inMsg, &hopLimit, HOP_LIMIT_SIZE);
    cout<<gPigOwnNode.portNumber<<": Forwarding nbr msg for "<<destPort<<endl;
    sendMsg(permInMsg, permInMsgSize, gPigOwnNode.logNbrPorts[1]);
    return;
  }

  // We are the intended recepient.
  cout<<gPigOwnNode.portNumber<<": Neighbor warning received, moving to "<<
    destLoc<<endl;
  gPigOwnNode.physLoc = destLoc;

  return;
}		/* -----  end of function informPhysicalNeighbor  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  informPhysicalNeighbor
 *  Description:  This function tries to inform physical neighbors to move.
 * =============================================================================
 */
void informPhysicalNeighbor (unsigned short int nbrPort, 
                             unsigned short int nbrLoc)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X---- Dest Port ---->
  <----- Dest Loc ----X---- Hop Limit ---->
  */

  cout<<gPigOwnNode.portNumber<<": Trying to inform physical nbr "<<nbrPort
    <<endl;
  char actualOutMsg[MAX_MSG_SIZE];
  char *outMsg = actualOutMsg;
  int outMsgSize = 0;
  if ((MSG_TYPE_SIZE + 2 * SRC_PORT_SIZE +  
      HOP_LIMIT_SIZE) > MAX_MSG_SIZE)
  {
    cout<<"Message is too big to be sent"<<endl;
    return;
  }

  short unsigned int msgType = INFORM_PHY_NBR_MSG;
  msgType = htons(msgType);
  memcpy (outMsg, &msgType, MSG_TYPE_SIZE);
  outMsg += MSG_TYPE_SIZE;
  outMsgSize += MSG_TYPE_SIZE;

  nbrPort = htons(nbrPort);
  memcpy (outMsg, &nbrPort, SRC_PORT_SIZE);
  outMsg += SRC_PORT_SIZE;
  outMsgSize += SRC_PORT_SIZE;

  nbrLoc = htons(nbrLoc);
  memcpy (outMsg, &nbrLoc, PORT_LOC_SIZE);
  outMsg += PORT_LOC_SIZE;
  outMsgSize += PORT_LOC_SIZE;

  gPigOtherMutex.lock();
  short unsigned int hopLimit = gPigOtherList.portNumber.size();
  gPigOtherMutex.unlock();
  hopLimit = htons(hopLimit);
  memcpy (outMsg, &hopLimit, HOP_LIMIT_SIZE);
  outMsg += HOP_LIMIT_SIZE;
  outMsgSize += HOP_LIMIT_SIZE;

  sendMsg(actualOutMsg, outMsgSize, gPigOwnNode.logNbrPorts[1]);
  return;
}		/* -----  end of function informPhysicalNeighbor  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  checkIfAffected
 *  Description:  This function sees if we are affected by the bird's landing.
 *                If any of our physical neighbors will be affected, we warn.
 * =============================================================================
 */
static void checkIfAffected ()
{
  // Most of the complex logic in this function is due to the assumption that
  // 2 pigs can't be in the same location.
  int myLoc = gPigOwnNode.physLoc;
  int birdLoc = gPigOwnNode.birdLoc;
  // All the walls are assumed to have a height of 3 units
  if (((birdLoc - myLoc) > 3) || ((birdLoc - myLoc) < -3))
  {
    // Not even a wall present close by with a pig adjacent can crush us.
    return;
  }

  bool moveRight = false;
  bool moveLeft = false;
  gPigWallMutex.lock();
  for (auto &wallLoc : gPigWallList.physLoc)
  {
    if (birdLoc != wallLoc)
    {
      // The wall won't directly be hit by the bird.
      continue;
    }
    if (((wallLoc - myLoc) < 3) && ((wallLoc - myLoc) > 0)) 
    {
      // Wall is to the right and the bird will hit it. Backpedal to the left.
      cout<<gPigOwnNode.portNumber<<": Nearby wall hit by bird!"<<endl;
      moveLeft = true;
      break;
    }
    if (((wallLoc - myLoc) > -3) && ((wallLoc - myLoc) < 0))
    {
      // Wall is to the left and the bird will hit it. Move right.
      cout<<gPigOwnNode.portNumber<<": Nearby wall hit by bird!"<<endl;
      moveRight = true;
      break;
    }
    if ((wallLoc - myLoc) == 3)
    {
      // There needs to be a pig adjacent to us for anything to go amiss
      gPigOtherMutex.lock();
      for (auto &pigLoc : gPigOtherList.physLoc)
      {
        if ((pigLoc - myLoc) == 1)
        {
          cout<<gPigOwnNode.portNumber<<": Nearby wall-pig chain hit!"<<endl;
          moveLeft = true;
          break;
        }
      }
      gPigOtherMutex.unlock();
    }
    else if ((wallLoc - myLoc) == -3)
    {
      // There needs to be a pig adjacent to us for anything to go amiss
      gPigOtherMutex.lock();
      for (auto &pigLoc : gPigOtherList.physLoc)
      {
        if ((pigLoc - myLoc) == -1)
        {
          cout<<gPigOwnNode.portNumber<<": Nearby wall-pig chain hit!"<<endl;
          moveRight = true;
          break;
        }
      }
      gPigOtherMutex.unlock();
    }
  }
  gPigWallMutex.unlock();

  // At this point, if the bird hits a stone column, we know what to do.
  // We now need to see what to do if the bird hits a pig.
  
  // A pig can't topple a wall.
  
  gPigOtherMutex.lock();

  for (auto &pigLoc : gPigOtherList.physLoc)
  {
    if (pigLoc != birdLoc)
    {
      continue;
    }
    if ((pigLoc - myLoc) == 1)
    {
      cout<<gPigOwnNode.portNumber<<": Nearby pig hit!"<<endl;
      moveLeft = true;
      break;
    }
    if ((pigLoc - myLoc) == -1)
    {
      cout<<gPigOwnNode.portNumber<<": Nearby pig hit!"<<endl;
      moveRight = true;
      break;
    }
  }

  gPigOtherMutex.unlock();

  // Finally, if we are hit directly, move left or right.
  if (myLoc == birdLoc)
  {
    moveLeft = true;
    moveRight = true;
  }

  // Now we move left or right. Note that it may not always be possible to move.
  // We move till we reach the next wall or bird. We assume that the pigs are
  // on steroids - once they decide to move somewhere, they get there in no time
  
  int canMove;
  short unsigned int origLoc = gPigOwnNode.physLoc;
  if (moveLeft == true)
  {
    // Let us see if there is any physical neighbor to our left. If there is, we
    // can try to save it.
    gPigOtherMutex.lock();
    unsigned short int index = 0;
    for (auto &pigLoc : gPigOtherList.physLoc)
    {
      if ((pigLoc - myLoc) == -1)
      {
        bool nbrCanMove = true;
        for (auto &tempPigLoc : gPigOtherList.physLoc)
        {
          if ((tempPigLoc - pigLoc) == -1)
          {
            // The neighbor can't move, don't send anything.
            nbrCanMove = false;
            break;
          }
        }
        if (nbrCanMove == true)
        {
          gPigOtherMutex.unlock();
          informPhysicalNeighbor (gPigOtherList.portNumber[index], pigLoc - 1);
          gPigOtherMutex.lock();
        }
        break;
      }
      index++;
    }
    gPigOtherMutex.unlock();

    // Updating myLoc. The global may have changed in the prev iteration.
    cout<<gPigOwnNode.portNumber<<": Trying to move left from "<<myLoc
      <<endl;
    while (true)
    {
      myLoc = gPigOwnNode.physLoc;
      canMove = true;
      gPigWallMutex.lock();
      for (auto &wallLoc : gPigWallList.physLoc)
      {
        if ((wallLoc - myLoc) == -1)
        {
          // Can't move any further.
          canMove = false;
          break;
        }
      }
      gPigWallMutex.unlock();

      gPigOtherMutex.lock();
      for (auto &pigLoc : gPigOtherList.physLoc)
      {
        if ((pigLoc - myLoc) == -1)
        {
          // Can't move any further
          canMove = false;
          break;
        }
      }
      gPigOtherMutex.unlock();

      if (canMove == false)
      {
        // Can't go left any further.
        cout<<gPigOwnNode.portNumber<<": Final Loc "<<gPigOwnNode.physLoc<<endl;
        break;
      }
      if (myLoc > 1)
      {
        gPigOwnNode.physLoc--;
      }
      else
      {
        // Can't go left any further.
        cout<<gPigOwnNode.portNumber<<": Final Loc "<<gPigOwnNode.physLoc<<endl;
        break;
      }
    }
  }
  if (moveRight == true)
  {
    if (origLoc == gPigOwnNode.physLoc)
    {
      cout<<gPigOwnNode.portNumber<<": Trying to move right from "
        <<gPigOwnNode.physLoc<<endl;
    }

    // Let us see if there is any physical neighbor to our right. If there is, 
    // we can try to save it.
    gPigOtherMutex.lock();
    unsigned short int index = 0;
    for (auto &pigLoc : gPigOtherList.physLoc)
    {
      index++;
      if ((pigLoc - myLoc) == 1)
      {
        bool nbrCanMove = true;
        for (auto &tempPigLoc : gPigOtherList.physLoc)
        {
          if ((tempPigLoc - pigLoc) == 1)
          {
            // The neighbor can't move, don't send anything.
            nbrCanMove = false;
            break;
          }
        }
        if (nbrCanMove == true)
        {
          gPigOtherMutex.lock();
          informPhysicalNeighbor (gPigOtherList.portNumber[index], pigLoc + 1);
          gPigOtherMutex.unlock();
        }
        break;
      }
    }
    gPigOtherMutex.unlock();

    while (true)
    {
      if (origLoc > gPigOwnNode.physLoc)
      {
        // We have already moved left. No need to move right now.
        break;
      }
      // Updating myLoc. The global may have changed in the prev iteration.
      myLoc = gPigOwnNode.physLoc;
      gPigWallMutex.lock();
      for (auto &wallLoc : gPigWallList.physLoc)
      {
        if ((wallLoc - myLoc) == 1)
        {
          // Can't move any further.
          cout<<gPigOwnNode.portNumber<<": Final Loc "<<gPigOwnNode.physLoc
            <<endl;
          return;
        }
      }
      gPigWallMutex.unlock();

      gPigOtherMutex.lock();
      for (auto &pigLoc : gPigOtherList.physLoc)
      {
        if ((pigLoc - myLoc) == 1)
        {
          // Can't move any further
          cout<<gPigOwnNode.portNumber<<": Final Loc "<<gPigOwnNode.physLoc
            <<endl;
          return;
        }
      }
      gPigOtherMutex.unlock();

      if (myLoc < MAX_POSN)
      {
        gPigOwnNode.physLoc++;
      }
      else
      {
        // Can't go right any further.
        cout<<gPigOwnNode.portNumber<<": Final Loc "<<gPigOwnNode.physLoc<<endl;
        return;
      }
    }
  }

  return;
}		/* -----  end of function checkIfAffected  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  handleInitPosnMsg
 * =============================================================================
 */
static void handleInitPosnMsg (int inMsgSize, char *inMsg)
{
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
  short unsigned int srcPort;
  char *permInMsg = inMsg;
  int permInMsgSize = inMsgSize;
  
  // For message type
  inMsg += MSG_TYPE_SIZE;
  inMsgSize -= MSG_TYPE_SIZE;
  if (inMsgSize < SRC_PORT_SIZE)
  {
    cout<<"Corrupted message at pig "<<gPigOwnNode.portNumber<<endl;
    return;
  }
  memcpy (&srcPort, inMsg, SRC_PORT_SIZE);
  srcPort = ntohs(srcPort);
  // Adding own port to the message
  int newSrcPort = ntohs(gPigOwnNode.portNumber);
  memcpy (inMsg, &newSrcPort, SRC_PORT_SIZE);
  inMsg += SRC_PORT_SIZE;
  inMsgSize -= SRC_PORT_SIZE;
  cout<<"Src Port "<<srcPort<<"\t";

  if (inMsgSize < NUMBER_PIGS_SIZE)
  {
    cout<<"Corrupted message at pig "<<gPigOwnNode.portNumber<<endl;
    return;
  }
  short unsigned int numberPigs;
  memcpy (&numberPigs, inMsg, NUMBER_PIGS_SIZE);
  numberPigs = ntohs(numberPigs);
  inMsg += NUMBER_PIGS_SIZE;
  inMsgSize -= NUMBER_PIGS_SIZE;
  if (inMsgSize < NUMBER_WALLS_SIZE)
  {
    cout<<"Corrupted message at pig "<<gPigOwnNode.portNumber<<endl;
    return;
  }
  short unsigned int numberWalls;
  memcpy (&numberWalls, inMsg, NUMBER_WALLS_SIZE);
  numberWalls = ntohs(numberWalls);
  inMsg += NUMBER_WALLS_SIZE;
  inMsgSize -= NUMBER_WALLS_SIZE;

  gPigOtherMutex.lock();
  short unsigned int tempNumberPigs = numberPigs;
  while (tempNumberPigs--)
  {
    short unsigned int otherPort;
    if (inMsgSize < OTHER_PORT_SIZE)
    {
      cout<<"Corrupted message 1 at pig "<<gPigOwnNode.portNumber<<endl;
      gPigOtherMutex.unlock();
      return;
    }
    memcpy (&otherPort, inMsg, OTHER_PORT_SIZE);
    otherPort = ntohs(otherPort);
    inMsg += OTHER_PORT_SIZE;
    inMsgSize -= OTHER_PORT_SIZE;
    gPigOtherList.portNumber.push_back(otherPort);
  }

  while (numberPigs--)
  {
    short unsigned int portLoc;
    if (inMsgSize < PORT_LOC_SIZE)
    {
      cout<<"Corrupted message 2 at pig "<<gPigOwnNode.portNumber<<endl;
      gPigOtherMutex.unlock();
      return;
    }
    memcpy (&portLoc, inMsg, PORT_LOC_SIZE);
    portLoc = ntohs(portLoc);
    inMsg += PORT_LOC_SIZE;
    inMsgSize -= PORT_LOC_SIZE;

    gPigOtherList.physLoc.push_back(portLoc);
  }

  // Update own node from other's list
  int count = 0;
  for (auto &port : gPigOtherList.portNumber)
  {
    if (port == gPigOwnNode.portNumber)
    {
      break;
    }
    count++;
  }
  if (gPigOwnNode.physLoc == 0)
  {
    // Ignore changes in case of a duplicate message
    gPigOwnNode.physLoc = gPigOtherList.physLoc[count];
  }
  // We have updated our own physical location

  gPigOtherMutex.unlock();
  gPigOwnNode.isClosestNode = false;
  if (srcPort == 0)
  {
    gPigOwnNode.isClosestNode = true;
  }

  gPigWallMutex.lock();
  while (numberWalls--)
  {
    short unsigned int wallLoc;
    if (inMsgSize < WALL_LOC_SIZE)
    {
      cout<<"Corrupted message 3 at pig "<<gPigOwnNode.portNumber<<endl;
      gPigWallMutex.unlock();
      return;
    }
    memcpy (&wallLoc, inMsg, WALL_LOC_SIZE);
    wallLoc = ntohs(wallLoc);
    inMsg += WALL_LOC_SIZE;
    inMsgSize -= WALL_LOC_SIZE;

    gPigWallList.physLoc.push_back(wallLoc);
  }
  gPigWallMutex.unlock();

  short unsigned int birdLoc;
  memcpy (&birdLoc, inMsg, PORT_LOC_SIZE);
  birdLoc = ntohs(birdLoc);
  inMsg += PORT_LOC_SIZE;
  inMsgSize -= PORT_LOC_SIZE;
  cout<<"Bird position: "<<birdLoc<<"\t";
  gPigOwnNode.birdLoc = birdLoc;

  short unsigned int hopCount;
  memcpy (&hopCount, inMsg, HOP_LIMIT_SIZE);
  hopCount = ntohs(hopCount);
  hopCount--;
  cout<<"Hop count: "<<hopCount<<endl;
  
  // We compute the logical neighbors, now that we know all the port numbers.
  computeLogicalNeighbors();

  // We check if we are affected by the bird
  checkIfAffected();

  if (hopCount == 0)
  {
    // No need to forward. Can just return.
    return;
  }
  hopCount = htons(hopCount);
  memcpy (inMsg, &hopCount, HOP_LIMIT_SIZE);
  inMsg += HOP_LIMIT_SIZE;
  inMsgSize -= HOP_LIMIT_SIZE;

  // At this stage, the message has been read, internal data structures updated.
  // Now, we should decide where to forward it to.

  cout<<"Own port: "<<gPigOwnNode.portNumber<<" Nbrs: "<<gPigOwnNode.logNbrPorts[0]<<", "<<gPigOwnNode.logNbrPorts[1]<<" Src port: "<<srcPort<<endl;
  if (gPigOwnNode.isClosestNode == true)
  {
    // Send to both logical neighbors
    sendMsg(permInMsg, permInMsgSize, gPigOwnNode.logNbrPorts[0]);
    sendMsg(permInMsg, permInMsgSize, gPigOwnNode.logNbrPorts[1]);
  }
  else
  {
    // Send to only one neighbor
    if (srcPort == gPigOwnNode.logNbrPorts[0])
    {
      // Send to larger neighbor
      sendMsg(permInMsg, permInMsgSize, gPigOwnNode.logNbrPorts[1]);
    }
    else
    {
      // Send to smaller neighbor
      sendMsg(permInMsg, permInMsgSize, gPigOwnNode.logNbrPorts[0]);
    }
  }
  
  return;

}		/* -----  end of function handleInitMsg  ----- */


/* ===  FUNCTION  ==============================================================
 *         Name:  pigMsgHandler
 *  Description:  This function accepts all the messages, finds out their type,
 *                and calls their respective handlers.
 * =============================================================================
 */
void pigMsgHandler (int inMsgSize, char *tempInMsg)
{
  char actualInMsg[MAX_MSG_SIZE];
  char *inMsg = actualInMsg;
  memset(inMsg, 0, MAX_MSG_SIZE);
  memcpy(inMsg, tempInMsg, MAX_MSG_SIZE);
  free (tempInMsg);
  if (inMsgSize < 2)
  {
    cout<<"Corrupted message at pig "<<gPigOwnNode.portNumber<<endl;
    return;
  }

  short unsigned int msgType;
  memcpy (&msgType, inMsg, MSG_TYPE_SIZE);
  msgType = ntohs(msgType);
  // Msg type not pruned from string. Should move over it inside handlers

  switch (msgType)
  {
    case INIT_POSN_MSG:
    {
      handleInitPosnMsg (inMsgSize, inMsg);
      break;
    }
    case INFORM_PHY_NBR_MSG:
    {
      handlePhyNbrMsg (inMsgSize, inMsg);
      break;
    }
    default:
    {
      cout<<"Invalid msg received at pig "<<gPigOwnNode.portNumber<<endl;
      break;
    }
  }
  return;
}		/* -----  end of function pigMsgHandler  ----- */
