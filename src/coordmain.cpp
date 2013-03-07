/*==============================================================================
 *
 *       Filename:  coordmain.cpp
 *
 *    Description:  This is where the control flow for the coordinator starts.
 *                  The primary functions are as follows:
 *                  1. Read in the config file with the ports of the peers
 *                  2. Launch the peers so that they listen to their own ports
 *                  3. Do game related stuff for each launch:
 *                     a. Randomize physical posn. for each pig and transmit
 *                     b. Randomize bird landing and transmit to closest pig
 *                     c. Request and collect status after play
 *
 * =============================================================================
 */

#include "../inc/coordinc.h"
using namespace std;

// Global pigPorts
vector<int> pigPorts;
mutex gPigPortsMutex;
atomic<int> score;
atomic<int> total;

/* ===  FUNCTION  ==============================================================
 *         Name:  initRand
 *  Description:  This function initializes the random number generator.
 * =============================================================================
 */
void initRand ()
{
  srand(time(NULL));
}		/* -----  end of function initRand  ----- */


/* ===  FUNCTION  ==============================================================
 *         Name:  coordSpawnPigs
 * =============================================================================
 */
int coordSpawnPigs ()
{
  gPigPortsMutex.lock();
  vector<int> tempPigPorts(pigPorts);
  gPigPortsMutex.unlock();
  system ("killall -q -9 pig");
  system ("killall -q -9 pig");
  for (auto &curPort : tempPigPorts)
  {
    int child = fork();
    if (child < 0)
    {
      cout<<"Problem spawning pigs!"<<endl;
      return EXIT_FAILURE;
    }
    else if (child > 0)
    {
      // Child address space
      char strCurPort[5];
      memset(strCurPort, 0, 5);
      sprintf(strCurPort, "%d", curPort);
      cout<<(execl ("./bin/pig", "pig", (const char *)strCurPort, NULL));
      cout<<"Problem spawning child"<<endl;
      cout<<errno;
      exit(0);
    }
  }
  return EXIT_SUCCESS;
}		/* -----  end of function coordSpawnPigs  ----- */
/* ===  FUNCTION  ==============================================================
 *         Name:  getWallPosns
 * =============================================================================
 */
static void getWallPosns (int numberWalls, vector<int> &wallPosns)
{
  // Storing wall positions
  while (numberWalls--)
  {
    int posn;
    while (true)
    {
      posn = (rand() % MAX_POSN) + 1;
      int duplicate = false;
      for (auto &num : wallPosns)
      {
        // making sure there is no overlap
        if (num == posn)
        {
          duplicate = true;
          break;
        }
      }
      if (duplicate == false)
      {
        break;
      }
    }
    wallPosns.push_back(posn);
  }
  cout<<"Wall posns\t";
  for (auto &wallLoc : wallPosns)
  {
    cout<<wallLoc<<"\t";
  }
  cout<<endl;
}		/* -----  end of function getWallPosns  ----- */


/* ===  FUNCTION  ==============================================================
 *         Name:  getPigPosns
 * =============================================================================
 */
static void getPigPosns (int numberPigs, vector<int> &wallPosns, 
                         vector<int> &pigPosns)
{
  // Storing pig positions
  while (numberPigs--)
  {
    int posn;
    while (true)
    {
      posn = (rand() % MAX_POSN) + 1;
      int duplicate = false;
      for (auto &num : pigPosns)
      {
        // making sure there is no overlap with other pigs
        if (num == posn)
        {
          duplicate = true;
          break;
        }
      }
      if (duplicate == true)
      {
        continue;
      }
      for (auto &num : wallPosns)
      {
        // making sure there is no overlap with walls
        if (num == posn)
        {
          duplicate = true;
          break;
        }
      }
      if (duplicate == false)
      {
        break;
      }
    }
    pigPosns.push_back(posn);
  }
  cout<<"Pig Positions\t";
  for (auto &pigLoc : pigPosns)
  {
    cout<<pigLoc<<"\t";
  }
  cout<<endl;
}		/* -----  end of function getPigPosns  ----- */


/* ===  FUNCTION  ==============================================================
 *         Name:  getPigPorts
 * =============================================================================
 */
static int getPigPorts ()
{
  ifstream portFile;
  string str;
  portFile.open("portConfig");
  if (!portFile.good())
  {
    cout<<"No port config file found.\n";
    return EXIT_FAILURE;
  }
  cout<<"Port numbers \t";
  gPigPortsMutex.lock();
  while (true)
  {
    getline(portFile, str);
    if (portFile.eof())
    {
      break;
    }
    int portNum = atoi(str.c_str());
    if (portNum == COORD_LISTEN_PORT)
    {
      cout<<portNum<<" is the port the coordinator listens at! Remove from"<<
        "port list!"<<endl;
      return EXIT_FAILURE;
    }
    pigPorts.push_back(portNum);
    cout<<portNum<<"\t";
    fflush(stdout);
  }
  gPigPortsMutex.unlock();
  cout<<endl;

  return EXIT_SUCCESS;
}		/* -----  end of function getPigPorts  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  coordStartGame
 *  Description:  Control for each game flows from here.
 * =============================================================================
 */
void coordStartGame ()
{
  // Calculate number of walls
  int numberWalls = rand() % (MAX_WALLS + 1); // Limit number of walls

  vector<int> wallPosns;
  getWallPosns (numberWalls, wallPosns);

  vector<int> pigPosns;
  gPigPortsMutex.lock();
  getPigPosns (pigPorts.size(), wallPosns, pigPosns);
  auto closestPig = min_element (pigPosns.begin(), pigPosns.end()) -
                    pigPosns.begin();
  short unsigned int closestPigPort = pigPorts[closestPig];
  gPigPortsMutex.unlock();

  short unsigned int birdLoc = (rand() % MAX_POSN) + 1;
  cout<<"Bird Position: "<<birdLoc<<endl;
  if (coordSendPosnMsg (wallPosns, pigPosns, birdLoc) 
      == EXIT_FAILURE)
  {
    cout<<"Could not send positions to closest pig."<<endl;
    cout<<"Next iteration of the game will proceed"<<endl;
  }

  // Random sleep before bird launch
  sleep (rand() % (MAX_BIRD_TIME + 1));

  if (coordSendBirdLandMsg (wallPosns, pigPosns, birdLoc)
      == EXIT_FAILURE)
  {
    cout<<"Could not send bird land msg to closest pig."<<endl;
  }
  gPigPortsMutex.lock();
  sleep(pigPorts.size());
  gPigPortsMutex.unlock();

  if (coordSendStatusReqMsg (closestPigPort)
      == EXIT_FAILURE)
  {
    cout<<"Could not send bird land msg to closest pig."<<endl;
  }

  return;
}		/* -----  end of function coordStartGame  ----- */


/* ===  FUNCTION  ==============================================================
 *         Name:  listenerFlow
 *  Description:  The coordinator listens for incoming messages here.
 * =============================================================================
 */
static void listenerFlow ()
{ 

  // Block for msg receipt
  char *inMsg;
  inMsg = (char *)malloc (MAX_MSG_SIZE);
  memset(inMsg, 0, MAX_MSG_SIZE);
  int inMsgSize;

  try
  {
    static UDPSocket listenSocket (COM_IP_ADDR, COORD_LISTEN_PORT);
    inMsgSize = listenSocket.recv(inMsg, MAX_MSG_SIZE);
  }
  catch (SocketException &e)
  {
    cout<<"COORD: "<<e.what()<<endl;
  }
  inMsg[inMsgSize] = '\0';

  coordMsgHandler(inMsgSize, inMsg);
}   /* -----  end of function listenerFlow  ----- */


/* ===  FUNCTION  ==============================================================
 *         Name:  main
 *  Description:  Main function for coord module
 * =============================================================================
 */
int main(int argc, char **argv)
{
  // Initialize the random number generator
  initRand();

  score = 0;
  total = 0;
  if (EXIT_FAILURE == getPigPorts ())
  {
    return EXIT_FAILURE;
  }

  while (true)
  {
    if (EXIT_FAILURE == coordSpawnPigs ())
    {
      return EXIT_FAILURE;
    }

    // Give the spawned pigs a chance to get their bearings
    sleep(1);

    thread handlerThread (listenerFlow);
    handlerThread.detach();
    coordStartGame();
    sleep (1);
  }

  return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */


