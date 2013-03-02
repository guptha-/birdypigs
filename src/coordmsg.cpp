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
 *         Name:  coordSendPosnMsg
 *  Description:  This function sends the positions of all the pigs to the 
 *                closest pig.
 * =============================================================================
 */
int coordSendPosnMsg (vector<int> pigPorts, vector<int> wallPosns, 
                       vector<int> pigPosns)
{
  int closestPig = min_element
                       (pigPosns.begin(), pigPosns.end()) - pigPosns.begin();

  return EXIT_SUCCESS;
}		/* -----  end of function coordSendPosnMsg  ----- */
