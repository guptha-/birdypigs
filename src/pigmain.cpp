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
static void listenerFlow (int listenerPort)
{

  gPigOwnNode.portNumber = listenerPort;
  UDPSocket listenSocket (COM_IP_ADDR, listenerPort);

  while (true)
  {
    // Block for msg receipt
    int inMsgSize;
    char *inMsg;
    inMsg = (char *)malloc (MAX_MSG_SIZE);
    memset(inMsg, 0, MAX_MSG_SIZE);
    try
    {
      inMsgSize = listenSocket.recv(inMsg, MAX_MSG_SIZE);
    }
    catch (SocketException &e)
    {
      cout<<gPigOwnNode.portNumber<<": "<<e.what()<<endl;
    }
    inMsg[inMsgSize] = '\0';

    thread handlerThread (pigMsgHandler, inMsgSize, inMsg);
    handlerThread.detach();
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
