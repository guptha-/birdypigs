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
 *         Name:  initRand
 *  Description:  This function initializes the random number generator.
 * =============================================================================
 */
static void initRand ()
{
  fstream configFile;
  char str[4];
  int seed;
  configFile.open("/dev/urandom");
  configFile.get(str, 4);

  seed = atoi(str);
  srand(seed);
}		/* -----  end of function initRand  ----- */


/* ===  FUNCTION  ==============================================================
 *         Name:  spawnPigs
 * =============================================================================
 */
int spawnPigs (vector<int> pigPorts)
{
  // Fill this up once a skeleton of the pig's code is done
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
  cout<<"The port numbers are:"<<endl;
  while (true)
  {
    getline(portFile, str);
    if (portFile.eof())
    {
      break;
    }
    int portNum = atoi(str.c_str());
    pigPorts.push_back(portNum);
    cout<<portNum<<endl;
    fflush(stdout);
  }

  return EXIT_SUCCESS;
}		/* -----  end of function getPigPorts  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  startGame
 *  Description:  Control for each game flows from here.
 * =============================================================================
 */
static void startGame (int numberPigs, vector<int> &pigPorts)
{
  // Calculate number of walls
  int numberWalls = rand() % (MAX_WALLS + 1); // Limit number of walls

  vector<int> wallPosns;
  getWallPosns (numberWalls, wallPosns);

  // TODO remove
  cout<<"Pig ports:"<<endl;
  for (auto &temp : pigPorts)
  {
    cout<<temp<<endl;
  }

  cout<<"Wall positions:"<<endl;
  for (auto &temp : wallPosns)
  {
    cout<<temp<<endl;
  }

  vector<int> pigPosns;
  getPigPosns (numberPigs, wallPosns, pigPosns);

  cout<<"Pig positions:"<<endl;
  for (auto &temp : pigPosns)
  {
    cout<<temp<<endl;
  }

 if (coordSendPosnMsg (pigPorts, wallPosns, pigPosns) 
     == EXIT_FAILURE)
  {
    cout<<"Could not send positions to closest pig."<<endl;
    cout<<"Next iteration of the game will proceed"<<endl;
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

  int number;
  if (argc == 2)
  {
    number = atoi(argv[1]);
  }
  else
  {
    number = DEF_NUM_PIGS;
  }

  vector<int> pigPorts;
  if (EXIT_FAILURE == getPigPorts (pigPorts))
  {
    return EXIT_FAILURE;
  }

  if (EXIT_FAILURE == spawnPigs (pigPorts))
  {
    return EXIT_FAILURE;
  }

  while (true)
  {
    // Start an instance of the game
    startGame(number, pigPorts);
    sleep(5);
  }

  return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */


