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

/* ===  FUNCTION  ==============================================================
 *         Name:  listenerFlow
 *  Description:  The coordinator listens for incoming messages here.
 * =============================================================================
 */
static void listenerFlow ()
{ 
  UDPSocket listenSocket (COM_IP_ADDR, COORD_LISTEN_PORT);

  while (true)
  { 
    // Block for msg receipt
    char *inMsg;
    inMsg = (char *)malloc (MAX_MSG_SIZE);
    memset(inMsg, 0, MAX_MSG_SIZE);
    int inMsgSize = listenSocket.recv(inMsg, MAX_MSG_SIZE);
    inMsg[inMsgSize] = '\0';

    thread handlerThread (coordMsgHandler, inMsgSize, inMsg);
    handlerThread.detach();
  }
}   /* -----  end of function listenerFlow  ----- */



/* ===  FUNCTION  ==============================================================
 *         Name:  initRand
 *  Description:  This function initializes the random number generator.
 * =============================================================================
 */
void initRand ()
{
/*  fstream configFile;
  char str[4];
  int seed;
  configFile.open("/dev/urandom");
  configFile.get(str, 4);

  seed = atoi(str);*/
  srand(time(NULL));
}		/* -----  end of function initRand  ----- */


/* ===  FUNCTION  ==============================================================
 *         Name:  spawnPigs
 * =============================================================================
 */
int spawnPigs (vector<int> pigPorts)
{
  for (auto &curPort : pigPorts)
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
      cout<<errno;
      exit(0);
    }
  }
  return EXIT_SUCCESS;
}		/* -----  end of function spawnPigs  ----- */
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
static int getPigPorts (vector<int> &pigPorts)
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
  cout<<endl;

  return EXIT_SUCCESS;
}		/* -----  end of function getPigPorts  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  startGame
 *  Description:  Control for each game flows from here.
 * =============================================================================
 */
static void startGame (vector<int> &pigPorts)
{
  // Calculate number of walls
  int numberWalls = rand() % (MAX_WALLS + 1); // Limit number of walls

  vector<int> wallPosns;
  getWallPosns (numberWalls, wallPosns);

  vector<int> pigPosns;
  getPigPosns (pigPorts.size(), wallPosns, pigPosns);
 
  short unsigned int birdLoc = (rand() % MAX_POSN) + 1;
  if (coordSendPosnMsg (pigPorts, wallPosns, pigPosns, birdLoc) 
      == EXIT_FAILURE)
  {
    cout<<"Could not send positions to closest pig."<<endl;
    cout<<"Next iteration of the game will proceed"<<endl;
  }

  sleep (3);

  if (coordSendBirdLandMsg (pigPorts, wallPosns, pigPosns, birdLoc)
      == EXIT_FAILURE)
  {
    cout<<"Could not send bird land msg to closest pig."<<endl;
  }

  return;
}		/* -----  end of function startGame  ----- */


/* ===  FUNCTION  ==============================================================
 *         Name:  main
 *  Description:  Main function for coord module
 * =============================================================================
 */
int main(int argc, char **argv)
{
  // Initialize the random number generator
  initRand();

  vector<int> pigPorts;
  if (EXIT_FAILURE == getPigPorts (pigPorts))
  {
    return EXIT_FAILURE;
  }

  if (EXIT_FAILURE == spawnPigs (pigPorts))
  {
    return EXIT_FAILURE;
  }

  // Give the spawned pigs a chance to get their bearings
  sleep(1);

  // The listener is in a separate plane of existance
  thread listenerThread (listenerFlow);
  listenerThread.detach();

//  while (true)
  {
    // Start an instance of the game
    startGame(pigPorts);
    sleep(50);
  }

  return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */


