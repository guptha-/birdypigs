#ifndef inc_COORD_INC
#define inc_COORD_INC

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <string>
#include <thread>
#include <mutex>
#include <arpa/inet.h>

#include "coordconst.h"
#include "coordprot.h"
#include "comconst.h"
#include "PracticalSocket.h"

extern vector<int> pigPorts;
extern mutex gPigPortsMutex;
#endif
