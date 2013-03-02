/*==============================================================================
 *
 *       Filename:  pigmain.cpp
 *
 *    Description:  The pig's execution starts here, but you already deduced 
 *                  that from the file's name, didn't you?
 *
 * =============================================================================
 */

#include "../inc/piginc.h"

// GLOBALS START
PigOwnNode gPigOwnNode;
PigOtherList gPigOtherList;
PigWallList gPigWallList;
mutex gPigOtherMutex;
mutex gPigWallMutex;
// GLOBALS END

/* ===  FUNCTION  ==============================================================
 *         Name:  listenerFlow
 *  Description:  The pig listens for incoming messages here. This is the only
 *                way to trigger events on a pig. Everything on the pig is in
 *                response to some message.
 * =============================================================================
 */
void listenerFlow (int listenerPort)
{
  gPigOwnNode.portNumber = listenerPort;
  UDPSocket listenSocket (listenerPort);
  char inMsg[MAX_MSG_SIZE];
  memset(inMsg, 0, MAX_MSG_SIZE);

  while (true)
  {
    // Block for msg receipt
    short unsigned int sourcePort;
    string sourceAddr;
    int inMsgSize = listenSocket.recvFrom(inMsg, MAX_MSG_SIZE, sourceAddr, 
                                          sourcePort);

    thread handlerThread (pigMsgHandler, inMsgSize, inMsg);
  }
}		/* -----  end of function listenerFlow  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  main
 * =============================================================================
 */
int main (int argc, char *argv[])
{
  // GLOBALS INIT
  memset (&gPigOwnNode, 0, sizeof(gPigOwnNode));
  // GLOBALS INIT END
  
  if (argc < 2)
  {
    cout<<"Port number has not been specified!"<<endl;
    return EXIT_FAILURE;
  }

  listenerFlow (atoi(argv[1]));

  return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
