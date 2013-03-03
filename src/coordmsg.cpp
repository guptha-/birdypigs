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
    cout<<"Corrupted message at coord";
    return;
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
 *         Name:  coordSendPosnMsg
 *  Description:  This function sends the positions of all the pigs to the 
 *                closest pig. It also sends the landing posn of the bird.
 * =============================================================================
 */
int coordSendPosnMsg (vector<int> pigPorts, vector<int> wallPosns, 
                       vector<int> pigPosns)
{
  auto closestPig = min_element (pigPosns.begin(), pigPosns.end()) -
                    pigPosns.begin();
  short unsigned int closestPigPort = pigPorts[closestPig];
  
  // This is the message sent at the beginning of each launch. This 
  // gives details about the other pigs and walls.
  char actualOutMsg[MAX_MSG_SIZE];
  char *outMsg = actualOutMsg;
  int outMsgSize = 0;
  if ((MSG_TYPE_SIZE + SRC_PORT_SIZE + NUMBER_PIGS_SIZE + NUMBER_WALLS_SIZE +
      (pigPorts.size() * (OTHER_PORT_SIZE + PORT_LOC_SIZE)) +
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

  short unsigned int numberPigs = pigPorts.size();
  numberPigs = htons(numberPigs);
  memcpy (outMsg, &numberPigs, NUMBER_PIGS_SIZE);
  outMsg += NUMBER_PIGS_SIZE;
  outMsgSize += NUMBER_PIGS_SIZE;

  short unsigned int numberWalls = wallPosns.size();
  numberWalls = htons(numberWalls);
  memcpy (outMsg, &numberWalls, NUMBER_WALLS_SIZE);
  outMsg += NUMBER_WALLS_SIZE;
  outMsgSize += NUMBER_WALLS_SIZE;

  for (auto &otherPort : pigPorts)
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

  short unsigned int birdLoc = (rand() % MAX_POSN) + 1;
  birdLoc = htons(birdLoc);
  memcpy (outMsg, &birdLoc, PORT_LOC_SIZE);
  outMsg += PORT_LOC_SIZE;
  outMsgSize += PORT_LOC_SIZE;

  short unsigned int hopCount = pigPorts.size() / 2 + 1;
  hopCount = htons(hopCount);
  memcpy (outMsg, &hopCount, HOP_LIMIT_SIZE);
  outMsg += HOP_LIMIT_SIZE;
  outMsgSize += HOP_LIMIT_SIZE;

  sendMsg(actualOutMsg, outMsgSize, closestPigPort);
  return EXIT_SUCCESS;
}		/* -----  end of function coordSendPosnMsg  ----- */
