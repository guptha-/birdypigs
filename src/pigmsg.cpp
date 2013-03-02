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
 *         Name:  handleInitPosnMsg
 * =============================================================================
 */
static void handleInitPosnMsg (int inMsgSize, char *inMsg)
{
  // This is the message sent at the beginning of each launch. This 
  // gives details about the other pigs and walls.
  short unsigned int srcPort;
  if (inMsgSize < SRC_PORT_SIZE)
  {
    cout<<"Corrupted message at pig "<<gPigOwnNode.portNumber<<endl;
  }
  memcpy (&srcPort, inMsg, SRC_PORT_SIZE);
  srcPort = ntohs(srcPort);
  inMsg += SRC_PORT_SIZE;
  inMsgSize -= SRC_PORT_SIZE;

  if (inMsgSize < NUMBER_PIGS_SIZE)
  {
    cout<<"Corrupted message at pig "<<gPigOwnNode.portNumber<<endl;
  }
  short unsigned int numberPigs;
  memcpy (&numberPigs, inMsg, NUMBER_PIGS_SIZE);
  numberPigs = ntohs(numberPigs);
  inMsg += NUMBER_PIGS_SIZE;
  inMsgSize -= NUMBER_PIGS_SIZE;
  if (inMsgSize < NUMBER_WALLS_SIZE)
  {
    cout<<"Corrupted message at pig "<<gPigOwnNode.portNumber<<endl;
  }
  short unsigned int numberWalls;
  memcpy (&numberWalls, inMsg, NUMBER_WALLS_SIZE);
  numberWalls = ntohs(numberWalls);
  inMsg += NUMBER_WALLS_SIZE;
  inMsgSize -= NUMBER_WALLS_SIZE;

  gPigOtherMutex.lock();
  while (numberPigs--)
  {
    short unsigned int otherPort;
    if (inMsgSize < OTHER_PORT_SIZE)
    {
      cout<<"Corrupted message at pig "<<gPigOwnNode.portNumber<<endl;
    }
    memcpy (&otherPort, inMsg, OTHER_PORT_SIZE);
    otherPort = ntohs(otherPort);
    inMsg += OTHER_PORT_SIZE;
    inMsgSize -= OTHER_PORT_SIZE;

    short unsigned int portLoc;
    if (inMsgSize < PORT_LOC_SIZE)
    {
      cout<<"Corrupted message at pig "<<gPigOwnNode.portNumber<<endl;
    }
    memcpy (&portLoc, inMsg, PORT_LOC_SIZE);
    portLoc = ntohs(portLoc);
    inMsg += PORT_LOC_SIZE;
    inMsgSize -= PORT_LOC_SIZE;

    gPigOtherList.portNumber.push_back(otherPort);
    gPigOtherList.physLoc.push_back(portLoc);

    if (srcPort == 0)
    {
      gPigOtherList.isClosestNode = true;
    }
  }
  gPigOtherMutex.unlock();

  gPigWallMutex.lock();
  while (numberWalls--)
  {
    short unsigned int wallLoc;
    if (inMsgSize < WALL_LOC_SIZE)
    {
      cout<<"Corrupted message at pig "<<gPigOwnNode.portNumber<<endl;
    }
    memcpy (&wallLoc, inMsg, WALL_LOC_SIZE);
    wallLoc = ntohs(wallLoc);
    inMsg += WALL_LOC_SIZE;
    inMsgSize -= WALL_LOC_SIZE;

    gPigWallList.physLoc.push_back(wallLoc);
  }
  gPigWallMutex.unlock();
  return;

}		/* -----  end of function handleInitMsg  ----- */



/* ===  FUNCTION  ==============================================================
 *         Name:  pigMsgHandler
 *  Description:  This function accepts all the messages, finds out their type,
 *                and calls their respective handlers.
 * =============================================================================
 */
void pigMsgHandler (int inMsgSize, char *inMsg)
{
  if (inMsgSize < 2)
  {
    cout<<"Corrupted message at pig "<<gPigOwnNode.portNumber<<endl;
  }

  short unsigned int msgType;
  memcpy (&msgType, inMsg, MSG_TYPE_SIZE);
  msgType = ntohs(msgType);
  inMsg += MSG_TYPE_SIZE;
  inMsgSize -= MSG_TYPE_SIZE;

  switch (msgType)
  {
    case INIT_POSN_MSG:
    {
      handleInitPosnMsg (inMsgSize, inMsg);
      break;
    }
  }
  return;
}		/* -----  end of function pigMsgHandler  ----- */
